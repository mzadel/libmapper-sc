
#include "PyrObject.h"
#include "PyrPrimitive.h"
#include "VMGlobals.h"
#include "PyrSched.h"

#include <mapper/mapper.h>

PyrSymbol *s_callAction;

extern bool compiledOK;

namespace Mapper {

	struct Device
	{

		Device( mapper_device dev ) : m_dev( dev ), m_polling( false ) {}

		static void* polling_loop( void* arg );

		mapper_device m_dev;
		pthread_t m_thread;
		bool m_polling;

	};

	static inline Device* getDeviceStruct( PyrSlot* slot )
	{
		// Get the PyrObject pointer from the argument, then get that object's
		// first slot, which is a pointer to the internal C++ data structure
		return (Device*) slotRawPtr( slotRawObject(slot)->slots+0 );
	}

	namespace Signal
	{
		static void input_handler( mapper_signal msig, mapper_db_signal props, mapper_timetag_t *timetag, void *value );
	}

}

// This is the function that is run by the polling thread, created in
// mapperStartPolling().  It polls the libmapper device for changes on a
// regular interval.
//
void* Mapper::Device::polling_loop( void* arg )
{
	Mapper::Device *devstruct = (Mapper::Device*) arg;

	devstruct->m_polling = true;

	while (devstruct->m_polling) {
		int numhandled = 1;
		while ( numhandled > 0 ) {
			numhandled = mdev_poll(devstruct->m_dev, 0);
		}
		usleep(5000);
	}
}

// This is the callback that gets registered with libmapper.  It is called
// whenever there is an incoming signal change.  It propagates the signal name
// and value to MapperSignal:prCallAction() on the supercollider language
// side, which calls the appropriate user-defined action function.
//
void Mapper::Signal::input_handler( mapper_signal msig, mapper_db_signal props, mapper_timetag_t *timetag, void *value )
{
	pthread_mutex_lock (&gLangMutex);

	if (compiledOK) {

		VMGlobals* g = gMainVMGlobals;

		PyrObject *obj = (PyrObject*) props->user_data;

		// MapperSignal object we are dispatching to
		++g->sp; SetObject(g->sp, obj);
		int numArgsPushed = 1;

		// the received osc address string
		++g->sp; SetSymbol(g->sp, getsym(props->name));
		numArgsPushed++;

		// the value
		if (value) {
			char type = props->type;

			if ( type == 'f' ) {
				++g->sp; SetFloat(g->sp, *(float*)value);
				numArgsPushed++;
			}
			else if ( type == 'i' ) {
				++g->sp; SetInt(g->sp, *(int*)value);
				numArgsPushed++;
			}
			else {
				post("Mapper::Signal::input_handler(): ignoring value of unsupported type (%c)\n", type);
			}
		}

		runInterpreter(g, s_callAction, numArgsPushed);

	}

	pthread_mutex_unlock (&gLangMutex);
}

int mapperDeviceNew(struct VMGlobals *g, int numArgsPushed);
int mapperDeviceNew(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp - 2;
	PyrSlot *b = g->sp - 1;
	PyrSlot *c = g->sp;

	int err, portrequested;
	PyrSymbol *devicename;
	mapper_device dev;
	Mapper::Device *devstruct;

	err = slotSymbolVal(b, &devicename);
	if (err) return errWrongType;

	err = slotIntVal(c, &portrequested);
	if (err) return errWrongType;

	dev = mdev_new(devicename->name, portrequested, 0);
	devstruct = new Mapper::Device(dev);

	SetPtr(slotRawObject(a)->slots+0, devstruct);

	return errNone;
}

int mapperDeviceFree(struct VMGlobals *g, int numArgsPushed);
int mapperDeviceFree(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;

	Mapper::Device *devstruct;
	mapper_device dev;

	devstruct = Mapper::getDeviceStruct(a);
	dev = devstruct->m_dev;

	// free the device in libmapper
	mdev_free( dev );

	// free the local structure associated with the device
	delete devstruct;
	SetPtr(slotRawObject(a)->slots+0, NULL);

	return errNone;
}

int mapperDeviceAddInput(struct VMGlobals *g, int numArgsPushed);
int mapperDeviceAddInput(struct VMGlobals *g, int numArgsPushed)
{

	PyrSlot *pa = g->sp - 7; // object pointer
	PyrSlot *pb = g->sp - 6; // name
	PyrSlot *pc = g->sp - 5; // length
	PyrSlot *pd = g->sp - 4; // type
	PyrSlot *pe = g->sp - 3; // unit
	PyrSlot *pf = g->sp - 2; // min
	PyrSlot *pg = g->sp - 1; // max
	PyrSlot *ph = g->sp;     // sclang signal object

	int err;

	PyrSymbol *signalnamesymbol;
	PyrSymbol *unitsymbol;

	// parameters for mdev_add_input()
	mapper_device dev;
	char *signalname;
	int length;
	char type;
	char *unit;
	float min;
	float max;
	PyrObject *signalobj;

	// parse the arguments

	dev = Mapper::getDeviceStruct(pa)->m_dev;

	err = slotSymbolVal(pb, &signalnamesymbol);
	if (err) return errWrongType;
	signalname = signalnamesymbol->name;

	err = slotIntVal(pc, &length);
	if (err) return errWrongType;

	type = slotRawChar(pd);
	// assume for now that it's 'f'

	err = slotSymbolVal(pe, &unitsymbol);
	if (err) return errWrongType;
	unit = unitsymbol->name;

	err = slotFloatVal(pf, &min);
	if (err) return errWrongType;

	err = slotFloatVal(pg, &max);
	if (err) return errWrongType;

	signalobj = slotRawObject(ph);

	// add the signal
	mapper_signal sig = mdev_add_input( dev, signalname, length, type, unit, &min, &max, Mapper::Signal::input_handler, signalobj );

	// set the dataptr field in signalobj to hold the mapper_signal pointer
	SetPtr(signalobj->slots+0, sig);

	// return the MapperSignal object
	SetObject(pa, signalobj);

	return errNone;
}

int mapperStartPolling(struct VMGlobals *g, int numArgsPushed);
int mapperStartPolling(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;

	Mapper::Device *devstruct = Mapper::getDeviceStruct(a);

	// FIXME the way this works now, there's one thread per Mapper instance
	// there should only be one thread of all of them; make a manager class or
	// something?
	pthread_create( &devstruct->m_thread, NULL, Mapper::Device::polling_loop, (void*)devstruct );

	return errNone;
}

int mapperStopPolling(struct VMGlobals *g, int numArgsPushed);
int mapperStopPolling(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;

	Mapper::Device *devstruct = Mapper::getDeviceStruct(a);

	devstruct->m_polling = false;
	pthread_join(devstruct->m_thread, 0);

	return errNone;
}

int mapperIsPolling(struct VMGlobals *g, int numArgsPushed);
int mapperIsPolling(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	Mapper::Device *devstruct = Mapper::getDeviceStruct(a);
	SetBool( a, devstruct->m_polling );
	return errNone;
}

void initMapperPrimitives()
{

	int base, index = 0;

	base = nextPrimitiveIndex();

	definePrimitive(base, index++, "_MapperDeviceNew", mapperDeviceNew, 3, 0);
	definePrimitive(base, index++, "_MapperDeviceFree", mapperDeviceFree, 1, 0);
	definePrimitive(base, index++, "_MapperDeviceAddInput", mapperDeviceAddInput, 8, 0);
	definePrimitive(base, index++, "_MapperStartPolling", mapperStartPolling, 1, 0);
	definePrimitive(base, index++, "_MapperStopPolling", mapperStopPolling, 1, 0);
	definePrimitive(base, index++, "_MapperIsPolling", mapperIsPolling, 1, 0);

	s_callAction = getsym("prCallAction");

}


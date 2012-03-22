
#include "PyrObject.h"
#include "PyrPrimitive.h"
#include "VMGlobals.h"
#include "PyrSched.h"

#include <mapper/mapper.h>

PyrSymbol *s_dispatchInputAction;

extern bool compiledOK;

namespace Mapper {

	struct Device
	{

		Device() : m_dev( NULL ), m_running( false ) {}

		static void input_handler( mapper_signal msig, mapper_db_signal props, mapper_timetag_t *timetag, void *value );

		static void* polling_loop( void* arg );

		mapper_device m_dev;
		pthread_t m_thread;
		bool m_running;

	};

	static inline Device* getDeviceStruct( PyrSlot* slot )
	{
		// Get the PyrObject pointer from the argument, then get that object's
		// first slot, which is a pointer to the internal C++ data structure
		return (Device*) slotRawPtr( &slotRawObject(slot)->slots[0] );
	}

}

void Mapper::Device::input_handler( mapper_signal msig, mapper_db_signal props, mapper_timetag_t *timetag, void *value )
{
	pthread_mutex_lock (&gLangMutex);

	if (compiledOK) {

		VMGlobals* g = gMainVMGlobals;

		PyrObject *obj = (PyrObject*) props->user_data;

		// Mapper object we are dispatching to
		++g->sp; SetObject(g->sp, obj);
		int numArgsPushed = 1;

		// the received osc address string
		++g->sp; SetSymbol(g->sp, getsym(props->name));
		numArgsPushed++;

		// the value
		if (value) {
			char type = props->type;

			if ( type == 'f' ) {
				++g->sp; SetFloat(g->sp, *(float*)value );
				numArgsPushed++;
			}
			else if ( type == 'i' ) {
				++g->sp; SetInt(g->sp, *(int*)value );
				numArgsPushed++;
			}
			else {
				post("Mapper::Device::input_handler(): ignoring value of unsupported type (%c)\n", type);
			}
		}

		runInterpreter(g, s_dispatchInputAction, numArgsPushed);

	}

	pthread_mutex_unlock (&gLangMutex);
}

void* Mapper::Device::polling_loop( void* arg )
{
	Mapper::Device *devstruct = (Mapper::Device*) arg;

	devstruct->m_running = true;

	while (devstruct->m_running) {
		int numhandled = 1;
		while ( numhandled > 0 ) {
			numhandled = mdev_poll(devstruct->m_dev, 0);
		}
		usleep(5000);
	}
}

int mapperInit(struct VMGlobals *g, int numArgsPushed);
int mapperInit(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp - 2;
	PyrSlot *b = g->sp - 1;
	PyrSlot *c = g->sp;

	// allocate a new Device class instance
	Mapper::Device *devstruct = new Mapper::Device;

	int err, portrequested;
	PyrSymbol *devicename;

	err = slotIntVal(b, &portrequested);
	if (err) return errWrongType;

	err = slotSymbolVal(c, &devicename);
	if (err) return errWrongType;

	// poplate the device field in the Device instance
	devstruct->m_dev = mdev_new(devicename->name, portrequested, 0);

	SetPtr(slotRawObject(a)->slots+0, devstruct);

	return errNone;
}

int mapperAddInput(struct VMGlobals *g, int numArgsPushed);
int mapperAddInput(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp - 4; // object
	PyrSlot *b = g->sp - 3; // osc address (the signal name)
	PyrSlot *c = g->sp - 2; // type char
	PyrSlot *d = g->sp - 1; // min value
	PyrSlot *e = g->sp;     // max value

	// FIXME need error checking on the types here since it'll abort sc if it
	// doesn't get what it's expecting
	// FIXME also need to support default arguments (float type, min 0.0, max
	// 1.0)

	PyrObject *obj = slotRawObject(a);
	PyrSymbol *name = slotRawSymbol(b);
	char typechar = slotRawChar(c);

	// extract the min and max using the appropriate type
	void *min = NULL;
	void *max = NULL;
	float minfloat;
	float maxfloat;
	int minint;
	int maxint;

	if ( typechar == 'f' ) {
		minfloat = slotRawFloat(d);
		maxfloat = slotRawFloat(e);
		min = &minfloat;
		max = &maxfloat;
	}
	else if ( typechar == 'i' ) {
		minint = slotRawInt(d);
		maxint = slotRawInt(e);
		min = &minint;
		max = &maxint;
	}
	else {
		post("mapperAddInput(): ignoring min/max for signal of unsupported type (%c)\n", typechar);
	}

	mapper_device dev = Mapper::getDeviceStruct( a )->m_dev;

	// extract the signal name (osc address) from the name symbol
	char *signalname = name->name;

	// register the callback for the input.  store the object pointer in the
	// user data for the callback, which will be passed to invocations of the
	// callback.
	mdev_add_input(dev, signalname, 1, typechar, 0, min, max, Mapper::Device::input_handler, obj);

	return errNone;
}

int mapperStart(struct VMGlobals *g, int numArgsPushed);
int mapperStart(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;

	Mapper::Device *devstruct = Mapper::getDeviceStruct( a );

	// FIXME the way this works now, there's one thread per Mapper instance
	// there should only be one thread of all of them; make a manager class or
	// something?
	pthread_create( &devstruct->m_thread, NULL, Mapper::Device::polling_loop, (void*)devstruct );

	return errNone;
}

int mapperStop(struct VMGlobals *g, int numArgsPushed);
int mapperStop(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;

	Mapper::Device *devstruct = Mapper::getDeviceStruct( a );

	devstruct->m_running = false;
	pthread_join(devstruct->m_thread, 0);

	return errNone;
}

int mapperDevFree(struct VMGlobals *g, int numArgsPushed);
int mapperDevFree(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;

	Mapper::Device *devstruct = Mapper::getDeviceStruct( a );

	mdev_free( devstruct->m_dev );
	devstruct->m_dev = NULL;

	return errNone;
}

int mapperGetCurrentValue(struct VMGlobals *g, int numArgsPushed);
int mapperGetCurrentValue(struct VMGlobals *g, int numArgsPushed)
{
	// pull the last polled value out of currentvalue

	PyrSlot *receiver = g->sp;

	// set the receiver slot to return the value
	//SetFloat(receiver, currentvalue);
	// FIXME change this to use the built-in libmapper function for querying
	// the current value

	return errNone;
}

int mapperPort(struct VMGlobals *g, int numArgsPushed);
int mapperPort(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	mapper_device dev = Mapper::getDeviceStruct( a )->m_dev;
	SetInt( a, mdev_port( dev ) );
	return errNone;
}

void initMapperPrimitives()
{
	int base, index = 0;

	base = nextPrimitiveIndex();

	definePrimitive(base, index++, "_MapperInit", mapperInit, 3, 0);
	definePrimitive(base, index++, "_MapperAddInput", mapperAddInput, 5, 0);
	definePrimitive(base, index++, "_MapperStart", mapperStart, 1, 0);
	definePrimitive(base, index++, "_MapperStop", mapperStop, 1, 0);
	definePrimitive(base, index++, "_MapperDevFree", mapperDevFree, 1, 0);
	definePrimitive(base, index++, "_MapperGetCurrentValue", mapperGetCurrentValue, 1, 0);
	definePrimitive(base, index++, "_MapperPort", mapperPort, 1, 0);

	s_dispatchInputAction = getsym("prDispatchInputAction");

}

// vim:sw=4:ts=4:

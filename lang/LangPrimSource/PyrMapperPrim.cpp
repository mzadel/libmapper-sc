
#include "PyrObject.h"
#include "PyrPrimitive.h"
#include "PyrSymbol.h"
#include "PyrSched.h"
#include "SCBase.h"

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
		static void input_handler( mapper_signal msig, mapper_db_signal props, int instance_id, void *value, int count, mapper_timetag_t *timetag );
	}

}

inline int numericSlotToPointer( PyrSlot *slot, char type, float *floatstorage, int *intstorage, void **storagepointer );
inline int pointerToNumericSlot( char type, void *value, PyrSlot *slot );


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
void Mapper::Signal::input_handler( mapper_signal msig, mapper_db_signal props, int instance_id, void *value, int count, mapper_timetag_t *timetag )
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

		// the instance id
		++g->sp; SetInt(g->sp, instance_id);
		numArgsPushed++;

		// the value
		++g->sp;
		int err = pointerToNumericSlot( props->type, value, g->sp );
		if ( err != errNone ) {
			SetNil(g->sp);
			post("Mapper::Signal::input_handler(): pointerToNumericSlot() failed, setting incoming value to nil\n");
		}
		numArgsPushed++;

		runInterpreter(g, s_callAction, numArgsPushed);

	}

	pthread_mutex_unlock (&gLangMutex);
}

inline int numericSlotToPointer( PyrSlot *slot, char type, float *floatstorage, int *intstorage, void **storagepointer )
{
	// copies the value from the sclang slot to one of the storage variables
	// fills storagepointer to point to the storage variable of the correct type
	// if the slot is nil, sets the storagepointer to NULL

	if ( IsNil(slot) ) {
		*storagepointer = NULL;
	}

	else if ( type == 'f' ) {
		int err = slotFloatVal(slot, floatstorage);
		if (err) return err;
		*storagepointer = floatstorage;
	}

	else if ( type == 'i' ) {
		int err = slotIntVal(slot, intstorage);
		if (err) return err;
		*storagepointer = intstorage;
	}

	else {
		post("numericSlotToPointer(): unsupported type (%c)\n", type);
		return errFailed;
	}

	return errNone;
}

inline int pointerToNumericSlot( char type, void *value, PyrSlot *slot )
{

	if( value == NULL ) {
		SetNil( slot );
	}

	else if( type == 'f' ) {
		SetFloat( slot, *((float*) value) );
	}

	else if( type == 'i' ) {
		SetInt( slot, *((int*) value) );
	}

	else {
		post("pointerToNumericSlot(): unsupported type (%c)\n", type);
		return errFailed;
	}

	return errNone;

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

	if ( devstruct->m_polling ) {
		post("mapperDeviceFree(): still polling!  this shouldn't happen!\n");
	}

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

	float minfloat;
	float maxfloat;
	int minint;
	int maxint;

	// parameters for mdev_add_input()
	mapper_device dev;
	char *signalname;
	int length;
	char type;
	char *unit;
	void *min = NULL;
	void *max = NULL;
	PyrObject *signalobj;

	// parse the arguments

	dev = Mapper::getDeviceStruct(pa)->m_dev;

	err = slotSymbolVal(pb, &signalnamesymbol);
	if (err) return errWrongType;
	signalname = signalnamesymbol->name;

	err = slotIntVal(pc, &length);
	if (err) return errWrongType;

	type = slotRawChar(pd);
	if ( type != 'f' && type != 'i' ) {
		post("mapperDeviceAddInput(): unsupported type (%c)\n", type);
		return errFailed;
	}

	err = slotSymbolVal(pe, &unitsymbol);
	if (err) return errWrongType;
	unit = unitsymbol->name;

	err = numericSlotToPointer( pf, type, &minfloat, &minint, &min );
	if (err) return errWrongType;

	err = numericSlotToPointer( pg, type, &maxfloat, &maxint, &max );
	if (err) return errWrongType;

	signalobj = slotRawObject(ph);

	// add the signal
	mapper_signal sig = mdev_add_input( dev, signalname, length, type, unit, min, max, Mapper::Signal::input_handler, signalobj );

	// determine return value based on the result of mdev_add_input()
	if ( sig != NULL ) {
		// set the dataptr field in signalobj to hold the mapper_signal pointer
		SetPtr(signalobj->slots+0, sig);
		// return the MapperSignal object
		SetObject(pa, signalobj);
	}
	else {
		// NULL sig means the signal was not added (eg tried to add a duplicate signal name)
		post("mapperDeviceAddInput(): mdev_add_input() failed\n");
		// return nil
		SetNil(pa);
	}

	return errNone;

}

int mapperDeviceAddOutput(struct VMGlobals *g, int numArgsPushed);
int mapperDeviceAddOutput(struct VMGlobals *g, int numArgsPushed)
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

	float minfloat;
	float maxfloat;
	int minint;
	int maxint;

	// parameters for mdev_add_output()
	mapper_device dev;
	char *signalname;
	int length;
	char type;
	char *unit;
	void *min = NULL;
	void *max = NULL;
	PyrObject *signalobj;

	// parse the arguments

	dev = Mapper::getDeviceStruct(pa)->m_dev;

	err = slotSymbolVal(pb, &signalnamesymbol);
	if (err) return errWrongType;
	signalname = signalnamesymbol->name;

	err = slotIntVal(pc, &length);
	if (err) return errWrongType;

	type = slotRawChar(pd);
	if ( type != 'f' && type != 'i' ) {
		post("mapperDeviceAddOutput(): unsupported type (%c)\n", type);
		return errFailed;
	}

	err = slotSymbolVal(pe, &unitsymbol);
	if (err) return errWrongType;
	unit = unitsymbol->name;

	err = numericSlotToPointer( pf, type, &minfloat, &minint, &min );
	if (err) return errWrongType;

	err = numericSlotToPointer( pg, type, &maxfloat, &maxint, &max );
	if (err) return errWrongType;

	signalobj = slotRawObject(ph);

	// add the signal
	mapper_signal sig = mdev_add_output( dev, signalname, length, type, unit, min, max );

	// determine return value based on the result of mdev_add_output()
	if ( sig != NULL ) {
		// set the dataptr field in signalobj to hold the mapper_signal pointer
		SetPtr(signalobj->slots+0, sig);
		// return the MapperSignal object
		SetObject(pa, signalobj);
	}
	else {
		// NULL sig means the signal was not added (eg tried to add a duplicate signal name)
		post("mapperDeviceAddOutput(): mdev_add_output() failed\n");
		// return nil
		SetNil(pa);
	}

	return errNone;

}

int mapperDeviceRemoveInput(struct VMGlobals *g, int numArgsPushed);
int mapperDeviceRemoveInput(struct VMGlobals *g, int numArgsPushed)
{

	PyrSlot *a = g->sp - 1;
	PyrSlot *b = g->sp;

	mapper_device dev;
	mapper_signal sig;
	PyrObject *signalobj;

	signalobj = slotRawObject(b);

	dev = Mapper::getDeviceStruct(a)->m_dev;
	sig = (mapper_signal) slotRawPtr( signalobj->slots+0 );

	if ( msig_properties(sig)->is_output ) {
		post("mapperDeviceRemoveInput() failed: signal is not an input\n");
		return errFailed;
	}

	mdev_remove_input( dev, sig );

	// set the dataptr in the MapperSignal object to NULL to indicate that it's
	// been freed
	SetPtr(signalobj->slots+0, NULL);

	return errNone;

}

int mapperDeviceRemoveOutput(struct VMGlobals *g, int numArgsPushed);
int mapperDeviceRemoveOutput(struct VMGlobals *g, int numArgsPushed)
{

	PyrSlot *a = g->sp - 1;
	PyrSlot *b = g->sp;

	mapper_device dev;
	mapper_signal sig;
	PyrObject *signalobj;

	signalobj = slotRawObject(b);

	dev = Mapper::getDeviceStruct(a)->m_dev;
	sig = (mapper_signal) slotRawPtr( signalobj->slots+0 );

	if ( ! msig_properties(sig)->is_output ) {
		post("mapperDeviceRemoveOutput() failed: signal is not an output\n");
		return errFailed;
	}

	mdev_remove_output( dev, sig );

	// set the dataptr in the MapperSignal object to NULL to indicate that it's
	// been freed
	SetPtr(signalobj->slots+0, NULL);

	return errNone;

}

int mapperDeviceStartPolling(struct VMGlobals *g, int numArgsPushed);
int mapperDeviceStartPolling(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;

	Mapper::Device *devstruct = Mapper::getDeviceStruct(a);

	// FIXME the way this works now, there's one thread per Mapper instance
	// there should only be one thread of all of them; make a manager class or
	// something?
	pthread_create( &devstruct->m_thread, NULL, Mapper::Device::polling_loop, (void*)devstruct );

	return errNone;
}

int mapperDeviceStopPolling(struct VMGlobals *g, int numArgsPushed);
int mapperDeviceStopPolling(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;

	Mapper::Device *devstruct = Mapper::getDeviceStruct(a);

	devstruct->m_polling = false;
	pthread_join(devstruct->m_thread, 0);

	return errNone;
}

int mapperDeviceIsPolling(struct VMGlobals *g, int numArgsPushed);
int mapperDeviceIsPolling(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	Mapper::Device *devstruct = Mapper::getDeviceStruct(a);
	SetBool( a, devstruct->m_polling );
	return errNone;
}

int mapperDeviceGetName(struct VMGlobals *g, int numArgsPushed);
int mapperDeviceGetName(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	mapper_device dev = Mapper::getDeviceStruct(a)->m_dev;
	const char *name = mdev_name(dev);
	if( name )
		SetSymbol( a, getsym(name) );
	else
		SetNil(a);
	return errNone;
}

int mapperDeviceGetPort(struct VMGlobals *g, int numArgsPushed);
int mapperDeviceGetPort(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	mapper_device dev = Mapper::getDeviceStruct(a)->m_dev;
	SetInt( a, mdev_port(dev) );
	return errNone;
}

int mapperSignalSetMinimum(struct VMGlobals *g, int numArgsPushed);
int mapperSignalSetMinimum(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp - 1;
	PyrSlot *b = g->sp;

	float minfloat;
	int minint;
	void *min = NULL;

	mapper_signal sig = (mapper_signal) slotRawPtr( slotRawObject(a)->slots+0 );

	int err = numericSlotToPointer( b, msig_properties(sig)->type, &minfloat, &minint, &min );
	if (err) return errWrongType;

	msig_set_minimum( sig, min );

	return errNone;
}

int mapperSignalSetMaximum(struct VMGlobals *g, int numArgsPushed);
int mapperSignalSetMaximum(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp - 1;
	PyrSlot *b = g->sp;

	float maxfloat;
	int maxint;
	void *max = NULL;

	mapper_signal sig = (mapper_signal) slotRawPtr( slotRawObject(a)->slots+0 );

	int err = numericSlotToPointer( b, msig_properties(sig)->type, &maxfloat, &maxint, &max );
	if (err) return errWrongType;

	msig_set_maximum( sig, max );

	return errNone;
}

int mapperSignalIsOutput(struct VMGlobals *g, int numArgsPushed);
int mapperSignalIsOutput(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	mapper_signal sig = (mapper_signal) slotRawPtr( slotRawObject(a)->slots+0 );
	SetBool( a, msig_properties(sig)->is_output );
	return errNone;
}

int mapperSignalGetType(struct VMGlobals *g, int numArgsPushed);
int mapperSignalGetType(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	mapper_signal sig = (mapper_signal) slotRawPtr( slotRawObject(a)->slots+0 );
	SetChar( a, msig_properties(sig)->type );
	return errNone;
}

int mapperSignalGetLength(struct VMGlobals *g, int numArgsPushed);
int mapperSignalGetLength(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	mapper_signal sig = (mapper_signal) slotRawPtr( slotRawObject(a)->slots+0 );
	SetInt( a, msig_properties(sig)->length );
	return errNone;
}

int mapperSignalGetName(struct VMGlobals *g, int numArgsPushed);
int mapperSignalGetName(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	mapper_signal sig = (mapper_signal) slotRawPtr( slotRawObject(a)->slots+0 );
	SetSymbol( a, getsym(msig_properties(sig)->name) );
	return errNone;
}

int mapperSignalGetDeviceName(struct VMGlobals *g, int numArgsPushed);
int mapperSignalGetDeviceName(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	mapper_signal sig = (mapper_signal) slotRawPtr( slotRawObject(a)->slots+0 );
	SetSymbol( a, getsym(msig_properties(sig)->device_name) );
	return errNone;
}

int mapperSignalGetUnit(struct VMGlobals *g, int numArgsPushed);
int mapperSignalGetUnit(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	mapper_signal sig = (mapper_signal) slotRawPtr( slotRawObject(a)->slots+0 );
	SetSymbol( a, getsym(msig_properties(sig)->unit) );
	return errNone;
}

int mapperSignalGetMinimum(struct VMGlobals *g, int numArgsPushed);
int mapperSignalGetMinimum(struct VMGlobals *g, int numArgsPushed)
{

	PyrSlot *a = g->sp;
	mapper_signal sig = (mapper_signal) slotRawPtr( slotRawObject(a)->slots+0 );
	mapper_db_signal props = msig_properties(sig);

	return pointerToNumericSlot( props->type, props->minimum, a );

}

int mapperSignalGetMaximum(struct VMGlobals *g, int numArgsPushed);
int mapperSignalGetMaximum(struct VMGlobals *g, int numArgsPushed)
{

	PyrSlot *a = g->sp;
	mapper_signal sig = (mapper_signal) slotRawPtr( slotRawObject(a)->slots+0 );
	mapper_db_signal props = msig_properties(sig);

	return pointerToNumericSlot( props->type, props->maximum, a );

}

int mapperSignalUpdate(struct VMGlobals *g, int numArgsPushed);
int mapperSignalUpdate(struct VMGlobals *g, int numArgsPushed)
{

	PyrSlot *a = g->sp - 1;
	PyrSlot *b = g->sp;

	mapper_signal sig = (mapper_signal) slotRawPtr( slotRawObject(a)->slots+0 );
	mapper_db_signal props = msig_properties(sig);

	float floatstorage;
	int intstorage;
	void *storagepointer;

	int err = numericSlotToPointer( b, props->type, &floatstorage, &intstorage, &storagepointer );
	if ( err != errNone ) return err;

	msig_update( sig, storagepointer, 0, MAPPER_NOW );

	return errNone;

}

int mapperSignalGetFullName(struct VMGlobals *g, int numArgsPushed);
int mapperSignalGetFullName(struct VMGlobals *g, int numArgsPushed)
{

	PyrSlot *a = g->sp;
	char name[1024];
	mapper_signal sig = (mapper_signal) slotRawPtr( slotRawObject(a)->slots+0 );
	int length;

	length = msig_full_name(sig, name, 1024);

	if ( length > 0 ) {
		SetSymbol( a, getsym(name) );
	}
	else {
		SetNil(a);
	}

	return errNone;

}

void initMapperPrimitives()
{

	int base, index = 0;

	base = nextPrimitiveIndex();

	definePrimitive(base, index++, "_MapperDeviceNew", mapperDeviceNew, 3, 0);
	definePrimitive(base, index++, "_MapperDeviceFree", mapperDeviceFree, 1, 0);
	definePrimitive(base, index++, "_MapperDeviceAddInput", mapperDeviceAddInput, 8, 0);
	definePrimitive(base, index++, "_MapperDeviceAddOutput", mapperDeviceAddOutput, 8, 0);
	definePrimitive(base, index++, "_MapperDeviceRemoveInput", mapperDeviceRemoveInput, 2, 0);
	definePrimitive(base, index++, "_MapperDeviceRemoveOutput", mapperDeviceRemoveOutput, 2, 0);
	definePrimitive(base, index++, "_MapperDeviceStartPolling", mapperDeviceStartPolling, 1, 0);
	definePrimitive(base, index++, "_MapperDeviceStopPolling", mapperDeviceStopPolling, 1, 0);
	definePrimitive(base, index++, "_MapperDeviceIsPolling", mapperDeviceIsPolling, 1, 0);
	definePrimitive(base, index++, "_MapperDeviceGetName", mapperDeviceGetName, 1, 0);
	definePrimitive(base, index++, "_MapperDeviceGetPort", mapperDeviceGetPort, 1, 0);

	definePrimitive(base, index++, "_MapperSignalSetMinimum", mapperSignalSetMinimum, 2, 0);
	definePrimitive(base, index++, "_MapperSignalSetMaximum", mapperSignalSetMaximum, 2, 0);
	definePrimitive(base, index++, "_MapperSignalIsOutput", mapperSignalIsOutput, 1, 0);
	definePrimitive(base, index++, "_MapperSignalGetType", mapperSignalGetType, 1, 0);
	definePrimitive(base, index++, "_MapperSignalGetLength", mapperSignalGetLength, 1, 0);
	definePrimitive(base, index++, "_MapperSignalGetName", mapperSignalGetName, 1, 0);
	definePrimitive(base, index++, "_MapperSignalGetDeviceName", mapperSignalGetDeviceName, 1, 0);
	definePrimitive(base, index++, "_MapperSignalGetUnit", mapperSignalGetUnit, 1, 0);
	definePrimitive(base, index++, "_MapperSignalGetMinimum", mapperSignalGetMinimum, 1, 0);
	definePrimitive(base, index++, "_MapperSignalGetMaximum", mapperSignalGetMaximum, 1, 0);
	definePrimitive(base, index++, "_MapperSignalUpdate", mapperSignalUpdate, 2, 0);
	definePrimitive(base, index++, "_MapperSignalGetFullName", mapperSignalGetFullName, 1, 0);

	s_callAction = getsym("prCallAction");

}


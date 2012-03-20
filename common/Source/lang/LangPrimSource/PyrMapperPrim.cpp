
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

		++g->sp; SetObject(g->sp, obj);

		runInterpreter(g, s_dispatchInputAction, 1);

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
		usleep(200);
	}
}

int mapperInit(struct VMGlobals *g, int numArgsPushed);
int mapperInit(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp - 1;
	PyrSlot *b = g->sp;

	// allocate a new Device class instance
	Mapper::Device *devstruct = new Mapper::Device;

	int err, portrequested;
	err = slotIntVal(b, &portrequested);
	if (err) return errWrongType;

	// poplate the device field in the Device instance
	devstruct->m_dev = mdev_new("supercollider", portrequested, 0);

	SetPtr(slotRawObject(a)->slots+0, devstruct);

	return errNone;
}

float currentvalue = 0.0;
float min0;
float max1000;

int mapperAddInput(struct VMGlobals *g, int numArgsPushed);
int mapperAddInput(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;

	PyrObject *obj = slotRawObject(a);
	mapper_device dev = Mapper::getDeviceStruct( a )->m_dev;

	// register the callback for the input.  store the object pointer in the
	// user data for the callback, which will be passed to invocations of the
	// callback.
	mdev_add_input(dev, "/freq", 1, 'f', 0, &min0, &max1000, Mapper::Device::input_handler, obj);

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
	SetFloat(receiver, currentvalue);

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

	definePrimitive(base, index++, "_MapperInit", mapperInit, 2, 0);
	definePrimitive(base, index++, "_MapperAddInput", mapperAddInput, 1, 0);
	definePrimitive(base, index++, "_MapperStart", mapperStart, 1, 0);
	definePrimitive(base, index++, "_MapperStop", mapperStop, 1, 0);
	definePrimitive(base, index++, "_MapperDevFree", mapperDevFree, 1, 0);
	definePrimitive(base, index++, "_MapperGetCurrentValue", mapperGetCurrentValue, 1, 0);
	definePrimitive(base, index++, "_MapperPort", mapperPort, 1, 0);

	s_dispatchInputAction = getsym("prDispatchInputAction");

}

// vim:sw=4:ts=4:

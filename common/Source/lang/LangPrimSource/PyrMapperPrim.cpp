
#include "PyrObject.h"
#include "PyrPrimitive.h"
#include "VMGlobals.h"

#include <mapper/mapper.h>

class Mapper
{

public:
	Mapper();
	//~Mapper() {}

	static void input_handler( mapper_signal msig, mapper_db_signal props, mapper_timetag_t *timetag, void *value );

	static inline mapper_device getDev( PyrSlot* slot )
	{
		// Get the PyrObject pointer from the argument, then get that object's
		// first slot (which is a pointer to the internal C++ data structure),
		// then return the mapper_device field from that
		return ( (Mapper*) slotRawPtr( &slotRawObject(slot)->slots[0] ) )->m_dev;
	}

	mapper_device m_dev;

};

Mapper::Mapper( void ) {
	m_dev = NULL;
}

void Mapper::input_handler( mapper_signal msig, mapper_db_signal props, mapper_timetag_t *timetag, void *value ) {
	printf("Mapper::input_handler() called\n");
}

int mapperInit(struct VMGlobals *g, int numArgsPushed);
int mapperInit(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp - 1;
	PyrSlot *b = g->sp;

	// allocate a new Mapper class instance
	Mapper *mdev = new Mapper;

	int err, portrequested;
	err = slotIntVal(b, &portrequested);
	if (err) return errWrongType;

	// poplate the device field in the Mapper instance
	mdev->m_dev = mdev_new("supercollider", portrequested, 0);

	SetPtr(slotRawObject(a)->slots+0, mdev);

	return errNone;
}

float currentvalue = 0.0;
float min0;
float max1000;

int mapperAddInput(struct VMGlobals *g, int numArgsPushed);
int mapperAddInput(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	mapper_device dev = Mapper::getDev( a );
    mdev_add_input(dev, "/freq", 1, 'f', 0, &min0, &max1000, Mapper::input_handler, NULL);
	return errNone;
}

int mapperPoll(struct VMGlobals *g, int numArgsPushed);
int mapperPoll(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	mapper_device dev = Mapper::getDev( a );
	int numhandled = 1;
	while ( numhandled > 0 ) {
		numhandled = mdev_poll(dev, 0);
	}
	return errNone;
}

int mapperDevFree(struct VMGlobals *g, int numArgsPushed);
int mapperDevFree(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;

	Mapper *mapperdatastructure = (Mapper*) slotRawPtr( &slotRawObject(a)->slots[0] );
	mapper_device dev = mapperdatastructure->m_dev;

	mdev_free( dev );
	mapperdatastructure->m_dev = NULL;

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
	mapper_device dev = Mapper::getDev( a );
	SetInt( a, mdev_port( dev ) );
	return errNone;
}

void initMapperPrimitives()
{
	int base, index = 0;

	base = nextPrimitiveIndex();

	// libmapper
	definePrimitive(base, index++, "_MapperInit", mapperInit, 2, 0);
	definePrimitive(base, index++, "_MapperAddInput", mapperAddInput, 1, 0);
	definePrimitive(base, index++, "_MapperPoll", mapperPoll, 1, 0);
	definePrimitive(base, index++, "_MapperDevFree", mapperDevFree, 1, 0);
	definePrimitive(base, index++, "_MapperGetCurrentValue", mapperGetCurrentValue, 1, 0);
	definePrimitive(base, index++, "_MapperPort", mapperPort, 1, 0);

	// add getsym()s you need here

}

// vim:sw=4:ts=4:


#include "PyrObject.h"
#include "PyrPrimitive.h"
#include "VMGlobals.h"

#include <mapper/mapper.h>
#include <lo/lo.h>

class Mapper
{

public:
	//Mapper() {}
	//~Mapper() {}

//private:
	mapper_device m_dev;

};

int mapperNew(struct VMGlobals *g, int numArgsPushed);
int mapperNew(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	SetPtr(slotRawObject(a)->slots+0, new Mapper);
	return errNone;
}

int mapperDevNew(struct VMGlobals *g, int numArgsPushed);
int mapperDevNew(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	Mapper *mapperdata = (Mapper*)slotRawPtr(&slotRawObject(a)->slots[0]);
	mapperdata->m_dev = mdev_new("supercollider", 9444, 0);
	return errNone;
}

float min0 = 0;
float max1000 = 1000;

float currentvalue = 0.0;

void handler_freq(mapper_signal sig, mapper_db_signal props, mapper_timetag_t *timetag, void *pfreq)
{
	// store the value in a local variable to poll later
	currentvalue = *(float*)pfreq;
}

int mapperAddInput(struct VMGlobals *g, int numArgsPushed);
int mapperAddInput(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	Mapper *mapperdata = (Mapper*)slotRawPtr(&slotRawObject(a)->slots[0]);
    mdev_add_input(mapperdata->m_dev, "/freq", 1, 'f', 0, &min0, &max1000, handler_freq, NULL);
	return errNone;
}

int mapperPoll(struct VMGlobals *g, int numArgsPushed);
int mapperPoll(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	Mapper *mapperdata = (Mapper*)slotRawPtr(&slotRawObject(a)->slots[0]);
	int numhandled = 1;
	while ( numhandled > 0 ) {
		numhandled = mdev_poll(mapperdata->m_dev, 0);
	}
	return errNone;
}

int mapperDevFree(struct VMGlobals *g, int numArgsPushed);
int mapperDevFree(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	Mapper *mapperdata = (Mapper*)slotRawPtr(&slotRawObject(a)->slots[0]);
	mdev_free( mapperdata->m_dev );
	mapperdata->m_dev = NULL;
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

void initMapperPrimitives()
{
	int base, index = 0;

	base = nextPrimitiveIndex();

	// libmapper
	definePrimitive(base, index++, "_MapperNew", mapperNew, 1, 0);
	definePrimitive(base, index++, "_MapperDevNew", mapperDevNew, 1, 0);
	definePrimitive(base, index++, "_MapperAddInput", mapperAddInput, 1, 0);
	definePrimitive(base, index++, "_MapperPoll", mapperPoll, 1, 0);
	definePrimitive(base, index++, "_MapperDevFree", mapperDevFree, 1, 0);
	definePrimitive(base, index++, "_MapperGetCurrentValue", mapperGetCurrentValue, 1, 0);

	// add getsym()s you need here

}

// vim:sw=4:ts=4:

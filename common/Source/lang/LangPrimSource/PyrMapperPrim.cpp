
#include "PyrObject.h"
#include "PyrPrimitive.h"
#include "VMGlobals.h"

#include <mapper/mapper.h>

namespace Mapper {

	struct Device
	{

		Device( mapper_device dev ) : m_dev( dev ) {}

		mapper_device m_dev;

	};

	static inline Device* getDeviceStruct( PyrSlot* slot )
	{
		// Get the PyrObject pointer from the argument, then get that object's
		// first slot, which is a pointer to the internal C++ data structure
		return (Device*) slotRawPtr( slotRawObject(slot)->slots+0 );
	}

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
	// FIXME where will this get freed?

	SetPtr(slotRawObject(a)->slots+0, devstruct);

	return errNone;
}

int mapperDeviceFree(struct VMGlobals *g, int numArgsPushed);
int mapperDeviceFree(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;

	mapper_device dev = Mapper::getDeviceStruct(a)->m_dev;

	mdev_free( dev );

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
	mapper_signal sig = mdev_add_input( dev, signalname, length, type, unit, &min, &max, NULL, signalobj );

	// set the dataptr field in signalobj to hold the mapper_signal pointer
	SetPtr(signalobj->slots+0, sig);

	// FIXME return the signalobj on the stack -- need to figure out how to do this

	return errNone;
}

void initMapperPrimitives()
{

	int base, index = 0;

	base = nextPrimitiveIndex();

	definePrimitive(base, index++, "_MapperDeviceNew", mapperDeviceNew, 3, 0);
	definePrimitive(base, index++, "_MapperDeviceFree", mapperDeviceFree, 1, 0);
	definePrimitive(base, index++, "_MapperDeviceAddInput", mapperDeviceAddInput, 8, 0);

}


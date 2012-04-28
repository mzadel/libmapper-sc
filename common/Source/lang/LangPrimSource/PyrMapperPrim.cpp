
#include "PyrPrimitive.h"

int mapperDeviceNew(struct VMGlobals *g, int numArgsPushed);
int mapperDeviceNew(struct VMGlobals *g, int numArgsPushed)
{
	return errNone;
}

int mapperDeviceFree(struct VMGlobals *g, int numArgsPushed);
int mapperDeviceFree(struct VMGlobals *g, int numArgsPushed)
{
	return errNone;
}

int mapperDeviceAddInput(struct VMGlobals *g, int numArgsPushed);
int mapperDeviceAddInput(struct VMGlobals *g, int numArgsPushed)
{
	return errNone;
}

void initMapperPrimitives()
{
	int base, index = 0;

	base = nextPrimitiveIndex();

	definePrimitive(base, index++, "_MapperDeviceNew", mapperDeviceNew, 1, 0);
	definePrimitive(base, index++, "_MapperDeviceFree", mapperDeviceFree, 1, 0);
	definePrimitive(base, index++, "_MapperDeviceAddInput", mapperDeviceAddInput, 1, 0);

}



MapperDevice {

	var dataptr;

	prNew { arg devicename, port;
		_MapperDeviceNew
		^this.primitiveFailed
	}

	prFree {
		_MapperDeviceFree
		^this.primitiveFailed
	}

	prAddInput {
		_MapperDeviceAddInput
		^this.primitiveFailed
	}

}

MapperSignal {

	var dataptr, <>action;

}


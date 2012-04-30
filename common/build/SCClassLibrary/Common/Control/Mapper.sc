
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

	prAddInput { arg name, length, type, unit, min, maximum, signalobj;
		_MapperDeviceAddInput
		^this.primitiveFailed
	}

	startPolling {
		_MapperStartPolling;
		^this.primitiveFailed
	}

	stopPolling {
		_MapperStopPolling;
		^this.primitiveFailed
	}

}

MapperSignal {

	var dataptr, <>action;

}


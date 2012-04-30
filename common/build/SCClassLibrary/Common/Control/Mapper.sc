
MapperDevice {

	var dataptr;

	*new { arg devicename = 'supercollider', port = 9444;
		^super.new.init( devicename, port )
	}

	init { arg port, devicename;
		this.prNew( devicename, port );
	}

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

	isPolling {
		_MapperIsPolling
		^this.primitiveFailed
	}

}

MapperSignal {

	var dataptr, <>action;

	prCallAction { arg name, value;
		action.value( value );
	}

}


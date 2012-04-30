
MapperDevice {

	var dataptr, signals;

	*new { arg devicename = 'supercollider', port = 9444;
		^super.new.init( devicename, port )
	}

	init { arg devicename, port;
		signals = [];
		this.prNew( devicename, port );
		this.prStartPolling;
	}

	free {
		this.prStopPolling;
		signals = [];
		this.prFree;
	}

	addInput { arg name, length, type, unit, min, maximum, action;
		var sig;
		sig = this.prAddInput( name, length, type, unit, min, maximum, MapperSignal.new );
		sig.action = action;
		signals = signals.add(sig);
		^sig;
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

	prStartPolling {
		_MapperDeviceStartPolling;
		^this.primitiveFailed
	}

	prStopPolling {
		_MapperDeviceStopPolling;
		^this.primitiveFailed
	}

	prIsPolling {
		_MapperDeviceIsPolling
		^this.primitiveFailed
	}

}

MapperSignal {

	var <>action;

	getName {
		_MapperSignalGetName
		^this.primitiveFailed
	}

	getDeviceName {
		_MapperSignalGetDeviceName
		^this.primitiveFailed
	}

	getMinimum {
		_MapperSignalGetMinimum
		^this.primitiveFailed
	}

	getMaximum {
		_MapperSignalGetMaximum
		^this.primitiveFailed
	}

	prCallAction { arg name, value;
		action.value( name, value );
	}

}



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
		_MapperStartPolling;
		^this.primitiveFailed
	}

	prStopPolling {
		_MapperStopPolling;
		^this.primitiveFailed
	}

	prIsPolling {
		_MapperIsPolling
		^this.primitiveFailed
	}

}

MapperSignal {

	var <>action;

	prCallAction { arg name, value;
		action.value( name, value );
	}

}


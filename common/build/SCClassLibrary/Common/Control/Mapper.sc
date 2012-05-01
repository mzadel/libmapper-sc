
MapperDevice {

	var dataptr, <signals;

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

	addInput { arg name, length, type, unit, min, max, action;
		var sig;
		sig = this.prAddInput( name, length, type, unit, min, max, MapperSignal.new );
		if ( sig.notNil ) {
			sig.action = action;
			signals = signals.add(sig);
		};
		^sig;
	}

	addOutput { arg name, length, type, unit, min, max;
		var sig;
		sig = this.prAddOutput( name, length, type, unit, min, max, MapperSignal.new );
		if ( sig.notNil ) { signals = signals.add(sig) };
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

	prAddInput { arg name, length, type, unit, min, max, signalobj;
		_MapperDeviceAddInput
		^this.primitiveFailed
	}

	prAddOutput { arg name, length, type, unit, min, max, signalobj;
		_MapperDeviceAddOutput
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

	getName {
		_MapperDeviceGetName
		^this.primitiveFailed
	}

	getPort {
		_MapperDeviceGetPort
		^this.primitiveFailed
	}

}

MapperSignal {

	var dataptr, <>action;

	isOutput {
		_MapperSignalIsOutput
		^this.primitiveFailed
	}

	getType {
		_MapperSignalGetType
		^this.primitiveFailed
	}

	getLength {
		_MapperSignalGetLength
		^this.primitiveFailed
	}

	getName {
		_MapperSignalGetName
		^this.primitiveFailed
	}

	getDeviceName {
		_MapperSignalGetDeviceName
		^this.primitiveFailed
	}

	getUnit {
		_MapperSignalGetUnit
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


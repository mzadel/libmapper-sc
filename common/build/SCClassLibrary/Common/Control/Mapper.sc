
MapperDevice {

	var dataptr, <inputsignals, <outputsignals;

	*new { arg devicename = 'supercollider', port = 9444;
		^super.new.init( devicename, port )
	}

	init { arg devicename, port;
		inputsignals = [];
		outputsignals = [];
		this.prNew( devicename, port );
		this.prStartPolling;
	}

	free {
		this.prStopPolling;
		inputsignals = [];
		outputsignals = [];
		this.prFree;
	}

	addInput { arg name, length, type, unit, min, max, action;
		var sig;
		sig = this.prAddInput( name, length, type, unit, min, max, MapperSignal.new );
		if ( sig.notNil ) {
			sig.action = action;
			inputsignals = inputsignals.add(sig);
		};
		^sig;
	}

	addOutput { arg name, length, type, unit, min, max;
		var sig;
		sig = this.prAddOutput( name, length, type, unit, min, max, MapperSignal.new );
		if ( sig.notNil ) { outputsignals = outputsignals.add(sig) };
		^sig;
	}

	removeInput { arg name;
		var sigtoremove;
		sigtoremove = inputsignals.detect( { var sig; sig.getName == name } );
		if ( sigtoremove.notNil ) {
			inputsignals.remove(sigtoremove);
			this.prRemoveInput(sigtoremove);
		}
	}

	removeOutput { arg name;
		var sigtoremove;
		sigtoremove = outputsignals.detect( { var sig; sig.getName == name } );
		if ( sigtoremove.notNil ) {
			outputsignals.remove(sigtoremove);
			this.prRemoveOutput(sigtoremove);
		}
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

	prRemoveInput { arg signalobj;
		_MapperDeviceRemoveInput
		^this.primitiveFailed
	}

	prRemoveOutput { arg signalobj;
		_MapperDeviceRemoveOutput
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

	setMinimum { arg value;
		_MapperSignalSetMinimum
		^this.primitiveFailed
	}

	setMaximum { arg value;
		_MapperSignalSetMaximum
		^this.primitiveFailed
	}

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

	update { arg value;
		_MapperSignalUpdate
		^this.primitiveFailed
	}

	getFullName {
		_MapperSignalGetFullName
		^this.primitiveFailed
	}

	prCallAction { arg name, value;
		action.value( name, value );
	}

}


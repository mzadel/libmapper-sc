
Mapper {

	var <dataptr, <actions;

	*new { arg port = 9444, devicename='supercollider';
		^super.new.init( port, devicename )
	}

	init { arg port, devicename;
		actions = IdentityDictionary.new;
		this.prInit( port, devicename )
	}

	addInput { arg name, type, min, max, action;
		actions[name] = action;
		this.prAddInput( name, type, min, max );
	}

	startPolling {
		_MapperStartPolling;
		^this.primitiveFailed
	}

	stopPolling {
		_MapperStopPolling;
		^this.primitiveFailed
	}

	devFree {
		_MapperDevFree
		^this.primitiveFailed
	}

	getCurrentValue {
		_MapperGetCurrentValue
		^this.primitiveFailed
	}

	port {
		_MapperPort
		^this.primitiveFailed
	}

	isPolling {
		_MapperIsPolling
		^this.primitiveFailed
	}

	prInit { arg port, devicename;
		_MapperInit
		^this.primitiveFailed
	}

	prAddInput { arg name, type, min, max;
		_MapperAddInput
		^this.primitiveFailed
	}

	// this is called when a libmapper signal message is received
	prDispatchInputAction { arg name, instanceid, value;
		actions[name].value( instanceid, value );
	}

}


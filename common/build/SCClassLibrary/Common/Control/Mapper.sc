
Mapper {

	var <dataptr, <actions;

	*new { arg port = 9444, devicename='supercollider';
		^super.new.init( port, devicename )
	}

	init { arg port, devicename;
		actions = IdentityDictionary.new;
		this.prMapperInit( port, devicename )
	}

	mapperAddInput { arg name, type, min, max, action;
		actions[name] = action;
		this.prMapperAddInput( name, type, min, max );
	}

	mapperStart {
		_MapperStart;
		^this.primitiveFailed
	}

	mapperStop {
		_MapperStop;
		^this.primitiveFailed
	}

	mapperDevFree {
		_MapperDevFree
		^this.primitiveFailed
	}

	mapperGetCurrentValue {
		_MapperGetCurrentValue
		^this.primitiveFailed
	}

	mapperPort {
		_MapperPort
		^this.primitiveFailed
	}

	isRunning {
		_MapperIsRunning
		^this.primitiveFailed
	}

	prMapperInit { arg port, devicename;
		_MapperInit
		^this.primitiveFailed
	}

	prMapperAddInput { arg name, type, min, max;
		_MapperAddInput
		^this.primitiveFailed
	}

	// this is called when a libmapper signal message is received
	prDispatchInputAction { arg name, value;
		actions[name].value( value );
	}

}



Mapper {

	var <dataptr, <actions;

	*new { arg port = 9444;
		^super.new.init( port )
	}

	init { arg port;
		actions = IdentityDictionary.new;
		this.prMapperInit( port )
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

	prMapperInit {
		_MapperInit
		^this.primitiveFailed
	}

	prMapperAddInput { arg name, type, min, max;
		_MapperAddInput
		^this.primitiveFailed
	}

	prDispatchInputAction { arg name, value;
		actions[name].value( value );
	}

}



Mapper {

	var <dataptr;

	*new { arg port = 9444;
		^super.new.init( port )
	}

	init { arg port;
		this.prMapperInit( port )
	}

	mapperAddInput {
		_MapperAddInput
		^this.primitiveFailed
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

	prDispatchInputAction { arg name, value;
		"prDispatchInputAction() called".postln;
		name.postln;
		value.postln;
	}

}


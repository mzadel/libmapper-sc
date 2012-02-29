
Mapper {

	var <dataptr;

	*new { arg port = 9444;
		^super.new.init( port )
	}

	init { arg port;
		this.prMapperInit( port )
	}

	// libmapper
	// FIXME *this* is the call that should be getting the port number
	// FIXME decide if you're going to accept the port here or at instantiation
	mapperDevNew {
		_MapperDevNew
		^this.primitiveFailed
	}

	mapperAddInput {
		_MapperAddInput
		^this.primitiveFailed
	}

	mapperPoll {
		_MapperPoll
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

}


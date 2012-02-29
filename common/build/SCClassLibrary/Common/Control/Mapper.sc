
Mapper {

	var <dataptr;

	*new {
		^super.new.init
	}

	init {
		this.prMapperInit;
	}

	// libmapper
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


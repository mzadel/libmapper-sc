
Mapper {

	var <dataptr;

	*new {
		^super.new.init
	}

	init {
		this.prMapperNew;
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

	prMapperNew {
		_MapperInit
		^this.primitiveFailed
	}

}


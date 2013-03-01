
//
// libmapper-sc example code
// Mark Zadel 2013
//
// This is an example of how to use the the SuperCollider bindings for
// libmapper.
//
// see http://libmapper.org/ for further information
//
// The three basics steps are:
//  1 allocate a MapperDevice
//  2 add signals to it
//  - (map to those signals from outside SuperCollider using libmapper, and do
//    whatever work you want to do)
//  3 free the device
//


a = MapperDevice.new;
// can also provide the desired device name and desired port number as
// arguments, eg: a = MapperDevice( "mydevicename", 9999 );

// you have to wait briefly for the mapper device name and port to get
// allocated before these will return the exepcted values
a.getName;
a.getPort;

// add input and output signals to our device
// NB a mapper device will not be visible on the mapper network (ie, in the
// mapper gui) until at least one signal is added
b = a.addInput( '/insig', 1, $f, \cents, 1.neg, 15.7, { arg signame, instanceid, value; signame.postln; instanceid.postln; value.postln } );
c = a.addOutput( '/outsigone', 1, $i, \ceeents, 22, 33 );
d = a.addOutput( '/outsigtwo', 3, $i, \ceeents, 22, 33 );

// return a list of input and output signals
a.inputsignals;
a.outputsignals;

// query the signal for its properties
b.isOutput;
b.getType;
b.getLength;
b.getName;
b.getDeviceName;
b.getUnit;
b.getMinimum;
b.getMaximum;
b.getFullName;

// change the signal's min and max
b.setMinimum( -10 );
b.setMaximum( 111 );

// send output on one of the output signals
c.update(22);
c.update(23);
c.update(23.1);
c.update(25);
c.update(30);
c.update(33);

// you can remove signals if you need to while you're experimenting.  (this is
// not necessary before freeing the device, however.)
a.removeInput( '/insig' );
a.removeOutput( '/outsigone' );

// need to free the mapper device when we're done so that the libmapper network
// knows that it's no longer active
a.free;


require( "../setup" );

var OicDevice = require( "../../index" ).OicDevice;

test( "Enable Presence", function( assert ) {
	var done = assert.async();

	expect( 1 );

	var device = OicDevice();
	var settings = {
		role: "server"
	}
	device.configure( settings );
	device._server.enablePresence( 1000 );
	device._server.disablePresence().then(
		function() {
			ok( true, "Presence Disabled successfully" );
			done();
		},
		function( error ) {
			ok( false, "Presence disabled error with code: " + error.result );
			done();
		} );
} );

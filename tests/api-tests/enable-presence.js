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

	device._server.enablePresence( 1000 ).then(
		function() {
			ok( true, "Presence Enabled successfully" );
			done();
		},
		function( error ) {
			ok( false, "Presence Enable error with code: " + error.result );
			done();
		} );
} );

var QUnit = require( "../setup" ),
	OicDevice = require( "../../index" ).OicDevice;

QUnit.test( "Enable Presence", function( assert ) {
	var done = assert.async();

	assert.expect( 1 );

	var device = OicDevice();
	var settings = {
		role: "server"
	}
	device.configure( settings );
	device._server.enablePresence( 1000 );
	device._server.disablePresence().then(
		function() {
			assert.ok( true, "Presence Disabled successfully" );
			done();
		},
		function( error ) {
			assert.ok( false, "Presence disabled error with code: " + error.result );
			done();
		} );
} );

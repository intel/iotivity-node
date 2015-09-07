var QUnit = require( "../setup" ),
	OicDevice = require( "../../index" ).OicDevice;

QUnit.test( "Enable Presence", function( assert ) {
	assert.expect( 1 );

	var done = assert.async();
	var device = OicDevice();
	var settings = {
		role: "server"
	};

	device.configure( settings );

	device._server.enablePresence( 1000 ).then(
		function() {
			assert.ok( true, "Presence Enabled successfully" );
			done();
		},
		function( error ) {
			assert.ok( false, "Presence Enable error with code: " + error.result );
			done();
		} );
} );

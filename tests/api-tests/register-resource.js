require( "../setup" );

var OicDevice = require( "../../index" ).OicDevice;

test( "Register Resource", function( assert ) {
	var done = assert.async();

	expect( 1 );

	var device = OicDevice();
	var settings = {
		role: "server",
		connectionMode: "acked",
		info: {
			uuid: "INTEL"
		}

	}
	device.configure( settings );

	var lightResource = null;
	if (device._settings.info.uuid) {
		var deviceId = device._settings.info.uuid;
		var connMode = device._settings.connectionMode;

		device._server.registerResource({
			url: "/light/ambience/blue",
			deviceId: deviceId,
			connectionMode: connMode,
			resourceTypes: [ "Light" ],
			interfaces: [ "/oic/if/rw" ],
			discoverable: true,
			observable: true,
			properties: { color: "light-blue", dimmer: 0.2 }
		}).then(
			function( resource ) {
				ok( true, "Resource registered successfully" );
				console.log ( resource );
				done();
			},
			function( error ) {
				ok( false, "Resource not registered, error with code: " + error.result );
				done();
			} );
	}
} );

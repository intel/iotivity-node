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
			url: "/a/light2",
			deviceId: deviceId,
			connectionMode: connMode,
			resourceTypes: [ "core.light" ],
			interfaces: [ "oc.mi.def" ],
			discoverable: true,
			observable: true,
			properties: { color: "light-blue", dimmer: 0.2 }
		}).then(
			function( resource ) {
				lightResource = resource;
				device.onrequest = requestHandler;
				console.log ( resource );
			},
			function( error ) {
				ok( false, "Resource not registered, error with code: " + error.result );
				done();
			} );
	}

	function requestHandler( request ) {
		if (request.type == "retrieve") {
			request.sendResponse(lightResource);
			ok ( true, "Get resource successfully responded")
		}
	}
} );

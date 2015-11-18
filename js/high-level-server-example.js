var lightResource,
	sensor = require( "./mock-sensor" )(),
	device = require( "iotivity-node" )(),
	settings = {
		role: "server",
		info: {
			name: "api-server-example",
			manufacturerName: "Intel",
			manufactureDate: "Wed Sep 23 10:04:17 EEST 2015",
			platformVersion: "0.9.2",
			firmwareVersion: "0.0.1",
			supportUrl: "http://example.com/"
		}
	};

device.configure( settings ).then( function() {

	sensor.on( "change", function( newData ) {
		var index;

		if ( !lightResource ) {
			return;
		}

		// Grab the updated data from the sensor and store it in the properties of the resource
		for ( index in newData ) {
			lightResource.properties[ index ] = newData[ index ];
		}

		device.notify( lightResource );
	} );

	function lightResourceOnRequest( request ) {
		if ( request.type === "retrieve" || request.type === "observe" ) {
			request.sendResponse( null );
		}
	}

	if ( device.settings.info.uuid ) {
		device.registerResource( {
			id: { path: "/a/high-level-example" },
			resourceTypes: [ "core.light" ],
			interfaces: [ "oic.if.baseline" ],
			discoverable: true,
			observable: true,
			properties: { someValue: 0, someOtherValue: "Helsinki" }
		} ).then(
			function( resource ) {
				lightResource = resource;
				device.addEventListener( "request", lightResourceOnRequest );
			},
			function( error ) {
				throw error;
			} );
	}
} );

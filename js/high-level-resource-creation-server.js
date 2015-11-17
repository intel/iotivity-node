var resourceCreatedByRemote,
	device = require( "iotivity-node" )(),
	_ = require( "lodash" ),
	settings = {
		role: "server",
		info: {
			uuid: "INTEL-server",
			name: "api-server-example",
			manufacturerName: "Intel",
			manufactureDate: "Wed Sep 23 10:04:17 EEST 2015",
			platformVersion: "0.9.2",
			firmwareVersion: "0.0.1",
			supportUrl: "http://example.com/"
		}
	};

function throwError( error ) {
	console.error( error.stack ? error.stack : ( error.message ? error.message : error ) );
	process.exit( 1 );
}

device.configure( settings ).then(
	function() {
		device.server.addEventListener( "request", function( request ) {
			console.log( JSON.stringify( request ), null, 4 );
			if ( request.type === "create" ) {
				console.log( "create request" );
				device.server.registerResource( _.extend( request.res, {
					discoverable: true
				} ) ).then( function( resource ) {
					console.log( "resource successfully registered" );
					resourceCreatedByRemote = resource;
					request.sendResponse( null );
				}, _.bind( request.sendError, request ) );
				return;
			} else if ( request.type === "delete" ) {
				console.log( "delete request" );
				if ( request.target === resourceCreatedByRemote.id ) {
					device.server.unregisterResource( resourceCreatedByRemote ).then(
						function() {
							console.log( "resource successfully deleted" );
							request.sendResponse( null );
						},
						_.bind( request.sendError, request ) );
				}
			}
		} );

		if ( device.settings.info.uuid ) {
			device.server.registerResource( {
				url: "/a/high-level-resource-creation-example",
				deviceId: device.settings.info.uuid,
				resourceTypes: [ "core.light" ],
				interfaces: [ "oic.if.baseline" ],
				discoverable: true,
				observable: true,
				properties: { someValue: 0, someOtherValue: "Helsinki" }
			} ).then( function() {
				console.log( "initial resource successfully registered" );
			}, throwError );
		}
	}, throwError );

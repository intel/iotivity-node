var resourceCreatedByRemote,
	device = require( "iotivity-node" )(),
	_ = require( "lodash" ),
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

function throwError( error ) {
	console.error( error.stack ? error.stack : ( error.message ? error.message : error ) );
	process.exit( 1 );
}

device.configure( settings ).then(
	function() {
		device.addEventListener( "request", function( request ) {
			console.log( JSON.stringify( request, null, 4 ) );
			if ( request.type === "create" ) {
				console.log( "create request" );
				device.registerResource( _.extend( request.res, {
					discoverable: true
				} ) ).then( function( resource ) {
					console.log( "resource successfully registered" );
					resourceCreatedByRemote = resource;
					request.sendResponse( null );
				}, _.bind( request.sendError, request ) );
				return;
			} else if ( request.type === "delete" ) {
				console.log( "delete request" );
				if ( request.target.id.path === resourceCreatedByRemote.id.path &&
						request.target.id.deviceId === resourceCreatedByRemote.id.deviceId ) {
					device.unregisterResource( resourceCreatedByRemote ).then(
						function() {
							console.log( "resource successfully deleted" );
							request.sendResponse( null );
						},
						_.bind( request.sendError, request ) );
				}
			}
		} );

		device.registerResource( {
			id: {
				path: "/a/high-level-resource-creation-example"
			},
			resourceTypes: [ "core.light" ],
			interfaces: [ "oic.if.baseline" ],
			discoverable: true,
			observable: true,
			properties: { someValue: 0, someOtherValue: "Helsinki" }
		} ).then( function() {
			console.log( "initial resource successfully registered" );
		}, throwError );
	}, throwError );

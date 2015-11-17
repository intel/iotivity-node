var device = require( "iotivity-node" )();

function throwError( error ) {
	console.error( error.stack ? error.stack : ( error.message ? error.message : error ) );
	process.exit( 1 );
}

device.configure( {
	role: "client"
} ).then(
	function() {

		new Promise( function( fulfill, reject ) {

			var resourceFoundHandler = function( event ) {
				console.log( "Discovered resource(s) via the following event:\n" +
					JSON.stringify( event, null, 4 ) );

				// We've discovered the resource we were seeking.
				if ( event.resource.uri === "/a/high-level-resource-creation-example" ) {
					console.log( "Found the test server" );
					device.client.removeEventListener( "resourcefound", resourceFoundHandler );
					fulfill( event.resource.id.deviceId );
				}
			};

			// Add a listener that will receive the results of the discovery
			device.client.addEventListener( "resourcefound", resourceFoundHandler );

			device.client.findResources().catch( function( error ) {
				device.client.removeEventListener( "resourcefound", resourceFoundHandler );
				reject( "findResource() failed: " + error );
			} );
		} ).then(
			function( deviceId ) {
				console.log( "deviceId discovered" );
				device.client.createResource( {
					id: {
						deviceId: deviceId,
						path: "/a/new-resource"
					},
					resourceTypes: [ "core.light" ],
					interfaces: [ "oic.if.baseline" ],
					properties: {
						exampleProperty: 23
					}
				} ).then(
					function( resource ) {
						console.log( "remote resource successfully created with id " +
							JSON.stringify( resource.id ) );
						device.client.deleteResource( resource.id ).then( function() {
							console.log( "remote resource successfully deleted" );
						}, throwError );
					},
					throwError );
			}, throwError );
	}, throwError );

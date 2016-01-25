var theResource,
	totalRequests = 0,
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )(),
	uuid = process.argv[ 2 ];

console.log( JSON.stringify( { assertionCount: 8 } ) );

function resourceOnRequest( request ) {
	totalRequests++;
	utils.assert( "strictEqual", request.type, "retrieve", "Server: Request is of type retrieve" );
	if ( request.type === "retrieve" ) {
		if ( totalRequests === 1 ) {
			request.sendResponse( request.target ).then(
				function() {
					utils.assert( "ok", true,
						"Server: Successfully responded to retrieve request" );
					device.unregisterResource( theResource ).then(
						function() {
							utils.assert( "ok", true,
								"Server: device.unregisterResource() successful" );
							theResource = null;
						},
						function( error ) {
							utils.die(
								"Server: device.unregisterResource() failed with: " + error +
								" and result " + error.result );
						} );
				},
				function( error ) {
					utils.die( "Server: Failed to send response with error " + error +
						" and result " + error.result );
				} );
		} else {
			request.sendError( new Error( "resource not found" ) ).then(
				function() {
					utils.assert( "ok", true,
						"Server: Successfully reported error to client" );
				},
				function( error ) {
					utils.die( "Server: Failed to report error to client: " + error +
						" and result " + error.result );
				} );
		}
	}
}

device.configure( {
	role: "server",
	info: {
		uuid: uuid,
		name: "api-retrieval-" + uuid,
		manufacturerName: "Intel"
	}
} ).then(
	function() {
		utils.assert( "ok", true, "Server: device.configure() successful" );

		device.registerResource( {
			id: { path: "/a/" + uuid },
			resourceTypes: [ "core.light" ],
			interfaces: [ "oic.if.baseline" ],
			discoverable: true,
			properties: {
				"How many angels can dance on the head of a pin?": "As many as wanting."
			}
		} ).then(
			function( resource ) {
				theResource = resource;
				utils.assert( "ok", true, "Server: device.registerResource() successful" );
				device.addEventListener( "request", resourceOnRequest );

				// Signal to the test suite that we're ready for the client
				console.log( JSON.stringify( { ready: true } ) );
			},
			function( error ) {
				utils.die( "Server: device.registerResource() failed with: " + error +
					" and result " + error.result );
			} );
	},
	function( error ) {
		utils.die( "Server: device.configure() failed with: " + error + " and result " +
			error.result );
	} );

// Cleanup on SIGINT
process.on( "SIGINT", function() {
	utils.assert( "strictEqual", totalRequests, 2,
		"Server: There have been exactly two requests" );
	process.exit( 0 );
} );

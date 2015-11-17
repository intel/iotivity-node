var theResource,
	totalRequests = 0,
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )(),
	uuid = process.argv[ 2 ];

console.log( JSON.stringify( { assertionCount: 6 } ) );

function resourceOnRequest( request ) {
	totalRequests++;
	utils.assert( "strictEqual", request.type, "retrieve", "Server: Request is of type retrieve" );
	if ( request.type === "retrieve" ) {
		request.sendResponse( request.source ).then(
			function() {
				utils.assert( "ok", true, "Server: Successfully responded to retrieve request" );
			},
			function( error ) {
				utils.die( "Server: Failed to send response with error " + error + " and result " +
					error.result );
			} );
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

		device.server.registerResource( {
			url: "/a/" + uuid,
			deviceId: uuid,
			resourceTypes: [ "core.light" ],
			interfaces: [ "oic.if.baseline" ],
			discoverable: true,
			properties: {
				"How many angels can dance on the head of a pin?": "As many as wanting."
			}
		} ).then(
			function( resource ) {
				theResource = resource;
				utils.assert( "ok", true, "Server: device.server.registerResource() successful" );
				device.server.addEventListener( "request", resourceOnRequest );

				// Signal to the test suite that we're ready for the client
				console.log( JSON.stringify( { ready: true } ) );
			},
			function( error ) {
				utils.die( "Server: device.server.registerResource() failed with: " + error +
					" and result " + error.result );
			} );
	},
	function( error ) {
		utils.die( "Server: device.configure() failed with: " + error + " and result " +
			error.result );
	} );

// Cleanup on SIGINT
process.on( "SIGINT", function() {
	utils.assert( "strictEqual", totalRequests, 1, "There has been exactly one request" );
	device.server.unregisterResource( theResource ).then(
		function() {
			utils.assert( "ok", true, "Server: device.server.unregisterResource() successful" );
			process.exit( 0 );
		},
		function( error ) {
			utils.die( "Server: device.server.unregisterResource() failed with: " + error +
				" and result " + error.result );
			process.exit( 0 );
		} );
} );

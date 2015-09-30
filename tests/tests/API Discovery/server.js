var theResource,
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )(),
	uuid = process.argv[ 2 ];

console.log( JSON.stringify( { assertionCount: 3 } ) );

device.configure( {
	role: "server",
	connectionMode: "acked",
	info: {
		uuid: uuid,
		name: "api-discovery-" + uuid,
		manufacturerName: "Intel"
	}
} ).then(
	function() {
		utils.assert( "ok", true, "Server: device.configure() successful" );

		device._server.registerResource( {
			url: "/a/" + uuid,
			deviceId: uuid,
			connectionMode: "acked",
			resourceTypes: [ "core.light" ],
			interfaces: [ "oic.if.baseline" ],
			discoverable: true,
			properties: { someValue: 0 }
		} ).then(
			function( resource ) {
				theResource = resource;
				utils.assert( "ok", true, "Server: device._server.registerResource() successful" );

				// Signal to the test suite that we're ready for the client
				console.log( JSON.stringify( { ready: true } ) );
			},
			function( error ) {
				utils.assert( "ok", false,
					"Server: device._server.registerResource() failed with: " + error );
			} );
	},
	function( error ) {
		utils.assert( "ok", false, "Server: device.configure() failed with: " + error );
	} );

// Cleanup on SIGINT
process.on( "SIGINT", function() {
	device._server.unregisterResource( theResource.id ).then(
		function() {
			utils.assert( "ok", true, "Server: device._server.unregisterResource() successful" );
			process.exit( 0 );
		},
		function( error ) {
			utils.assert( "ok", false,
				"Server: device._server.unregisterResource() failed with: " + error );
			process.exit( 0 );
		} );
} );

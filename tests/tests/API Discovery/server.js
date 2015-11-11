var theResource,
	async = require( "async" ),
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )(),
	uuid = process.argv[ 2 ];

console.log( JSON.stringify( { assertionCount: 4 } ) );

async.series( [
	function configureDevice( callback ) {
		device.configure( {
			role: "server",
			connectionMode: "acked",
			info: {
				uuid: uuid,
				name: "api-discovery-" + uuid,
				manufacturerName: "Intel"
			}
		} ).then( function() {
			utils.assert( "ok", true, "Server: device.configure() successful" );
			callback();
		}, callback );
	},

	function registerResource( callback ) {
		device.server.registerResource( {
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
				utils.assert( "ok", true, "Server: device.server.registerResource() successful" );
				callback();
			}, callback );
	},

	function processDeleteRequest( callback ) {
		var deleteHandler = function( request ) {
			var cleanup = function( error ) {
				utils.assert( "strictEqual", !!error, false,
					"Server: Unregistered resource successfully" );
				request[ error ? "sendError" : "sendResponse" ]( error ? error : null )
					.then( callback, callback );
			};

			utils.assert( "strictEqual", request.type, "delete",
				"Server: First request was 'delete'" );
			device.server.unregisterResource( theResource.id ).then( cleanup, cleanup );
			device.server.removeEventListener( "request", deleteHandler );
		};

		device.server.addEventListener( "request", deleteHandler );

		// Ready for client requests
		console.log( JSON.stringify( { ready: true } ) );
	}

], function( error ) {
	if ( error ) {
		utils.die( "Server: " + error.message + ", result: " + error.result );
	}
} );

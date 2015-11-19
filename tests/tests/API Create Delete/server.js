var resource,
	async = require( "async" ),
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )(),
	uuid = process.argv[ 2 ];

console.log( JSON.stringify( { assertionCount: 5 } ) );

async.series( [
	function initDevice( callback ) {
		device.configure( {
			role: "server",
			connectionMode: "acked"
		} ).then( callback, callback );
	},

	function registerResource( callback ) {
		device.server.registerResource( {
			url: "/a/" + uuid,
			resourceTypes: [ "core.light" ],
			interfaces: [ "oic.if.baseline" ],
			discoverable: true,
			properties: {
				someProperty: "someValue"
			}
		} ).then( function( theResource ) {
			resource = theResource;
			callback();
		}, callback );
	},

	function processRequests( callback ) {
		var done = function( error ) {
				device.server.removeEventListener( "request", requestHandler );
				callback( error );
			},
			requestIndex = 0,
			requestHandler = function( request ) {
				switch ( requestIndex ) {

				// Both the initial "create" request and the subsequent duplicate "create"
				// request are handled by the same code. The expected behaviour is asserted by
				// the client.
				case 0:
				case 1:
					utils.assert( "strictEqual", request.type, "create",
						"Server: " + ( requestIndex === 0 ? "First" : "Second" ) +
						" request is 'create'" );
					utils.assert( "deepEqual", request.res, {
						discoverable: true,
						url: "/a/new-resource",
						resourceTypes: [ "core.light" ],
						interfaces: [ "oic.if.baseline" ],
						properties: {
							someKey: "someValue"
						}
					}, "Server: Resource signature is as expected" );
					device.server.registerResource( request.res ).then(
						function( theResource ) {
							resource = theResource;
							request.sendResponse( null ).catch( done );
						},
						function( error ) {
							request.sendError( error ).catch( done );
						} );
					break;

				case 2:
					utils.assert( "strictEqual", request.type, "delete",
						"Server: Third request is 'delete'" );
					device.server.unregisterResource( resource.id ).then(
						function() {
							request.sendResponse( null ).then( done, done );
						},
						function( error ) {
							request.sendError( error ).then( done, done );
						} );
					break;

				default:
					done( new Error( "Unexpected request" ) );
					break;
				}
				requestIndex++;
			};
		device.server.addEventListener( "request", requestHandler );
		console.log( JSON.stringify( { ready: true } ) );
	}

], function( error ) {
	if ( error ) {
		utils.die( "Client: Error " + error.message + " and result " + error.result );
	}
} );

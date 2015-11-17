var remoteResource,
	_ = require( "lodash" ),
	async = require( "async" ),
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )(),
	uuid = process.argv[ 2 ];

console.log( JSON.stringify( { assertionCount: 5 } ) );

function findResourceByUrl( url, deviceId ) {
	return Promise.all( [
		new Promise( function( fulfill ) {
			var resourceFound = function( event ) {
				if ( event.resource.url === url ) {
					device.client.removeEventListener( "resourcefound", resourceFound );
					fulfill( event.resource );
				}
			};
			device.client.addEventListener( "resourcefound", resourceFound );
		} ),
		device.client.findResources( _.extend( {}, deviceId ? { deviceId: deviceId } : {} ) )
	] ).then( function( results ) {
		return results[ 0 ];
	} );
}

async.series( [
	function initDevice( callback ) {
		device.configure( {
			role: "client"
		} ).then( callback, callback );
	},

	function findTestServer( callback ) {
		findResourceByUrl( "/a/" + uuid ).then(
			function( resource ) {
				remoteResource = resource;
				callback();
			}, callback );
	},

	function createRemoteResource( callback ) {
		device.client.createResource( {
			url: "/a/new-resource",
			deviceId: remoteResource.deviceId,
			discoverable: true,
			properties: {
				someKey: "someValue"
			},
			resourceTypes: [ "core.light" ],
			interfaces: [ "oic.if.baseline" ]
		} ).then(
			function( resource ) {

				// Use the newly created remote resource instead of the initial resource that we
				// used for locating the test server.
				remoteResource = resource;
				utils.assert( "ok", true, "Client: Creating remote resource has succeeded" );
				callback();
			}, callback );
	},

	function findRemoteResource( callback ) {
		findResourceByUrl( "/a/new-resource", remoteResource.deviceId ).then(
			function( resource ) {
				utils.assert( "ok", true, "Client: Discovering remote resource has succeeded" );
				utils.assert( "strictEqual", resource.id, remoteResource.id,
					"Client: Discovered remote resource has the same id as the created resource" );
				callback();
			}, callback );
	},

	function createDuplicateRemoteResource( callback ) {
		device.client.createResource( {
			url: "/a/new-resource",
			deviceId: remoteResource.deviceId,
			discoverable: true,
			properties: {
				someKey: "someValue"
			},
			resourceTypes: [ "core.light" ],
			interfaces: [ "oic.if.baseline" ]
		} ).then(
			function() {
				callback( new Error( "Server created duplicate resource" ) );
			}, function() {
				utils.assert( "ok", true,
					"Client: Creating a duplicate remote resource is not possible" );
				callback();
			} );
	},

	function deleteRemoteResource( callback ) {
		device.client.deleteResource( remoteResource.id ).then(
			function() {
				utils.assert( "ok", true, "Client: Deleting the remote resource has succeeded" );
				console.log( JSON.stringify( { killPeer: true } ) );
				process.exit( 0 );
			}, callback );
	}
], function( error ) {
	if ( error ) {
		utils.die( "Client: Error: " + error.message + " and result " + error.result );
	}
} );

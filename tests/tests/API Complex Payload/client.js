var resource,
	_ = require( "lodash" ),
	async = require( "async" ),
	uuid = process.argv[ 2 ],
	device = require( "../../../index" )(),
	testUtils = require( "../../assert-to-console" );

console.log( JSON.stringify( { assertionCount: 1 } ) );

async.series( [

	function initStack( callback ) {
		device.configure( {
			role: "client"
		} ).then( callback, callback );
	},

	function discoverResource( callback ) {
		Promise.all( [
			new Promise( function( fulfill ) {
				var resourceFound = function( event ) {
					if ( event.resource.id.path === "/a/" + uuid ) {
						resource = event.resource;
						fulfill();
						device.removeEventListener( "resourcefound", resourceFound );
					}
				};
				device.addEventListener( "resourcefound", resourceFound );
			} ),
			device.findResources()
		] ).then(
			function() {
				callback();
			}, callback );
	},

	function performRetrieve( callback ) {
		device.retrieveResource( resource.id ).then(
			function( resource ) {
				testUtils.assert( "deepEqual", resource.properties, {
					primitiveValue: 42,
					objectValue: {
						childValue: 91
					},
					arrayValue: [ 19, 23, 7 ]
				}, "Client: Retrieved properties are correct" );
				callback();
			},
			callback );
	},

	function performUpdate( callback ) {
		device.updateResource( _.extend( resource, {
			properties: {
				putValue: "A string",
				anotherPutValue: 23.7,
				childValues: {
					putChildValue: false,
					putChildArray: [ [ 2, 3, 5 ], [ 9, 11, 17 ] ]
				}
			}
		} ) ).then( function() {
			console.log( JSON.stringify( { killPeer: true } ) );
			process.exit( 0 );
		}, callback );
	}
], function( error ) {
	if ( error ) {
		testUtils.die( "Client: " + error.message + ", result: " + error.result );
	}
} );

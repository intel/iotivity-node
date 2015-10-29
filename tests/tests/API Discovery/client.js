var _ = require( "lodash" ),
	async = require( "async" ),
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )(),
	uuid = process.argv[ 2 ];

console.log( JSON.stringify( { assertionCount: 6 } ) );

function discoverTheResource( callback ) {
	var eventHandler = function( event ) {
			var index,
				count = 0,
				url = "/a/" + uuid;

			if ( event.resource.url === url ) {
				utils.assert( "ok", true, "Client: Resource found" );

				for ( index in device.client._resources ) {
					if ( device.client._resources[ index ].url === url ) {
						count++;
					}
				}

				utils.assert( "strictEqual", count, 1,
					"Client: Resource present exactly once among resources" );

				maybeDone();
			}
		},
		maybeDoneCount = 0,
		maybeDone = function( error ) {
			var doCallback = false;
			if ( error ) {
				doCallback = true;
			} else {
				maybeDoneCount++;
				if ( maybeDoneCount >= 2 ) {
					doCallback = true;
				}
			}
			if ( doCallback ) {
				device.client.removeEventListener( "resourcefound", eventHandler );
				callback( error );
			}
		};

		device.client.addEventListener( "resourcefound", eventHandler );

		device.client.findResources().then(
			function() {
				utils.assert( "ok", true, "Client: device.client.findResources() successful" );
				maybeDone();
			},
			function( error ) {
				maybeDone( _.extend( error, { step: "device.client.findResources()" } ) );
			} );
}

async.series( [

	// Configure the device
	function( callback ) {
		device.configure( {
			role: "client",
			connectionMode: "acked"
		} ).then(
			function() {
				callback( null );
			},
			function( error ) {
				callback( _.extend( error, { step: "device.configure()" } ) );
			} );
	},

	// Discover the resource twice
	discoverTheResource,
	discoverTheResource

], function( error ) {
	if ( error ) {
		utils.die( "Client: " + error.step + " failed with " + error + " and result " +
			error.result );
	} else {
		console.log( JSON.stringify( { killPeer: true } ) );
		process.exit( 0 );
	}
} );

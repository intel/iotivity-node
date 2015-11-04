var _ = require( "lodash" ),
	async = require( "async" ),
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )(),
	iotivity = require( "bindings" )( "iotivity" ),
	uuid = process.argv[ 2 ];

console.log( JSON.stringify( { assertionCount: 9 } ) );

function discoverTheResource() {
	var eventHandler,
		removeListener = function() {
			if ( eventHandler ) {
				device.client.removeEventListener( "resourcefound", eventHandler );
			}
		};

	return Promise.all( [
		new Promise( function( fulfill ) {
			eventHandler = function( event ) {
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
					fulfill();
				}
			};

			device.client.addEventListener( "resourcefound", eventHandler );
		} ),
		device.client.findResources().then(
			function() {
				utils.assert( "ok", true, "Client: device.client.findResources() successful" );
			} )
	] ).then( removeListener, removeListener );
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

	// Discover the resource once
	function( callback ) {
		utils.assert( "strictEqual", device.client.findResources._handle, undefined,
			"Client: open-ended resource discovery handle is initially undefined" );
		discoverTheResource().then(
			function() {
				callback( null );
			},
			function( error ) {
				callback( _.extend( error, { step: "first discovery" } ) );
			} );
	},

	// Discover the resource again
	function( callback ) {

		// We move on when discovery has completed and when OCCancel() was called
		Promise.all( [
			new Promise( function( fulfill ) {
				var OCCancel = iotivity.OCCancel,
					discoveryHandle = device.client.findResources._handle;

				utils.assert( "strictEqual", !!device.client.findResources._handle, true,
					"Client: open-ended resource discovery handle is set after one discovery" );

				// Overwrite iotivity.OCCancel() to make sure it gets called during the next
				// discovery, and that it gets called with the existing open-ended resource
				// discovery handle.
				iotivity.OCCancel = function( handle ) {
					utils.assert( "strictEqual",
						iotivity.__compareDoHandles( handle, discoveryHandle ),
						true, "Client: OCCancel() called with open-ended resource discovery " +
						"handle before next discovery request" );
					fulfill();

					// We expect only one call to OCCancel() so restore the original
					iotivity.OCCancel = OCCancel;

					// Chain back to the original
					return OCCancel.apply( this, arguments );
				};
			} ),
			discoverTheResource()
		] ).then(
			function() {
				callback( null );
			},
			function( error ) {
				callback( _.extend( error, { step: "second discovery" } ) );
			} );
	}

], function( error ) {
	if ( error ) {
		utils.die( "Client: " + error.step + " failed with " + error + " and result " +
			error.result );
	} else {
		console.log( JSON.stringify( { killPeer: true } ) );
		process.exit( 0 );
	}
} );

var intervalId,
	handle = {},

	// This is the same value as server.get.js
	sampleUri = "/a/iotivity-node-observe-sample",
	iotivity = require( "iotivity" ),
	resourceMissing = true;

// Start iotivity and set up the processing loop
iotivity.OCInit( null, 0, iotivity.OCMode.OC_CLIENT );

intervalId = setInterval( function() {
	iotivity.OCProcess();
}, 1000 );

// Discover resources and list them
iotivity.OCDoResource(

	// The bindings fill in this object
	handle,

	iotivity.OCMethod.OC_REST_DISCOVER,

	// Standard path for discovering resources
	iotivity.OC_RSRVD_WELL_KNOWN_URI,

	// There is no destination
	null,

	// There is no payload
	null,
	iotivity.OCConnectivityType.CT_DEFAULT,
	iotivity.OCQualityOfService.OC_HIGH_QOS,
	function( handle, response ) {
		console.log( "Received response to DISCOVER request:" );
		console.log( JSON.stringify( response, null, 4 ) );
		var index,
			destination = response.addr,
			getHandle = {},
			resources = response && response.payload && response.payload.resources,
			resourceCount = resources.length ? resources.length : 0;

		if ( resourceMissing ) {

			// If the sample URI is among the resources, issue the OBSERVE request to it
			for ( index = 0 ; index < resourceCount ; index++ ) {
				if ( resources[ index ].uri === sampleUri ) {
					resourceMissing = false;

					console.log( "Observing " + sampleUri );

					iotivity.OCDoResource(
						getHandle,
						iotivity.OCMethod.OC_REST_OBSERVE,
						sampleUri,
						destination,
						null,
						iotivity.OCConnectivityType.CT_DEFAULT,
						iotivity.OCQualityOfService.OC_HIGH_QOS,
						function( handle, response ) {
							console.log( "Received response to OBSERVE request:" );
							console.log( JSON.stringify( response, null, 4 ) );
							return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
						},
						null );
				}
			}
		}

		return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
	},

	// There are no header options
	null );

// Exit gracefully when interrupted
process.on( "SIGINT", function() {
	console.log( "SIGINT: Quitting..." );

	// Tear down the processing loop and stop iotivity
	clearInterval( intervalId );
	iotivity.OCStop();

	// Exit
	process.exit( 0 );
} );

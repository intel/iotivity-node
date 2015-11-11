var intervalId,
	handleReceptacle = {},

	// This is the same value as server.presence.js
	sampleUri = "/a/iotivity-node-presence-sample",
	iotivity = require( "iotivity-node/lowlevel" );

// Start iotivity and set up the processing loop
iotivity.OCInit( null, 0, iotivity.OCMode.OC_CLIENT );

intervalId = setInterval( function() {
	iotivity.OCProcess();
}, 1000 );

// Discover resources and list them
iotivity.OCDoResource(

	// The bindings fill in this object
	handleReceptacle,

	iotivity.OCMethod.OC_REST_DISCOVER,

	// Standard path for discovering resources
	iotivity.OC_MULTICAST_DISCOVERY_URI,

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
			presenceHandleReceptacle = {},
			resources = response && response.payload && response.payload.resources,
			resourceCount = resources ? resources.length : 0,
			presenceResponseHandler = function( handle, response ) {
				console.log( "Received response to PRESENCE request:" );
				console.log( JSON.stringify( response, null, 4 ) );
				return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
			};

		// If the sample URI is among the resources, issue the GET request to it
		for ( index = 0; index < resourceCount; index++ ) {
			if ( resources[ index ].uri === sampleUri ) {
				iotivity.OCDoResource(
					presenceHandleReceptacle,
					iotivity.OCMethod.OC_REST_PRESENCE,
					iotivity.OC_RSRVD_PRESENCE_URI,
					destination,
					null,
					iotivity.OCConnectivityType.CT_DEFAULT,
					iotivity.OCQualityOfService.OC_HIGH_QOS,
					presenceResponseHandler,
					null );
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

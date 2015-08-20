var intervalId,
	handle = {},

	// This is the same value as server.get.js
	sampleUri = "/a/iotivity-node-get-sample",
	iotivity = require( "iotivity" );

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

		// If the sample URI is among the resources, issue the GET request to it
		for ( index = 0 ; index < resourceCount ; index++ ) {
			if ( resources[ index ].uri === sampleUri ) {
				iotivity.OCDoResource(
					getHandle,
					iotivity.OCMethod.OC_REST_GET,
					sampleUri,
					destination,
					{
						type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
						values: {
							question: "How many angels can dance on the head of a pin?"
						}
					},
					iotivity.OCConnectivityType.CT_DEFAULT,
					iotivity.OCQualityOfService.OC_HIGH_QOS,
					function( handle, response ) {
						console.log( "Received response to GET request:" );
						console.log( JSON.stringify( response, null, 4 ) );
						return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
					},
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

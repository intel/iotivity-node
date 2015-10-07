var intervalId,
	handleReceptacle = {},
	iotivity = require( "iotivity-node/lowlevel" );

// Start iotivity and set up the processing loop
iotivity.OCInit( null, 0, iotivity.OCMode.OC_CLIENT );

intervalId = setInterval( function() {
	iotivity.OCProcess();
}, 1000 );

function listenForPresence() {

	// Request resource presence notifications
	console.log( "OCDoResource(presence): " + iotivity.OCDoResource(

			// The bindings fill in this object
			handleReceptacle,

			iotivity.OCMethod.OC_REST_PRESENCE,
			iotivity.OC_RSRVD_PRESENCE_URI,

			// Initialize the destination with an empty structure
			{
				adapter: iotivity.OCTransportAdapter.OC_DEFAULT_ADAPTER,
				flags: iotivity.OCTransportFlags.OC_DEFAULT_FLAGS,
				interface: 0,
				port: 0,
				addr: [ 0 ],
				identity: [ 0 ]
			},
			null,
			iotivity.OCConnectivityType.CT_DEFAULT,
			iotivity.OCQualityOfService.OC_HIGH_QOS,
			function( handle, response ) {
				var returnValue = iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;

				console.log( "Received response to PRESENCE request:" );
				console.log( JSON.stringify( response, null, 4 ) );

				if ( response.result === iotivity.OCStackResult.OC_STACK_COMM_ERROR ) {

					console.log( "Request failed. Trying again." );
					setTimeout( listenForPresence, 0 );
					returnValue = iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
				}

				return returnValue;
			},

			// There are no header options
			null ) );
}

listenForPresence();

// Exit gracefully when interrupted
process.on( "SIGINT", function() {
	console.log( "SIGINT: Quitting..." );

	// Tear down the processing loop and stop iotivity
	clearInterval( intervalId );
	iotivity.OCStop();

	// Exit
	process.exit( 0 );
} );

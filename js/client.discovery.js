var intervalId,
	handleReceptacle = {},
	iotivity = require( "iotivity/lowlevel" ),
	options = ( function() {
		var index,
			returnValue = {

				// By default we discover resources
				discoveryUri: iotivity.OC_RSRVD_WELL_KNOWN_URI
			};

		for ( index in process.argv ) {
			if ( process.argv[ index ] === "-d" || process.argv[ index ] === "--device" ) {
				returnValue.discoveryUri = iotivity.OC_RSRVD_DEVICE_URI;
			} else if ( process.argv[ index ] === "-p" || process.argv[ index ] === "-platform" ) {
				returnValue.discoveryUri = iotivity.OC_RSRVD_PLATFORM_URI;
			}
		}

		return returnValue;
	} )();

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

	// Standard path for discovering devices/resources
	options.discoveryUri,

	// There is no destination
	null,

	// There is no payload
	null,
	iotivity.OCConnectivityType.CT_DEFAULT,
	iotivity.OCQualityOfService.OC_HIGH_QOS,
	function( handle, response ) {
		console.log( JSON.stringify( response, null, 4 ) );
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

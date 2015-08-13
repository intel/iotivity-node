var intervalId,
	options = ( function() {
		var index;
			returnValue = {
				discoverDevices: false
			};

		for ( index in process.argv ) {
			if ( process.argv[ index ] === "-d" || process.argv[ index ] === "--devices" ) {
				returnValue.discoverDevices = true;
			}
		}

		return returnValue;
	} )();
	handle = {},
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

	// Standard path for discovering devices/resources
	options.discoverDevices ?
		iotivity.OC_RSRVD_DEVICE_URI :
		iotivity.OC_RSRVD_WELL_KNOWN_URI,

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

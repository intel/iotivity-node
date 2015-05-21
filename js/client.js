var intervalId,
	iotivity = require( "iotivity" ),
	handle = {};

iotivity.OCInit( null, 0, iotivity.OCMode.OC_CLIENT );

intervalId = setInterval( function() {
	iotivity.OCProcess();
}, 100 );

iotivity.OCDoResource(
	handle,
	iotivity.OCMethod.OC_REST_GET,
	"light",
	null,
	null,
	iotivity.OCConnectivityType.OC_ALL,
	iotivity.OCQualityOfService.OC_HIGH_QOS,
	function( handle, response ) {
		console.log( "OCDoResource() handler: Entering" );
		return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
	},
	null,
	0 );

process.on( "SIGINT", function() {
	console.log( "SIGINT: Quitting..." );
	clearInterval( intervalId );
	iotivity.OCStop();
	process.exit( 0 );
} );

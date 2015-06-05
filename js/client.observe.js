var intervalId,
	iotivity = require( "iotivity" ),
	handle = {};

// Construct the absolute URL for the resource from the OCDoResource() response
function getAbsoluteUrl( response ) {
	var ipv4Bytes = [],
		portHolder = {};

	if ( iotivity.OCStackResult.OC_STACK_OK !==
			iotivity.OCDevAddrToIPv4Addr( response.addr, ipv4Bytes ) ) {
		return;
	}

	if ( iotivity.OCStackResult.OC_STACK_OK !==
			iotivity.OCDevAddrToPort( response.addr, portHolder ) ) {
		return;
	}

	return "coap://" +
		ipv4Bytes[ 0 ] + "." + ipv4Bytes[ 1 ] + "." + ipv4Bytes[ 2 ] + "." + ipv4Bytes[ 3 ] + ":" +
		portHolder.port + JSON.parse( response.resJSONPayload ).oic[ 0 ].href;
}

iotivity.OCInit( null, 0, iotivity.OCMode.OC_CLIENT );

intervalId = setInterval( function() {
	iotivity.OCProcess();
}, 100 );

// Initial call by which we discover the resource we wish to observe
iotivity.OCDoResource(
	handle,
	iotivity.OCMethod.OC_REST_GET,
	"/oc/core",
	null,
	null,
	iotivity.OCConnectivityType.OC_ALL,
	iotivity.OCQualityOfService.OC_HIGH_QOS,
	function( handle, response ) {
		var observeHandle = {},
			absoluteUrl = getAbsoluteUrl( response );
		console.log( "OCDoResource() handler for discovery: Entering" );
		console.log( "absolute url discovered: " + absoluteUrl );
		if ( absoluteUrl ) {

			// With this second call we request notifications for this resource
			iotivity.OCDoResource(
				observeHandle,
				iotivity.OCMethod.OC_REST_OBSERVE,
				absoluteUrl,
				null,
				null,
				response.connType,
				iotivity.OCQualityOfService.OC_HIGH_QOS,
				function( handle, response ) {
					console.log( "OCDoResource() handler for notification: Entering" );
					console.log( response );

					return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
				},
				null,
				0 );
		}

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

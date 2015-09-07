var result,
	uuid = process.argv[ 2 ],
	processCallCount = 0,
	processLoop = null,
	discoverHandleReceptacle = {},
	observeResponseCount = 0,
	iotivity = require( "../../../lowlevel" ),
	testUtils = require( "../../utils" )( iotivity );

console.log( JSON.stringify( { assertionCount: 12 } ) );

// Initialize
result = iotivity.OCInit( null, 0, iotivity.OCMode.OC_CLIENT );
testUtils.stackOKOrDie( "Client", "OCInit", result );

// Set up OCProcess loop
processLoop = setInterval( function() {
	var processResult = iotivity.OCProcess();

	if ( processResult === iotivity.OCStackResult.OC_STACK_OK ) {
		processCallCount++;
	} else {
		testUtils.stackOKOrDie(
			"Client",
			"OCProcess(after " + processCallCount + " successful calls)",
			processResult );
	}
}, 100 );

// Discover resources and list them
result = iotivity.OCDoResource(

	// The bindings fill in this object
	discoverHandleReceptacle,

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

		// We retain the discovery callback until we've found the resource identified by the uuid.
		var returnValue = iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;

		if ( testUtils.findResource( response, uuid ) ) {

			// We've successfully found the resource so let's issue a GET request on it.
			testUtils.assert( "ok", true, "Client: Resource found" );
			doObserveRequest( response.addr );
			returnValue = iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
		}

		return returnValue;
	},

	// There are no header options
	null );
testUtils.stackOKOrDie( "Client", "OCDoResource(discovery)", result );

function doObserveRequest( destination ) {
	var observeResult,
		observeHandleReceptacle = {};

	observeResult = iotivity.OCDoResource(
		observeHandleReceptacle,
		iotivity.OCMethod.OC_REST_OBSERVE,
		"/a/" + uuid,
		destination,
		null,
		iotivity.OCConnectivityType.CT_DEFAULT,
		iotivity.OCQualityOfService.OC_HIGH_QOS,
		function( handle, response ) {
			var cancelResult;

			testUtils.assert( "deepEqual",
				response.payload,
				{ type: 4, values: { observedValue: uuid + "-" + observeResponseCount } },
				"Client: Observed value is as expected" );

			if ( ++observeResponseCount >= 5 ) {
				cancelResult = iotivity.OCCancel(
					handle,
					iotivity.OCQualityOfService.OC_HIGH_QOS,
					[] );
				testUtils.stackOKOrDie( "Client", "OCCancel", cancelResult );

				return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
			}
			return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
		},
		null );
	testUtils.stackOKOrDie( "Client", "OCDoResource(observation)", observeResult );
}

function cleanup() {
	var cleanupResult;

	if ( processLoop ) {
		clearInterval( processLoop );
		processLoop = null;
	}

	testUtils.assert( "ok", true, "Client: OCProcess succeeded " + processCallCount + " times" );

	cleanupResult = iotivity.OCStop();
	testUtils.stackOKOrDie( "Client", "OCStop", cleanupResult );
}

// Exit gracefully when interrupted
process.on( "SIGINT", cleanup );

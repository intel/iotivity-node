var result,
	uuid = process.argv[ 2 ],
	processCallCount = 0,
	processLoop = null,
	discoverHandleReceptacle = {},
	iotivity = require( "../../../index" ),
	testUtils = require( "../../utils" )( iotivity );

console.log( JSON.stringify( { assertionCount: 5 } ) );

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

// Discover
result = iotivity.OCDoResource(
	discoverHandleReceptacle,
	iotivity.OCMethod.OC_REST_DISCOVER,
	iotivity.OC_RSRVD_WELL_KNOWN_URI,
	null,
	null,
	iotivity.OCConnectivityType.CT_DEFAULT,
	iotivity.OCQualityOfService.OC_HIGH_QOS,
	function( handle, response ) {
		// We retain the discovery callback until we've found the resource identified by the uuid.
		var returnValue = iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;

		if ( testUtils.findResource( response, uuid ) ) {

			// We've successfully completed the test so let's kill the server and test the cleanup.
			console.log( JSON.stringify( {
				assertion: "ok",
				arguments: [ true, "Client: Resource found" ]
			} ) );
			returnValue = iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
			cleanup();

		}

		return returnValue;
	},
	null );
testUtils.stackOKOrDie( "Client", "OCDoResource(discovery)", result );

function cleanup() {
	var cleanupResult;

	if ( processLoop ) {
		clearInterval( processLoop );
		processLoop = null;
	}

	console.log( JSON.stringify( {
		assertion: "ok",
		arguments: [ true, "Client: OCProcess succeeded " + processCallCount + " times" ]
	} ) );

	cleanupResult = iotivity.OCStop();
	if ( testUtils.stackOKOrDie( "Client", "OCStop", cleanupResult ) ) {
		console.log( JSON.stringify( { killPeer: true } ) );
		process.exit( 0 );
	}
}

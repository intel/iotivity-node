var result,
	uuid = process.argv[ 2 ],
	processCallCount = 0,
	processLoop = null,
	resourceHandleReceptacle = {},
	iotivity = require( "../../../lowlevel" ),
	testUtils = require( "../../utils" )( iotivity );

console.log( JSON.stringify( { assertionCount: 10 } ) );

// Initialize
result = iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );
testUtils.stackOKOrDie( "Server", "OCInit", result );

result = iotivity.OCSetDefaultDeviceEntityHandler( function( flag, request, uri ) {
	var responseResult;

	testUtils.assert( "deepEqual",
		testUtils.lookupBitfieldValueNames( "OCEntityHandlerFlag", flag ),
		{ OC_REQUEST_FLAG: true },
		"Server: Incoming request has the correct flags" );

	testUtils.assert( "deepEqual",
		uri,
		"/a/" + uuid + "-xyzzy",
		"Server: Incoming request has the correct URI" );

	if ( request &&
			request.payload &&
			request.payload.type === iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION &&
			request.payload.values &&
			request.payload.values.question ) {
		testUtils.assert( "strictEqual", request.payload.values.question,
			"How many angels can dance on the head of a pin?",
			"Server: Correct request received" );

		responseResult = iotivity.OCDoResponse( {
			requestHandle: request.requestHandle,
			resourceHandle: request.resource,
			ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
			payload: {
				type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
				values: {
					answer: "You're asking the wrong resource."
				}
			},
			resourceUri: "/a/" + uuid,
			sendVendorSpecificHeaderOptions: []
		} );
		testUtils.stackOKOrDie( "Server", "OCDoResponse", responseResult );
	} else {
		testUtils.die( "Server: Unexpected request:\n***\n" +
			JSON.stringify( request, null, 4 ) + "\n***" );
	}

	return iotivity.OCEntityHandlerResult.OC_EH_OK;
} );
testUtils.stackOKOrDie( "Server", "OCSetDefaultDeviceEntityHandler", result );

// Set up process loop
processLoop = setInterval( function() {
	var processResult = iotivity.OCProcess();

	if ( processResult === iotivity.OCStackResult.OC_STACK_OK ) {
		processCallCount++;
	} else {
		testUtils.stackOKOrDie(
			"Server",
			"OCProcess(after " + processCallCount + " successful calls)",
			processResult );
	}
}, 100 );

// Create resource
result = iotivity.OCCreateResource(
	resourceHandleReceptacle,
	"core.fan",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	"/a/" + uuid,
	function( flag, request ) {
		testUtils.die( "Server: resource entity handler should not get called" );
		return iotivity.OCEntityHandlerResult.OC_EH_ERROR;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE );
testUtils.stackOKOrDie( "Server", "OCCreateResource", result );

// Report that the server has successfully created its resource(s).
console.log( JSON.stringify( { ready: true } ) );

function cleanup() {
	var cleanupResult;

	if ( processLoop ) {
		clearInterval( processLoop );
		processLoop = null;
	}

	testUtils.assert( "ok", true, "Server: OCProcess succeeded " + processCallCount + " times" );

	cleanupResult = iotivity.OCDeleteResource( resourceHandleReceptacle.handle );
	testUtils.stackOKOrDie( "Server", "OCDeleteResource", result );

	cleanupResult = iotivity.OCStop();
	if ( testUtils.stackOKOrDie( "Server", "OCStop", result ) ) {
		process.exit( 0 );
	}
}

// Exit gracefully when interrupted
process.on( "SIGINT", cleanup );

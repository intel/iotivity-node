var result, notifyObserversTimeoutId, observationId,
	clientsComplete = 0,
	uuid = process.argv[ 2 ],
	notificationCount = 0,
	processCallCount = 0,
	processLoop = null,
	resourceHandleReceptacle = {},
	iotivity = require( "../../../index" ),
	testUtils = require( "../../utils" )( iotivity ),
	listOfObservers = [];

console.log( JSON.stringify( { assertionCount: 12 } ) );

// Tally up how many of our clients have informed us that they have made the requisite number of
// observations and quit if all have reported in.
function maybeQuit() {
	clientsComplete++;

	if ( clientsComplete === 2 ) {
		cleanup();
	}
}

// Initialize
result = iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );
testUtils.stackOKOrDie( "Server", "OCInit", result );

// Set up the process loop
processLoop = setInterval( function idlyLoopOCProcess() {
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

// Set up the notification loop
function notifyObservers() {
	var notificationResult;

	if ( resourceHandleReceptacle.handle && listOfObservers.length > 0 ) {
		notificationResult = iotivity.OCNotifyListOfObservers(
			resourceHandleReceptacle.handle,
			notificationCount < 5 ? listOfObservers : [ listOfObservers[ 0 ] ],
			{
				type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
				values: { observedValue: uuid + "-" + notificationCount }
			},
			iotivity.OCQualityOfService.OC_HIGH_QOS );

		if ( notificationResult === iotivity.OCStackResult.OC_STACK_OK ) {
			notificationCount++;
		} else {
			testUtils.stackOKOrDie(
				"Server",
				"OCNotifyListOfObservers(after " + notificationCount + " successful calls)",
				notificationResult );
		}
	}

	if ( notificationCount < 10 ) {

		// Run this function again between 200 and 400 ms from now
		notifyObserversTimeoutId = setTimeout( notifyObservers, Math.random() * 200 + 200 );
	}
}

// Create a new resource
result = iotivity.OCCreateResource(

	// The bindings fill in this object
	resourceHandleReceptacle,

	"core.fan",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	"/a/" + uuid,
	function( flag, request ) {
		var responseResult;

		if ( flag & iotivity.OCEntityHandlerFlag.OC_OBSERVE_FLAG ) {
			testUtils.assert( "ok", request.obsInfo.obsId !== 0,
				"Server: Observe request observation ID is not zero " +
				"(" + request.obsInfo.obsId + ")" );
			testUtils.assert( "strictEqual",
				testUtils.lookupEnumValueName( "OCObserveAction", request.obsInfo.action ),
				"OC_OBSERVE_REGISTER",
				"Server: Initial observe request has register action" );

			listOfObservers.push( request.obsInfo.obsId );

			if ( listOfObservers.length > 2 ) {
				testUtils.die( "Server: Too many obsIds encountered " +
					"(" + listOfObservers.length + ")" );
			}

			responseResult = iotivity.OCDoResponse( {
				requestHandle: request.requestHandle,
				resourceHandle: request.resource,
				ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
				payload: {
					type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
					values: {
						message: ( listOfObservers.length > 1 ?
							"You are not special." :
							"You are special." )
					}
				},
				resourceUri: "/a/" + uuid,
				sendVendorSpecificHeaderOptions: []
			} );
			testUtils.stackOKOrDie( "Server", "OCDoResponse", responseResult );

			if ( listOfObservers.length === 2 ) {
				setTimeout( notifyObservers, 200 );
			}
		} else if ( request.method === iotivity.OCMethod.OC_REST_PUT ) {
			maybeQuit();

		// After all our clients are complete, we don't care about any more requests.
		} else if ( clientsComplete < 2 ) {
			testUtils.die( "Server: Request was not an observe or a put request" );
		}

		return iotivity.OCEntityHandlerResult.OC_EH_OK;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE |
	iotivity.OCResourceProperty.OC_OBSERVABLE );
testUtils.stackOKOrDie( "Server", "OCCreateResource", result );

// Report that the server has successfully created its resource(s).
console.log( JSON.stringify( { ready: true } ) );

function cleanup() {
	var cleanupResult, oneAddress;

	if ( processLoop ) {
		clearInterval( processLoop );
		processLoop = null;
	}

	if ( notifyObserversTimeoutId ) {
		clearTimeout( notifyObserversTimeoutId );
		notifyObserversTimeoutId = null;
	}

	testUtils.assert( "ok", true, "Server: OCProcess succeeded " + processCallCount + " times" );

	testUtils.assert( "ok", true,
		"Server: OCNotifyListOfObservers succeeded " + notificationCount + " times" );

	cleanupResult = iotivity.OCDeleteResource( resourceHandleReceptacle.handle );
	testUtils.stackOKOrDie( "Server", "OCDeleteResource", result );

	cleanupResult = iotivity.OCStop();
	if ( testUtils.stackOKOrDie( "Server", "OCStop", result ) ) {
		console.log( JSON.stringify( { killPeer: true } ) );
		process.exit( 0 );
	}
}

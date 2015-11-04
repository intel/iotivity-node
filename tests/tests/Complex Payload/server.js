var result,
	uuid = process.argv[ 2 ],
	processCallCount = 0,
	processLoop = null,
	resourceHandleReceptacle = {},
	iotivity = require( "../../../lowlevel" ),
	testUtils = require( "../../utils" )( iotivity );

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
		console.log( JSON.stringify( { killPeer: true } ) );
		process.exit( 0 );
	}
}

console.log( JSON.stringify( { assertionCount: 8 } ) );

// Initialize
result = iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );
testUtils.stackOKOrDie( "Server", "OCInit", result );

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

var requestSequenceNumber = 0,
	requestSequence = [
		function( flag, request ) {
			if ( request.method !== iotivity.OCMethod.OC_REST_GET ) {
				testUtils.die( "Server: First method was not GET" );
			}
			result = iotivity.OCDoResponse( {
				requestHandle: request.requestHandle,
				resourceHandle: request.resource,
				resourceUri: "/a/" + uuid,
				ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
				payload: {
					type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
					values: {
						primitiveValue: 42,
						objectValue: {
							type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
							values: {
								childValue: 91
							}
						},
						arrayValue: [ 19, 23, 7 ]
					}
				},
				sendVendorSpecificHeaderOptions: []
			} );

			testUtils.stackOKOrDie( "Server", "OCDoResponse(get)", result );

			return iotivity.OCEntityHandlerResult.OC_EH_OK;
		},
		function( flag, request ) {
			if ( request.method !== iotivity.OCMethod.OC_REST_PUT ) {
				testUtils.die( "Server: Second method was not PUT" );
			}
			testUtils.assert( "deepEqual", request.payload, {
				type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
				values: {
					putValue: "A string",
					anotherPutValue: 23.7,
					childValues: {
						type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
						values: {
							putChildValue: false,
							putChildArray: [ [ 2, 3, 5 ], [ 9, 11, 17 ] ]
						}
					}
				}
			}, "Server: received payload is correct" );
			result = iotivity.OCDoResponse( {
				requestHandle: request.requestHandle,
				resourceHandle: request.resource,
				resourceUri: "/a/" + uuid,
				ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
				payload: null,
				sendVendorSpecificHeaderOptions: []
			} );

			testUtils.stackOKOrDie( "Server", "OCDoResponse(put)", result );

			cleanup();

			return iotivity.OCEntityHandlerResult.OC_EH_OK;
		}
	];

// Create resource
result = iotivity.OCCreateResource(
	resourceHandleReceptacle,
	"core.fan",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	"/a/" + uuid,
	function( flag, request ) {
		return requestSequence[ requestSequenceNumber++ ]( flag, request );
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE );
testUtils.stackOKOrDie( "Server", "OCCreateResource", result );

// Report that the server has successfully created its resource(s).
console.log( JSON.stringify( { ready: true } ) );

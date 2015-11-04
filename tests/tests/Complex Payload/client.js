var result, testServerAddress,
	async = require( "async" ),
	uuid = process.argv[ 2 ],
	processCallCount = 0,
	processLoop = null,
	iotivity = require( "../../../lowlevel" ),
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

async.series( [

	// Discover
	function( callback ) {
		var discoverHandleReceptacle = {};

		result = iotivity.OCDoResource(
			discoverHandleReceptacle,
			iotivity.OCMethod.OC_REST_DISCOVER,
			iotivity.OC_MULTICAST_DISCOVERY_URI,
			null,
			null,
			iotivity.OCConnectivityType.CT_DEFAULT,
			iotivity.OCQualityOfService.OC_HIGH_QOS,
			function( handle, response ) {

				// We retain the discovery callback until we've found the resource with the uuid.
				var returnValue = iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;

				if ( testUtils.findResource( response, uuid ) ) {

					// We've successfully found the resource so let's issue a GET request on it.
					testUtils.assert( "ok", true, "Client: Resource found" );

					testServerAddress = response.addr;
					callback();
					returnValue = iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
				}

				return returnValue;
			},
			null );
		if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
			callback( "OCDoResource(discovery): " + result );
		}
	},

	// GET
	function( callback ) {
		var getHandleReceptacle = {};

		result = iotivity.OCDoResource(
			getHandleReceptacle,
			iotivity.OCMethod.OC_REST_GET,
			"/a/" + uuid,
			testServerAddress,
			null,
			iotivity.OCConnectivityType.CT_DEFAULT,
			iotivity.OCQualityOfService.OC_HIGH_QOS,
			function( handle, response ) {
				testUtils.assert( "deepEqual", response.payload, {
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
				}, "Client: received payload is correct" );
				callback();
				return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
			},
			null, 0 );
		if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
			callback( "OCDoResource(get): " + result );
		}
	},

	// PUT
	function( callback ) {
		var putHandleReceptacle = {};

		result = iotivity.OCDoResource(
			putHandleReceptacle,
			iotivity.OCMethod.OC_REST_PUT,
			"/a/" + uuid,
			testServerAddress, {
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
			},
			iotivity.OCConnectivityType.CT_DEFAULT,
			iotivity.OCQualityOfService.OC_HIGH_QOS,
			function() {
				return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
			},
			null, 0 );
		if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
			callback( "OCDoResource(get): " + result );
		}
	}
], function( error ) {
	testUtils.die( "Client: " + error );
} );

process.on( "SIGINT", function() {
	if ( processLoop ) {
		clearInterval( processLoop );
		processLoop = null;
	}

	testUtils.assert( "ok", true, "Client: OCProcess succeeded " + processCallCount + " times" );

	result = iotivity.OCStop();
	if ( testUtils.stackOKOrDie( "Client", "OCStop", result ) ) {
		process.exit( 0 );
	}
} );

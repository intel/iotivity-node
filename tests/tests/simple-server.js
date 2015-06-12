require( "../setup" );

var iotivity = require( "../../lowlevel" ),
	testUtils = require( "../test-utils" )( iotivity, QUnit.assert );

test( "Simple server", function( assert ) {
	var result, stopProcessing, stopTestClient,
		resourcePath = "/a/simple-server-" + Math.round( Math.random() * 10000 ),
		magicRequest = { "setAnswer": 42 },
		done = assert.async(),
		failsafeTimeoutId = null,
		handle = {},

		// Clean up before we exit
		teardown = function() {

			// Kill the test client process
			if ( stopTestClient ) {
				stopTestClient();
			}

			// Stop the processing loop
			if ( stopProcessing ) {
				stopProcessing();
			}

			// Remove the failsafe timer
			if ( failsafeTimeoutId !== null ) {
				clearTimeout( failsafeTimeoutId );
				failsafeTimeoutId = null;
			}

			// Make sure OCDeleteResource works correctly
			result = iotivity.OCDeleteResource( handle );
			assert.deepEqual(
				testUtils.lookupEnumValueName( "OCStackResult", result ),
				"OC_STACK_OK",
				"OCDeleteResource succeeded" );

			// Make sure stack shutdown works
			testUtils.testShutdown();

			// Inform QUnit that this async test has concluded
			done();
		};

	// Since some of our assertions are inside asynchronously called functions, this asserts that
	// all the assertions are reached, but it does not assert the order in which they are reached.
	assert.expect( 6 );

	// Make sure the stack starts up correctly
	if ( testUtils.testStartup( iotivity.OCMode.OC_SERVER ) ===
			iotivity.OCStackResult.OC_STACK_OK ) {

		// Make sure OCCreateResource works correctly
		result = iotivity.OCCreateResource(
			handle,
			"core.light",
			"oc.mi.def",
			resourcePath,
			function( flag, request ) {
				var receivedRequest = request.reqJSONPayload ?
					JSON.parse( request.reqJSONPayload ) : undefined;

				assert.deepEqual( receivedRequest, magicRequest,
					"Entity handler has received the correct request" );
				teardown();

				return iotivity.OCEntityHandlerResult.OC_EH_OK;
			},
			iotivity.OCResourceProperty.OC_DISCOVERABLE |
			iotivity.OCResourceProperty.OC_OBSERVABLE );
		assert.deepEqual(
			testUtils.lookupEnumValueName( "OCStackResult", result ),
			"OC_STACK_OK",
			"OCCreateResource succeeded" );

		// If OCCreateResource was successful, we can make sure the entity handler gets called
		if ( result === iotivity.OCStackResult.OC_STACK_OK ) {

			// Start the processing loop
			stopProcessing = testUtils.testProcessing( 100, teardown );

			// If the entity handler does not get called within five seconds, give up
			failsafeTimeoutId = setTimeout( function() {
				failsafeTimeoutId = null;
				teardown();
			}, 5000 );

			// Start a test client which will send a request to the server under test
			stopTestClient = testUtils.startTestClient( teardown, {
				path: resourcePath,
				request: magicRequest
			} );
		}
	}
} );

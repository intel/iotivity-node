require( "../setup" );

var iotivity = require( "../../index" ),
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
			result = iotivity.OCDeleteResource( handle.handle );
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
	assert.expect( 10 );

	// Make sure the stack starts up correctly
	if ( testUtils.testStartup( iotivity.OCMode.OC_SERVER ) ===
			iotivity.OCStackResult.OC_STACK_OK ) {

		// Make sure OCCreateResource works correctly
		result = iotivity.OCCreateResource(
			handle,
			"core.light",
			iotivity.OC_RSRVD_INTERFACE_DEFAULT,
			resourcePath,
			function( flag, request ) {
				return iotivity.OCEntityHandlerResult.OC_EH_OK;
			},
			iotivity.OCResourceProperty.OC_DISCOVERABLE );
		assert.strictEqual(
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
				method: "OC_REST_DISCOVER",
				logResponse: function( response ) {
					testUtils.assertPathFromResponse( assert, response, resourcePath );
					teardown();
				},
				path: iotivity.OC_RSRVD_WELL_KNOWN_URI
			} );
		}
	}
} );

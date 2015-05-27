require( "../setup" );

var iotivity = require( "../../index" ),
	testUtils = require( "../test-utils" )( iotivity, QUnit.assert ),

	// Token that will make the trip from the server to the client. The test will assert that it is
	// successfully received by the OCDoResource() handler
	magicToken = { href: { rep: { "0": "magicValue", "1": "int", "2": 42 } } };

test( "Simple client", function( assert ) {
	var result, stopProcessing, stopTestServer, responseFromServer,
		done = assert.async(),
		failsafeTimeoutId = null,
		handle = {},

		// Clean up before we exit
		teardown = function() {

			// Kill the test server process
			if ( stopTestServer ) {
				stopTestServer();
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

			// Make sure the callback was called with the right response
			assert.deepEqual(
				responseFromServer,
				magicToken,
				"OCDoResource handler received the expected response" );

			// Make sure stack shutdown works
			testUtils.testShutdown();

			// Inform QUnit that this async test has concluded
			done();
		};

	// Make sure the stack starts up correctly
	if ( testUtils.testStartup( iotivity.OCMode.OC_CLIENT ) ===
			iotivity.OCStackResult.OC_STACK_OK ) {

		// Start a test server which will respond to the request made by the client under test
		stopTestServer = testUtils.startTestServer(

			// Make a request when the server is ready to answer it
			function() {

				// Start the processing loop
				stopProcessing = testUtils.testProcessing( 100, function() {
					teardown();
				} );

				// If the callback does not get called within five seconds, give up
				failsafeTimeoutId = setTimeout( function() {
					failsafeTimeoutId = null;
					teardown();
				}, 5000 );

				// Issue the request and make sure it has worked
				result = iotivity.OCDoResource(
					handle,
					iotivity.OCMethod.OC_REST_GET,
					"/light/1",
					null,
					null,
					iotivity.OCConnectivityType.OC_ALL,
					iotivity.OCQualityOfService.OC_HIGH_QOS,
					function( handle, response ) {
						responseFromServer = JSON.parse( response.resJSONPayload ).oc[ 0 ];
						teardown();
						return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
					},
					null,
					0 );
				assert.deepEqual(
					testUtils.lookupEnumValueName( "OCStackResult", result ),
					"OC_STACK_OK",
					"OCDoResource has succeeded" );
				assert.ok(
					handle.handle,
					"The resulting handle contains a non-null property named \"handle\"" );
				if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
					teardown();
				}
			},

			// If the test server fails to work correctly we cannot perform the test so we give up
			function() {
				teardown();
			},
			magicToken );
	}
} );

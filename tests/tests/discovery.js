require( "../setup" );

var iotivity = require( "../../index" ),
	testUtils = require( "../test-utils" )( iotivity, QUnit.assert ),
	resourcePath = "/simple-client-" + Math.round( Math.random() * 1048576 );

test( "Resource discovery", function( assert ) {
	expect( 10 );

	var result, stopProcessing, stopTestServer,
		done = assert.async(),
		failsafeTimeoutId = null,
		handle = {},
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
				stopProcessing = testUtils.testProcessing( 100, teardown );

				// If the callback does not get called within five seconds, give up
				failsafeTimeoutId = setTimeout( function() {
					failsafeTimeoutId = null;
					teardown();
				}, 5000 );

				// Issue the discovery request and make sure it has worked
				result = iotivity.OCDoResource(
					handle,
					iotivity.OCMethod.OC_REST_DISCOVER,
					iotivity.OC_RSRVD_WELL_KNOWN_URI,
					null,
					null,
					iotivity.OCConnectivityType.CT_DEFAULT,
					iotivity.OCQualityOfService.OC_HIGH_QOS,
					function( handle, response ) {
						testUtils.assertPathFromResponse( assert, response, resourcePath );
						teardown();
					},
					null );
				assert.strictEqual(
					testUtils.lookupEnumValueName( "OCStackResult", result ),
					"OC_STACK_OK",
					"OCDoResource for discovery has succeeded" );
				assert.ok(
					handle.handle,
					"The resulting handle contains a non-null property named 'handle'" );
				if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
					teardown();
				}
			},

			// If the test server fails to work correctly we cannot perform the test so we give up
			teardown,
			{
				path: resourcePath
			} );
	}
} );

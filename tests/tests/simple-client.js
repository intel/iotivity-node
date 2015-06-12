require( "../setup" );

var iotivity = require( "../../lowlevel" ),
	testUtils = require( "../test-utils" )( iotivity, QUnit.assert ),

	// Token that will make the trip from the server to the client. The test will assert that it is
	// successfully received by the OCDoResource() handler
	magicToken = { href: { rep: { "0": "magicValue", "1": "int", "2": 42 } } },
	resourcePath = "/simple-client-" + Math.round( Math.random() * 1048576 );

test( "Simple client", function( assert ) {
	var result, stopProcessing, stopTestServer,
		done = assert.async(),
		failsafeTimeoutId = null,
		handle = {},

		// Construct the absolute URL from the device address and the resource path, while testing
		// OCDevAddrToIPv4Addr and OCDevAddrToPort in the process
		getAbsoluteUrl = function( address ) {
			var result,
				ipv4Bytes = [],
				portHolder = {};

			result = iotivity.OCDevAddrToIPv4Addr( address, ipv4Bytes );
			assert.deepEqual(
				testUtils.lookupEnumValueName( "OCStackResult", result ),
				"OC_STACK_OK",
				"OCDevAddrToIPv4Addr has succeeded" );
			if ( iotivity.OCStackResult.OC_STACK_OK !== result ) {
				return;
			}

			result = iotivity.OCDevAddrToPort( address, portHolder );
			assert.deepEqual(
				testUtils.lookupEnumValueName( "OCStackResult", result ),
				"OC_STACK_OK",
				"OCDevAddrToPort has succeeded" );
			if ( iotivity.OCStackResult.OC_STACK_OK !== result ) {
				return;
			}

			return "coap://" +
				ipv4Bytes[ 0 ] + "." + ipv4Bytes[ 1 ] + "." +
				ipv4Bytes[ 2 ] + "." + ipv4Bytes[ 3 ] + ":" +
				portHolder.port + resourcePath;
		},

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

			// Make sure stack shutdown works
			testUtils.testShutdown();

			// Inform QUnit that this async test has concluded
			done();
		};

	assert.expect( 9 );

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

				// Issue the request and make sure it has worked
				result = iotivity.OCDoResource(
					handle,
					iotivity.OCMethod.OC_REST_GET,
					resourcePath,
					null,
					null,
					iotivity.OCConnectivityType.OC_ALL,
					iotivity.OCQualityOfService.OC_HIGH_QOS,
					function( handle, response ) {
						var url = getAbsoluteUrl( response.addr ),
							observeHandle = {};

						ok( true, "OCDoResource discovery handler was called" );

						if ( !url ) {
							teardown();
						} else {
							result = iotivity.OCDoResource(
								observeHandle,
								iotivity.OCMethod.OC_REST_OBSERVE,
								url,
								null,
								null,
								response.connType,
								iotivity.OCQualityOfService.OC_HIGH_QOS,
								function( handle, response ) {
									var payload, oic;

									payload = JSON.parse( response.resJSONPayload );
									oic = payload.oic || payload.oc;

									// Make sure the callback was called with the right response
									assert.deepEqual(
										oic[ 0 ],
										magicToken,
										"OCDoResource observation handler has received the " +
											"expected response" );

									teardown();
									return iotivity.OCStackApplicationResult
										.OC_STACK_DELETE_TRANSACTION;
								},
								null,
								0 );
						}
						return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
					},
					null,
					0 );
				assert.deepEqual(
					testUtils.lookupEnumValueName( "OCStackResult", result ),
					"OC_STACK_OK",
					"OCDoResource for discovery has succeeded" );
				assert.ok(
					handle.handle,
					"The resulting handle contains a non-null property named \"handle\"" );
				if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
					teardown();
				}
			},

			// If the test server fails to work correctly we cannot perform the test so we give up
			teardown,
			{
				payload: magicToken,
				path: resourcePath
			} );
	}
} );

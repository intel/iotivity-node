require( "../setup" );

var iotivity = require( "../../index" ),
	testUtils = require( "../test-utils" )( iotivity, QUnit.assert );

test( "Simple discovery", function( assert ) {
	var result, stopProcessing,
		done = assert.async(),
		failsafeTimeoutId = null,
		handle = {},
		teardown = function() {

			if ( stopProcessing ) {
				stopProcessing();
			}
			if ( failsafeTimeoutId !== null ) {
				clearTimeout( failsafeTimeoutId );
				failsafeTimeoutId = null;
			}
			assert.deepEqual( handlerWasCalled, true, "OCDoResource handler was called" );
			testUtils.testShutdown();
			done();
		};

	if ( testUtils.testStartup( iotivity.OCMode.OC_CLIENT ) ===
			iotivity.OCStackResult.OC_STACK_OK ) {

		stopProcessing = testUtils.testProcessing( 100, function() {
			teardown();
		} );

		failsafeTimeoutId = setTimeout( function() {
			failsafeTimeoutId = null;
			teardown();
		}, 5000 );

		result = iotivity.OCDoResource(
			handle,
			iotivity.OCMethod.OC_REST_GET,
			"light",
			null,
			null,
			iotivity.OCConnectivityType.OC_ALL,
			iotivity.OCQualityOfService.OC_HIGH_QOS,
			function( handle, response ) {
				handlerWasCalled = true;
				teardown();
			},
			null,
			0 );
	}
} );

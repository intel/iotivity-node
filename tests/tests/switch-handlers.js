require( "../setup" );

var iotivity = require( "../../lowlevel" ),
	testUtils = require( "../test-utils" )( iotivity, QUnit.assert );

test( "Switching handlers", function( assert ) {
	var result, stopProcessing, stopTestClient,
		done = assert.async(),
		callSequence = [],
		failsafeTimeoutId = null,
		handle = {},
		makeCallback = function( message, next ) {
			return function( flag, request ) {
				callSequence.push( message );
				if ( callSequence.length === 2 ) {
					teardown();
				} else {
					next();
				}
				return iotivity.OCEntityHandlerResult.OC_EH_OK;
			}
		},

		setUpSecondCall = function() {
			var rebindResult;

			if ( stopTestClient ) {
				stopTestClient();
				stopTestClient = null;
			}

			rebindResult = iotivity.OCBindResourceHandler(
				handle,
				makeCallback( "handler2", function() {} ) );

			assert.deepEqual(
				testUtils.lookupEnumValueName( "OCStackResult", rebindResult ),
				"OC_STACK_OK",
				"OCBindResourceHandler was successful" );

			stopTestClient = testUtils.startTestClient( function() {
				teardown();
			} );
		},

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

			// Make sure the entity handler was called
			assert.deepEqual(
				callSequence,
				[ "handler1", "handler2" ],
				"Handlers were called in the right sequence" );

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

	// Make sure the stack starts up correctly
	if ( testUtils.testStartup( iotivity.OCMode.OC_SERVER ) ===
			iotivity.OCStackResult.OC_STACK_OK ) {

		// Make sure OCCreateResource works correctly
		result = iotivity.OCCreateResource(
			handle,
			"core.light",
			"oc.mi.def",
			"/a/light",
			makeCallback( "handler1", function() {
				setTimeout( setUpSecondCall );
			} ),
			iotivity.OCResourceProperty.OC_DISCOVERABLE |
			iotivity.OCResourceProperty.OC_OBSERVABLE );
		assert.deepEqual(
			testUtils.lookupEnumValueName( "OCStackResult", result ),
			"OC_STACK_OK",
			"OCCreateResource succeeded" );

		// If OCCreateResource was successful, we can make sure the entity handler gets called
		if ( result === iotivity.OCStackResult.OC_STACK_OK ) {

			// Start the processing loop
			stopProcessing = testUtils.testProcessing( 100, function() {
				teardown();
			} );

			// If the entity handler does not get called within twenty seconds, give up
			failsafeTimeoutId = setTimeout( function() {
				failsafeTimeoutId = null;
				teardown();
			}, 20000 );

			// Start a test client which will send a request to the server under test
			stopTestClient = testUtils.startTestClient( function() {
				teardown();
			} );
		}
	}
} );

require( "../setup" );

var iotivity = require( "../../lowlevel" ),
	testUtils = require( "../test-utils" )( iotivity, QUnit.assert );

function discoverSequence( teardown, sequence ) {
	var stopTestClient,
		nextIterationId,
		index = 0,
		stop = function() {
			if ( stopTestClient ) {
				stopTestClient();
			}
			if ( nextIterationId ) {
				clearTimeout( nextIterationId );
			}
		},
		oneIteration = function() {
			nextIterationId = undefined;
			stopTestClient = testUtils.startTestClient( teardown, {
					path: iotivity.OC_MULTICAST_DISCOVERY_URI,
					logResponse: true
				},
				function discoverDataHandler( jsonObject ) {
					if ( !( "OCDoResource response" in jsonObject ) &&
							jsonObject.result !== iotivity.OCStackResult.OC_STACK_OK ) {
						teardown();
					}

					if ( jsonObject[ "OCDoResource response" ] ) {
						sequence[ index ]( jsonObject[ "OCDoResource response" ] );
						stopTestClient();
						stopTestClient = undefined;
						index++;
						if ( index < sequence.length ) {
							nextIterationId = setTimeout( oneIteration, 0 );
						}
					}
				} );
		};

	oneIteration();

	return stop;
}

// Collect the parts of the response that interest us
function constructActualResponse( prefix, resourcePath, response ) {
	var payload, index, item, oic;

	ok( response.resJSONPayload, prefix + ": resJSONPayload present" );
	if ( !response.resJSONPayload ) {
		return {};
	}

	payload = JSON.parse( response.resJSONPayload );
	oic = payload.oic || payload.oc;

	ok( oic, prefix + ": oic present in payload" );
	if ( !oic ) {
		return {};
	}

	for ( index in oic ) {
		if ( oic[ index ].href === resourcePath ) {
			item = oic[ index ];
		}
	}
	ok( item, prefix + ": " + resourcePath + " discovered" );

	ok( item.prop, prefix + ": resource has prop" );
	if ( !item.prop ) {
		return {};
	}

	return {
		rt: item.prop.rt,
		if: item.prop.if
	};
}

test( "Adding resource type and interface to resource", function( assert ) {
	var result, stopProcessing, stopDiscoverSequence,
		resourcePath = "/a/simple-server-" + Math.round( Math.random() * 10000 ),
		done = assert.async(),
		failsafeTimeoutId = null,
		handle = {},

		// Clean up before we exit
		teardown = function() {

			debugger;

			// Kill the test client process
			if ( stopDiscoverSequence ) {
				stopDiscoverSequence();
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
	assert.expect( 20 );

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
			}, 30000 );

			stopDiscoverSequence = discoverSequence( teardown, [
				function( response ) {
					deepEqual(
						constructActualResponse( "Initial", resourcePath, response ),
						{
							rt: [ "core.light" ],
							if: [ "oc.mi.def" ]
						},
						"Initial server description is correct" );
					iotivity.OCBindResourceInterfaceToResource( handle, "oc.mi.custom" );
				},
				function( response ) {
					deepEqual(
						constructActualResponse( "Additional interface", resourcePath, response ),
						{
							rt: [ "core.light" ],
							if: [ "oc.mi.def", "oc.mi.custom" ]
						},
						"Modified server description includes additional interface" );
					iotivity.OCBindResourceTypeToResource( handle, "core.fan" );
				},
				function( response ) {
					deepEqual(
						constructActualResponse(
							"Additional resource type",
							resourcePath,
							response ),
						{
							rt: [ "core.light", "core.fan" ],
							if: [ "oc.mi.def", "oc.mi.custom" ]
						},
						"Modified server description includes additional resource type" );
					teardown();
				},
			] );
		}
	}
} );

require( "../setup" );

var iotivity = require( "../../index" ),
	testUtils = require( "../test-utils" )( iotivity, QUnit.assert );

test( "Create/delete resource", function( assert ) {
	var result, keyCount,
		resource = {},
		callback = function testCallback( flag, request ) {};

	if ( testUtils.testStartup( iotivity.OCMode.OC_SERVER ) ===
			iotivity.OCStackResult.OC_STACK_OK ) {

		keyCount = Object.keys( iotivity._test_callbacks ).length;
		result = iotivity.OCCreateResource(
			resource,
			"light",
			"oc.mi.def",
			"/light/1",
			callback,
			iotivity.OCResourceProperty.OC_DISCOVERABLE );
		assert.deepEqual(
			testUtils.lookupEnumValueName( "OCStackResult", result ),
			"OC_STACK_OK",
			"OCCreateResource succeeded" );
		assert.deepEqual( typeof resource.uuid, "number", "uuid is present" );
		assert.deepEqual(
			Object.keys( iotivity._test_callbacks ).length,
			keyCount + 1,
			"The number of keys in the list of callbacks has increased by one" );
		assert.deepEqual( iotivity._test_callbacks[ resource.uuid ], callback,
			"The JS callback is present in the list of callbacks at the correct uuid" );

		keyCount = Object.keys( iotivity._test_callbacks ).length;
		result = iotivity.OCDeleteResource( resource );
		assert.deepEqual(
			testUtils.lookupEnumValueName( "OCStackResult", result ),
			"OC_STACK_OK",
			"OCDeleteResource succeeded" );
		assert.deepEqual(
			Object.keys( iotivity._test_callbacks ).length,
			keyCount - 1,
			"The number of keys in the list of callbacks has decreased by one" );

		testUtils.testShutdown();
	}
} );

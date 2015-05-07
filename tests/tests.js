debugger;

var iotivity = require( "../index" ),
	util = require( "util" );

function lookupEnumValueName( enumeration, value ) {
	var index;

	for ( index in enumeration ) {
		if ( enumeration[ index ] === value ) {
			return index;
		}
	}
}

function testStartup( assert ) {
	var result = iotivity.OCInit(
		null,
		0,
		iotivity.OCMode.OC_CLIENT_SERVER );

	assert.deepEqual(
		lookupEnumValueName(
			iotivity.OCStackResult,
			result ),
		"OC_STACK_OK",
		"OCInit succeeded" );

	return result;
}

function testShutdown( assert ) {
	assert.deepEqual(
		lookupEnumValueName(
			iotivity.OCStackResult,
			iotivity.OCStop() ),
		"OC_STACK_OK",
		"OCStop succeeded" );
}

// Each test requires that we start the stack before we run the test and stop the stack afterwards
function startStopStack( testBody ) {
	return function( assert ) {
		if ( testStartup( assert ) === iotivity.OCStackResult.OC_STACK_OK ) {
			testBody( assert );
			testShutdown( assert );
		}
	}
}

test( "Stack start/stop", startStopStack( function() {} ) );

test( "Create/delete resource", startStopStack( function( assert ) {
	var result, keyCount,
		resource = {},
		callback = function testCallback( flag, request ) {};

	keyCount = Object.keys( iotivity._test_callbacks ).length;
	result = iotivity.OCCreateResource(
		resource,
		"light",
		"oc.mi.def",
		"/light/1",
		callback,
		iotivity.OCResourceProperty.OC_DISCOVERABLE );
	assert.deepEqual(
		lookupEnumValueName( iotivity.OCStackResult, result ),
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
		lookupEnumValueName( iotivity.OCStackResult, result ),
		"OC_STACK_OK",
		"OCDeleteResource succeeded" );
	assert.deepEqual(
		Object.keys( iotivity._test_callbacks ).length,
		keyCount - 1,
		"The number of keys in the list of callbacks has decreased by one" );
} ) );

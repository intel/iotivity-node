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

// Each test requires that we start the stack before we run the test and stop the stack afterwards
function startStopStack( testBody ) {
	return function() {
		QUnit.assert.deepEqual(
			lookupEnumValueName(
				iotivity.OCStackResult,
				iotivity.OCInit(
					"0.0.0.0",
					10123,
					iotivity.OCMode.OC_CLIENT_SERVER ) ),
			"OC_STACK_OK",
			"OCInit succeeded" );

		testBody();

		QUnit.assert.deepEqual(
			lookupEnumValueName(
				iotivity.OCStackResult,
				iotivity.OCStop() ),
			"OC_STACK_OK",
			"OCStop succeeded" );
	}
}

test( "Stack start/stop", startStopStack( function() {} ) );

test( "Create/delete resource", startStopStack( function() {
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
	QUnit.assert.deepEqual(
		lookupEnumValueName( iotivity.OCStackResult, result ),
		"OC_STACK_OK",
		"OCCreateResource succeeded" );
	QUnit.assert.deepEqual( typeof resource[ 1 ], "number",
		"uuid is present at return array index 1" );
	QUnit.assert.deepEqual(
		Object.keys( iotivity._test_callbacks ).length,
		keyCount + 1,
		"The number of keys in the list of callbacks has increased by one" );
	QUnit.assert.deepEqual( iotivity._test_callbacks[ resource[ 1 ] ], callback,
		"The JS callback is present in the list of callbacks at the correct uuid" );

	keyCount = Object.keys( iotivity._test_callbacks ).length;
	result = iotivity.OCDeleteResource( resource );
	QUnit.assert.deepEqual(
		lookupEnumValueName( iotivity.OCStackResult, result ),
		"OC_STACK_OK",
		"OCDeleteResource succeeded" );
	QUnit.assert.deepEqual(
		Object.keys( iotivity._test_callbacks ).length,
		keyCount - 1,
		"The number of keys in the list of callbacks has decreased by one" );
} ) );

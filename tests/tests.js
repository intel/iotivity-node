debugger;

var iotivity = require( "../index" ),
	util = require( "util" ),
	eventLog = [];

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
			"OCInit success" );

		testBody();

		QUnit.assert.deepEqual(
			lookupEnumValueName(
				iotivity.OCStackResult,
				iotivity.OCStop() ),
			"OC_STACK_OK",
			"OCStop success" );
	}
}

iotivity._log = function() {
	eventLog.push( arguments );
};

test( "Stack start/stop", startStopStack( function() {} ) );

test( "Create/delete resource", startStopStack( function() {
	var resource,
		callback = function testCallback( flag, request ) {};

	resource = iotivity.OCCreateResource(
		"light",
		"oc.mi.def",
		"/light/1",
		callback,
		iotivity.OCResourceProperty.OC_DISCOVERABLE );
	QUnit.assert.deepEqual( util.isArray( resource ), true, "OCCreateResource returns an array" );
	QUnit.assert.deepEqual( resource.length, 3, "Returned array has length 3" );
	QUnit.assert.deepEqual(
		lookupEnumValueName( iotivity.OCStackResult, resource[ 0 ] ),
		"OC_STACK_OK",
		"OCCreateResource succeeded" );
	QUnit.assert.deepEqual( typeof resource[ 2 ], "number",
		"uuid is present at return array index 2" );
	QUnit.assert.deepEqual( iotivity._callbacks.callbacks[ resource[ 2 ] ], callback,
		"uuid identifies the correct callback" );

	QUnit.assert.deepEqual(
		lookupEnumValueName(
			iotivity.OCStackResult,
			iotivity.OCDeleteResource( resource ) ),
		"OC_STACK_OK",
		"OCDeleteResource succeeded" );
	QUnit.assert.deepEqual(
		( resource[ 2 ] in iotivity._callbacks.callbacks ),
		false,
		"callback at uuid is removed from list of callbacks" );
} ) );

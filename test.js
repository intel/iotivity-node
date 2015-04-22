var iotivity = require( "./index" ),
	util = require( "util" ),
	success = "✓",
	failure = "✗";

function lookupEnumValueName( enumeration, value ) {
	var index;

	for ( index in enumeration ) {
		if ( enumeration[ index ] === value ) {
			return index;
		}
	}
}

function assert( predicate, message ) {
	var result = ( typeof predicate === "boolean" ? predicate : predicate() );

	console.log( ( result ? success : failure ) + ": " + message );
	if ( !result ) {
		throw "Bailing";
	}
}

var tests = [];

function test( name, body ) {
	tests.push( { name: name, test: body } );
}

function startStopStack( testBody ) {
	return function() {
		var stackStatus = iotivity.OCInit(
			"0.0.0.0",
			10123,
			iotivity.OCMode.OC_CLIENT_SERVER );

		assert( stackStatus === iotivity.OCStackResult.OC_STACK_OK,
			"Stack started as OC_CLIENT_SERVER (return status: " +
				lookupEnumValueName( iotivity.OCStackResult, stackStatus ) + ")" );

		testBody();

		stackStatus = iotivity.OCStop();

		assert( stackStatus === iotivity.OCStackResult.OC_STACK_OK,
			"Stack stopped (return status: " +
				lookupEnumValueName( iotivity.OCStackResult, stackStatus ) + ")" );
	}
}

test( "Stack start/stop", startStopStack( function() {} ) );

test( "Create/delete resource", startStopStack( function() {
	var resource,
		stackStatus,
		callback = function testCallback( flag, request ) {};

	resource = iotivity.OCCreateResource(
		"light",
		"oc.mi.def",
		"/light/1",
		callback,
		iotivity.OCResourceProperty.OC_DISCOVERABLE );

	assert( util.isArray( resource ), "OCCreateResource returns an array" );
	assert( resource.length === 3, "Array returned by OCCreateResource has length 3" );
	assert( resource[ 0 ] === iotivity.OCStackResult.OC_STACK_OK,
		"OCCreateResource returned OC_STACK_OK" );
	assert( typeof resource[ 2 ] === "number", "uuid is present" );
	assert( iotivity._callbacks.callbacks[ resource[ 2 ] ] === callback,
		"uuid identifies the correct callback" );

	stackStatus = iotivity.OCDeleteResource( resource );
	assert( stackStatus === iotivity.OCStackResult.OC_STACK_OK,
		"OCDeleteResource returns OC_STACK_OK (return status: " +
			lookupEnumValueName( iotivity.OCStackResult, stackStatus ) + ")" );
	assert( !( resource[ 2 ] in iotivity._callbacks.callbacks ),
		"callback at uuid is removed from list of callbacks" );
} ) );

var index;

for ( index in tests ) {
	console.log( "*** Test: " + tests[ index ].name );
	tests[ index ].test();
	if ( index < tests.length - 1 ) {
		console.log( "" );
	}
}

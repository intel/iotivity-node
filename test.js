var iotivity = require( "./index" ),
	success = "✓",
	failure = "✗";

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

test( "Stack start/stop", function() {
	assert(
		iotivity.OCInit(
			"0.0.0.0",
			10123,
			iotivity.OC_CLIENT_SERVER ) === iotivity.OC_STACK_OK,
		"Stack started as OC_CLIENT_SERVER" );
	assert( iotivity.OCStop() === iotivity.OC_STACK_OK, "Stack stopped" );
} );

var index;

for ( index in tests ) {
	console.log( "*** Test: " + tests[ index ].name );
	tests[ index ].test();
	if ( index < tests.length - 1 ) {
		console.log( "" );
	}
}

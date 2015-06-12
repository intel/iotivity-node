require( "../setup" );

var OicDevice = require( "../../index" );

test( "Start the stack", function( assert ) {
	var done = assert.async();

	expect( 1 );

	OicDevice().configure().then(
		function() {
			ok( true, "Stack started successfully" );
			done();
		},
		function( error ) {
			ok( false, "Stack failed to start with code: " + error.result );
			done();
		} );
} );

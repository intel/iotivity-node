var QUnit = require( "../setup" ),
	OicDevice = require( "../../index" ).OicDevice;

QUnit.test( "Start the stack", function( assert ) {
	var done = assert.async();

	assert.expect( 1 );

	OicDevice().configure().then(
		function() {
			assert.ok( true, "Stack started successfully" );
			done();
		},
		function( error ) {
			assert.ok( false, "Stack failed to start with code: " + error.result );
			done();
		} );
} );

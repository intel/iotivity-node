var testUtils = require( "../assert-to-console" );

console.log( JSON.stringify( { assertionCount: 1 } ) );

require( "../../index" )().configure().then(
	function() {
		testUtils.assert( "ok", true, "Stack started successfully" );
		process.exit( 0 );
	},
	function( error ) {
		testUtils.assert( "ok", false, "Stack failed to start with code: " + error.result );
		process.exit( 0 );
	} );

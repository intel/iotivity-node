var testUtils = require( "../assert-to-console" );

console.log( JSON.stringify( { assertionCount: 1 } ) );

var device = require( "../../index" )();
var settings = {
	role: "server"
};

device.configure( settings );
device.enablePresence( 1000 );
device.disablePresence().then(
	function() {
		testUtils.assert( "ok", true, "Presence disabled successfully" );
		process.exit( 0 );
	},
	function( error ) {
		testUtils.assert( "ok", false, "Presence disabled error with code: " + error.result );
		process.exit( 0 );
	} );

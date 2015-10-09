var utils = require( "../assert-to-console" ),
	dlopennow = require( "bindings" )( "dlopennow" ).dlopennow,
	exec = require( "child_process" ).exec;

// Report assertion count
console.log( JSON.stringify( { assertionCount: 1 } ) );

exec( "node -e 'console.log( JSON.stringify( {" +
"path: require( \"bindings\" )( \"iotivity\" ).path " +
"} ) );'",
	function( error, stdout ) {
	if ( error !== null ) {
		utils.die( "Failed to load iotivity in child process: " + error );
	}
	var filename = JSON.parse( stdout.toString() ).path,
		result = dlopennow( filename );

	utils.assert( "strictEqual", result, undefined, "dlopennow() of iotivity was successful" );
	} );

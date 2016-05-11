var utils = require( "../assert-to-console" );

var requirePath = require( "path" )
	.join( require( "bindings" ).getRoot( __filename ), "index" );

console.log( JSON.stringify( { assertionCount: 4 } ) );

var stderr = "";
var stdout = "";

var childArguments = [ "-e",
		"try { require( '" + requirePath + "' )( 'server' ); } " +
		"catch( anError ) { " +
			"console.error( anError.stack ); process.exit( 1 ); " +
		"}" +
		"process.exit( 0 );"
	];

var theChild = require( "child_process" ).spawn( "node", childArguments )
	.on( "exit", function( code, signal ) {
		utils.assert( "strictEqual", code, 0, "require() from interactive shell succeeded" );
		utils.assert( "strictEqual", signal, null,
			"require() from interactive shell did not receive a signal" );
		utils.assert( "strictEqual", stdout, "", "Process stdout is empty" );
		utils.assert( "strictEqual", stderr, "", "Process stderr is empty" );
	} );

theChild.stdout.on( "data", function( data ) {
	stdout += data.toString();
} );
theChild.stderr.on( "data", function( data ) {
	stderr += data.toString();
} );

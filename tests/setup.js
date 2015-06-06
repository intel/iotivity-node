require( "qunitjs/qunit/qunit" );

var success = "\033[42;30m✓\033[0m",
	failure = "\033[41;30m✗\033[0m";

// Right-align runtime in a field that's 10 columns wide
function formatRuntime( runtime ) {
	var index,
		str = "" + runtime,
		indent = "";

	for ( index = 0 ; index < Math.max( 0, 10 - str.length ) ; index++ ) {
		indent += " ";
	}

	return indent + str;
}

QUnit.init();
QUnit.config.testTimeout = 30000;
QUnit.config.blocking = false;
QUnit.config.autorun = true;
QUnit.config.updateRate = 0;
QUnit.config.callbacks.moduleStart.push( function( status ) {

	// status: { name, tests }

	if ( status.name ) {
		console.log( "\n### " + status.name );
	}
} );
QUnit.config.callbacks.testStart.push( function( status ) {

	// status: { name, module, testId }

	if ( status.name ) {
		console.log( "\n" + status.name );
	}
} );
QUnit.config.callbacks.log.push( function( status ) {

	// status: { module, result(t/f), message, actual, expected, testId, runtime }

	console.log(
		( status.result ? success : failure ) +
		" @" + formatRuntime( status.runtime ) + ": " +
		status.message );
	if ( !status.result ) {
		console.log( "Actual: " );
		console.log( QUnit.dump.parse( status.actual ) );
		console.log( "Expected: " );
		console.log( QUnit.dump.parse( status.expected ) );
	}
} );

QUnit.config.callbacks.done.push( function( status ) {
	var passed = "\033[42;30m " + status.passed + " \033[0m",
		failed = "\033[41;30m " + status.failed + " \033[0m";

	console.log( "Total assertions: " +
		"(" + passed + ( status.failed > 0 ? "+" + failed : "" ) + ") / " + status.total );

	process.exit( status.failed > 0 ? 1 : 0 );
} );

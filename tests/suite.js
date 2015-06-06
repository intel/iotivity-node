var path = require( "path" ),
	async = require( "async" ),
	fs = require( "fs" ),
	child_process = require( "child_process" ),
	testsPath = path.join( __dirname, "tests" );

// Spawn all tests in ./tests/ in separate node instances sequentially
fs.readdir( testsPath, function( error, files ) {
	var failures = [];

	if ( error ) {
		throw error;
	} else {
		async.eachSeries( files,
			function( item, callback ) {
				var singleTest = path.join( testsPath, item );
				child_process
					.spawn(
						"node",
						[ singleTest ],
						{ stdio: "inherit" } )
					.on( "exit", function( code, signal ) {
						if ( !( ( code === 0 || code === null ) && signal !== "SIGSEGV" ) ) {
							failures.push( {
								test: singleTest,
								code: code,
								signal: signal
							} );
						}
						callback( null );
					} );
			},
			function( error ) {
				var index, message, failure;

				if ( failures.length > 0 ) {
					for ( index = 0, message = "\n" ; index < failures.length ; index++ ) {
						failure = failures[ index ];
						message += failure.test + " exited with " + failure.code +
							( failure.signal ? " and signal " + failure.signal : "" ) + "\n";
					}
					throw new Error( message );
				}
			} );
	}
} );

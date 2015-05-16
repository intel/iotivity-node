var path = require( "path" ),
	async = require( "async" ),
	fs = require( "fs" ),
	child_process = require( "child_process" ),
	testsPath = path.join( __dirname, "tests" );

// Spawn all tests in ./tests/ in separate node instances sequentially
fs.readdir( testsPath, function( error, files ) {
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
						callback( ( ( code === 0 || code === null ) && signal !== "SIGSEGV" ) ?
							null :
							{
								test: singleTest,
								code: code,
								signal: signal
							} );
					} );
			},
			function( error ) {
				if ( error ) {
					throw new Error( error.test + " exited with code " +
						error.code + " and signal " + error.signal + "." );
				}
			} );
	}
} );

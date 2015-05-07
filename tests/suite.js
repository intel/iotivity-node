var path = require( "path" ),
	fs = require( "fs" ),
	child_process = require( "child_process" ),
	testsPath = path.join( __dirname, "tests" );

// Spawn all tests in ./tests/ in separate node instances sequentially
fs.readdir( testsPath, function( error, files ) {
	var index;

	if ( error ) {
		throw error;
	} else {
		for ( index in files ) {
			child_process.spawnSync(
				"node",
				[ path.join( testsPath, files[ index ] ) ],
				{ stdio: "inherit" } );
		}
	}
} );

var QUnit, suites,
	_ = require( "underscore" ),
	childProcess = require( "child_process" ),
	fs = require( "fs" ),
	path = require( "path" ),
	uuid = require( "uuid" ),
	runningProcesses = [],
	getQUnit = function() {
		if ( !QUnit ) {
			QUnit = require( "./setup" );
		}
		return QUnit;
	};

// Spawn a single child and process its stdout.
function spawnOne( assert, options ) {
	var theChild = childProcess.spawn(
		"node",
		[ options.path ].concat( options.uuid ? [ options.uuid ] : [] ),
		{
			stdio: [ process.stdin, "pipe", process.stderr ]
		} );

	theChild.commandLine = "node" + " " + options.path + " " + options.uuid;
	runningProcesses.push( theChild );

	theChild
		.on( "exit", function( code, signal ) {
			var exitCodeOK = ( code === 0 || code === null ),
				signalOK = ( signal !== "SIGSEGV" );

			assert.ok( exitCodeOK, options.name + " exited successfully (" + code + ")" );
			assert.ok( signalOK, options.name + " did not segfault" );
		} )
		.on( "close", function() {
			var childIndex = runningProcesses.indexOf( theChild );
			if ( childIndex >= 0 ) {
				runningProcesses.splice( childIndex, 1 );
			}
			options.maybeQuit();
		} );

	// The stdout of the child is a sequence of \n-separated stringified JSON objects.
	theChild.stdout.on( "data", function serverStdoutData( data ) {
		_.each( data.toString().split( "\n" ), function( value ) {
			var jsonObject;

			if ( !value ) {
				return;
			}

			// Attempt to retrieve a JSON object from stdout.
			try {
				jsonObject = JSON.parse( value );
			} catch ( e ) {
				options.teardown( "Error parsing " + options.name + " JSON: '" + value + "'" +
					( e.message ? e.message : e ), true );
			}

			// The child is reporting the number of assertions it will be making. We add our own
			// two assertions ( 1.) successful exit and 2.) no segfault) to that count.
			if ( jsonObject.assertionCount ) {
				options.reportAssertions( jsonObject.assertionCount + 2 );

			// The child has requested a teardown.
			} else if ( jsonObject.teardown ) {
				options.teardown(
					options.name + " requested teardown: " + jsonObject.message );

			// The child has requested that its peer be killed.
			} else if ( jsonObject.killPeer ) {
				if ( options.killPeer ) {
					options.killPeer( theChild );
				}

			// The child is reporting that it is ready. Only servers do this.
			} else if ( jsonObject.ready ) {
				if ( options.onReady ) {
					options.onReady();
				}

			// The child is making an assertion.
			} else if ( jsonObject.assertion ) {
				assert[ jsonObject.assertion ].apply( assert, jsonObject.arguments );

			// Otherwise, we have received unknown JSON from the child - bail.
			} else {
				options.teardown( "Unkown JSON from " + options.name + ": " + value, true );
			}
		} );
	} );

	return theChild;
}

function runTestSuites( files ) {
	_.each( files, function( item ) {
		var singleTest = path.join( __dirname, "tests", item ),
			clientPath = path.join( singleTest, "client.js" ),
			serverPath = path.join( singleTest, "server.js" );

		if ( fs.lstatSync( singleTest ).isFile() ) {
			getQUnit().test( item.replace( /\.js$/, "" ), function( assert ) {
				var theChild,
					spawnOptions = {
						uuid: uuid.v4(),
						name: "Test",
						path: singleTest,
						teardown: function() {
							if ( theChild ) {
								theChild.kill( "SIGTERM" );
							}
						},
						maybeQuit: assert.async(),
						reportAssertions: _.bind( assert.expect, assert )
					};
				theChild = spawnOne( assert, spawnOptions );
			} );
			return;
		}

		if ( !fs.lstatSync( singleTest ).isDirectory() ) {
			return;
		}

		if ( !( fs.lstatSync( clientPath ).isFile() ) ) {
			throw new Error( "Cannot find client at " + clientPath );
		}

		if ( !( fs.lstatSync( serverPath ).isFile() ) ) {
			throw new Error( "Cannot find server at " + serverPath );
		}

		getQUnit().test( item, function( assert ) {
			var client, server,

				// Turn this test async
				done = assert.async(),

				// Count how many children have exited. Consider the test done when that number
				// reaches two (the client and the server).
				childrenExited = 0,

				// Count assertions made by the children. Report them to assert.expect() when both
				// children have reported their number of assertions.
				totalAssertions = 0,
				childrenAssertionsReported = 0,

				spawnOptions = {
					uuid: uuid.v4(),
					teardown: function( message ) {
						if ( client ) {
							client.kill( "SIGTERM" );
						}
						if ( server ) {
							server.kill( "SIGTERM" );
						}
						throw new Error( message );
					},
					killPeer: function( whosePeer ) {
						var thePeer = ( whosePeer === client ? server : client );

						if ( thePeer ) {
							thePeer.kill( "SIGINT" );
						}
					},
					maybeQuit: function() {
						childrenExited++;
						if ( childrenExited == 2 ) {
							done();
						}
					},
					reportAssertions: function( assertionCount ) {
						childrenAssertionsReported++;
						totalAssertions += assertionCount;
						if ( childrenAssertionsReported == 2 ) {
							assert.expect( totalAssertions );
						}
					}
				};

			// We run the server first, because the server has to be there before the client
			// can run. OTOH, if the client exits successfully, we may need to kill the server,
			// because the server may be designed to run "forever".
			server = spawnOne( assert, _.extend( {}, spawnOptions, {
				name: "server",
				path: serverPath,
				onReady: function() {
					client = spawnOne( assert, _.extend( {}, spawnOptions, {
						name: "client",
						path: clientPath
					} ) );
				}
			} ) );
		} );
	} );
}

// Process low level API tests. If no tests were specified on the command line, we scan the tests
// directory and run all the tests we find therein.
if ( process.argv.length > 2 ) {
	runTestSuites( process.argv[ 2 ].split( "," ) );
} else {
	fs.readdir( path.join( __dirname, "tests" ), function( error, files ) {
		if ( error ) {
			throw error;
		}

		runTestSuites( files );
	} );
}

process.on( "exit", function() {
	var childIndex;

	for ( childIndex in runningProcesses ) {
		runningProcesses[ childIndex ].kill( "SIGTERM" );
	}
} );

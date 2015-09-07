var QUnit,
	_ = require( "underscore" ),
	child_process = require( "child_process" ),
	fs = require( "fs" ),
	path = require( "path" ),
	uuid = require( "uuid" );

// Spawn a single child and process its stdout.
function spawnOne( assert, options ) {
	var theChild = child_process.spawn( "node", [ options.path, options.uuid ], {
		stdio: [ process.stdin, "pipe", process.stderr ]
	} );

	theChild
		.on( "exit", function( code, signal ) {
			var exitCodeOK = ( code === 0 || code === null ),
				signalOK = ( signal !== "SIGSEGV" );

			assert.ok( exitCodeOK, options.name + " exited successfully (" + code + ")" );
			assert.ok( signalOK, options.name + " did not segfault" );
		} )
		.on( "close", function() {
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
			} catch( e ) {
				options.teardown( "Error parsing " + options.name + " JSON: '" + value + "'" +
					( e.message ? e.message : e ), true );
			}

			// The child is reporting the number of assertions it will be making.
			if ( jsonObject.assertionCount ) {
				options.reportAssertions( jsonObject.assertionCount );

			// The child has requested a teardown.
			} else if ( jsonObject.teardown ) {
				options.teardown(
					options.name + " requested teardown: " + jsonObject.message );

			// The child has requested that its peer be killed.
			} else if ( jsonObject.killPeer ) {
				options.killPeer( theChild );

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

// process low level API tests
fs.readdir( path.join( __dirname, "tests" ), function( error, files ) {
	if ( error ) {
		throw error;
	}

	_.each( files, function( item ) {
		var
			singleTest = path.join( __dirname, "tests", item ),
			clientPath = path.join( singleTest, "client.js" ),
			serverPath = path.join( singleTest, "server.js" );

		if ( !fs.lstatSync( singleTest ).isDirectory() ) {
			callback( null );
			return;
		}

		if ( !( fs.lstatSync( clientPath ).isFile() ) ) {
			throw new Error( "Cannot find client at " + clientPath );
		}

		if ( !( fs.lstatSync( serverPath ).isFile() ) ) {
			throw new Error( "Cannot find server at " + serverPath );
		}

		if ( !QUnit ) {
			QUnit = require( "./setup" );
		}

		QUnit.test( item, function( assert ) {
			var client, server,

				// Turn this test async
				done = assert.async(),

				// Count how many children have exited. Consider the test done when that number
				// reaches two (the client and the server).
				childrenExited = 0,

				// Count assertions, including those made by the children. Report them to
				// assert.expect() when both children have reported their number of assertions.
				// Initially there are 4 assertions, because spawnOne() makes two assertions:
				// - that the process has exited with success
				// - that the process has not segfaulted
				// It makes these two assertions once for the client, and once for the server
				totalAssertions = 4,
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
} );

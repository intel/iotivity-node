var device = require( "../../index" )();
var async = require( "async" );
var testUtils = require( "../assert-to-console" );
var uuid = process.argv[ 2 ];

function runAsClient() {
	var currentServer, theResource;
	var _ = require( "lodash" );
	var childProcess = require( "child_process" );
	var launchServer = function( callback ) {
		currentServer = childProcess.spawn( "node", [
			process.argv[ 1 ], "server", uuid
		], {
			stdio: [ process.stdin, "pipe", process.stderr ]
		} );

		currentServer.on( "exit", function( code, signal ) {
			var exitCodeOK = ( code === 0 || code === null ),
				signalOK = ( signal !== "SIGSEGV" );

			if ( !( exitCodeOK && signalOK ) ) {
				callback( "Server died with code " + code + " and signal " + signal );
			}
			currentServer = null;
		} );

		currentServer.stdout.on( "data", function serverStdoutData( data ) {
			_.each( data.toString().split( "\n" ), function stdoutLineHandler( value ) {
				var jsonObject;

				if ( !value ) {
					return;
				}

				try {
					jsonObject = JSON.parse( value );
				} catch ( theError ) {
					currentServer.kill( "SIGTERM" );
				}

				// When the server signals that it's ready, proceed with the discovery
				if ( jsonObject.ready ) {
					testUtils.assert( "ok", true, "Server reports ready" );
					callback();
				}

				// Forward assertions and teardown requests from server
				if ( jsonObject.assert || jsonObject.teardown ) {
					console.log( JSON.stringify( jsonObject ) );
				}
			} );
		} );
	};
	var discoverResource = function( callback ) {
		var teardown;
		var resourcefoundHandler = function( event ) {
			if ( event.resource.id.path === "/a/" + uuid ) {
				testUtils.assert( "ok", true, "Client: Resource found" );
				theResource = event.resource;
				teardown();
			}
		};
		teardown = function( error ) {
			device.removeEventListener( "resourcefound", resourcefoundHandler );
			callback( error );
		};
		device.addEventListener( "resourcefound", resourcefoundHandler );
		device.findResources().catch( teardown );
	};
	var waitForDelete = function( callback ) {
		var teardownPreconditions = [];
		var maybeTeardown = function( error ) {
			teardownPreconditions.push( error );
			if ( teardownPreconditions.length === 2 ) {
				callback( ( teardownPreconditions[ 0 ] || teardownPreconditions[ 1 ] ) ?
					teardownPreconditions : null );
			}
		};
		var deleteHandler = function() {
			testUtils.assert( "ok", true, "Client: Delete event received" );
			theResource.removeEventListener( "delete", deleteHandler );
			theResource = null;

			// Issue second SIGINT, causing the server to quit
			currentServer.kill( "SIGINT" );
			maybeTeardown();
		};
		theResource.addEventListener( "delete", deleteHandler );
		currentServer.on( "exit", function() {
			testUtils.assert( "ok", true, "Client: Server has exited" );
			maybeTeardown();
		} );

		// Issue first SIGINT, causing presence to be disabled
		currentServer.kill( "SIGINT" );
	};

	console.log( JSON.stringify( { assertionCount: 9 } ) );

	async.series( [
		function configureAsClient( callback ) {
			device.configure( { role: "client" } ).then(
				function() {
					testUtils.assert( "ok", true, "Client: configure() was successful" );
					callback();
				}, callback );
		},
		launchServer,
		discoverResource,
		waitForDelete,
		launchServer,
		discoverResource,
		waitForDelete
	], function( error ) {
		if ( error ) {
			testUtils.die( error );
		} else {
			process.exit( 0 );
		}
	} );

	process.on( "exit", function clientOnExit() {
		if ( currentServer ) {
			currentServer.kill( "SIGTERM" );
		}
	} );
}

function runAsServer() {
	var theResource,
		sigintCount = 0;

	// Two-level SIGINT: The first SIGINT causes the server to disable presence, whereas the second
	// one causes the server to exit.
	process.on( "SIGINT", function() {
		if ( sigintCount === 0 ) {
			sigintCount = 1;
			device.disablePresence().then(
				function() {
					testUtils.assert( "ok", true, "Server: disablePresence() was successful" );
				},
				function( error ) {
					testUtils.die( "Server: disablePresence failed with error: " +
						JSON.stringify( error ) );
				} );
		} else {
			device.unregisterResource( theResource ).then(
				function() {
					testUtils.assert( "ok", true, "Server: unregisterResource() was successful" );
					process.exit( 0 );
				},
				function( error ) {
					testUtils.die( "Server: unregisterResource failed with error: " +
						JSON.stringify( error ) );
				} );
		}
	} );

	async.series( [

		function configureServer( callback ) {
			device.configure( { role: "server" } ).then(
				function() {
					testUtils.assert( "ok", true, "Server: configure() was successful" );
					callback();
				}, callback );
		},

		function registerResource( callback ) {
			device.registerResource( {
				id: { path: "/a/" + process.argv[ 3 ] },
				interfaces: [ "oic.if.baseline" ],
				resourceTypes: [ "core.light" ],
				discoverable: true
			} ).then(
				function( resource ) {
					theResource = resource;
					testUtils.assert( "ok", true, "Server: registerResource() was successful" );
					callback();
				}, callback );
		},

		function enablePresence( callback ) {
			device.enablePresence().then(
				function() {
					testUtils.assert( "ok", true, "Server: enablePresence() was successful" );
					callback();
				}, callback );
		},

		function signalReady( callback ) {
			console.log( JSON.stringify( { ready: true } ) );
			callback();
		}
	], function( error ) {
		if ( error ) {
			testUtils.die( "Server: Fatal error: " + JSON.stringify( error ) );
		}
	} );
}

if ( process.argv[ 2 ] === "server" ) {
	runAsServer();
} else {
	runAsClient();
}

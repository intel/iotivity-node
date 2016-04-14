// Copyright 2016 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

var uuid = process.argv[ 2 ];

function runAsServer() {
	var resource;
	var device = require( "../../index" )( "server" );

	process.on( "SIGINT", function() {
		device.unregisterResource( resource ).then(
			function() {
				process.exit( 0 );
			},
			function() {
				process.exit( 1 );
			} );
	} );

	device.registerResource( {
		id: { path: "/a/" + process.argv[ 3 ] },
		discoverable: true,
		resourceTypes: [ "core.light" ],
		interfaces: [ "oic.if.baseline" ]
	} ).then(
		function( theResource ) {
			resource = theResource;
			console.log( JSON.stringify( { ready: true } ) );
		},
		function() {
			process.exit( 1 );
		} );
}

function runAsClient() {
	var currentServer, serverId;
	var _ = require( "lodash" );
	var childProcess = require( "child_process" );
	var testUtils = require( "../assert-to-console" );
	var device = require( "../../index" )( "client" );
	var launchServer = function( callback ) {
		currentServer = childProcess.spawn( "node", [
			process.argv[ 1 ], "server", uuid
		], {
			stdio: [ process.stdin, "pipe", process.stderr ]
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
					callback( theError );
				}

				// When the server signals that it's ready, proceed with the discovery
				if ( jsonObject.ready ) {
					testUtils.assert( "ok", true, "Server reports ready" );
					callback();
				}
			} );
		} );
	};
	var retrieveServerIdAndKillServer = function( callback ) {
		var teardown,
			resourceFoundHandler = function( event ) {
				if ( event.resource.id.path === "/a/" + uuid ) {
					testUtils.assert( "ok", true, "Resource found" );
					if ( serverId ) {
						testUtils.assert( "strictEqual", serverId, event.resource.id.deviceId,
							"Device ID is the same the second time around" );
					} else {
						serverId = event.resource.id.deviceId;
					}

					currentServer.on( "close", function serverOnClose( code, signal ) {
						var exitCodeOK = ( code === 0 || code === null ),
							signalOK = ( signal !== "SIGSEGV" );

						currentServer = null;
						teardown( ( exitCodeOK && signalOK ) ? null : new Error( "Server died" ) );
					} );
					currentServer.kill( "SIGINT" );
				}
			};
		teardown = function( error ) {
			device.removeEventListener( "resourcefound", resourceFoundHandler );
			callback( error );
		};
		device.addEventListener( "resourcefound", resourceFoundHandler );
		device.findResources().then(
			function() {
				testUtils.assert( "ok", true, "findResources() was successful" );
			},
			teardown );
	};

	console.log( JSON.stringify( { assertionCount: 7 } ) );

	require( "async" ).series( [
		launchServer,
		retrieveServerIdAndKillServer,
		launchServer,
		retrieveServerIdAndKillServer
	], function( error ) {
		if ( error ) {
			testUtils.die( error );
		} else {
			process.exit( 0 );
		}
	} );

	process.on( "close", function clientOnClose() {
		if ( currentServer ) {
			currentServer.kill( "SIGTERM" );
		}
	} );
}

if ( process.argv[ 2 ] === "server" ) {
	runAsServer();
} else {
	runAsClient();
}

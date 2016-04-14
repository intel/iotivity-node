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

var theResource,
	requestCount = 0,
	utils = require( "../../assert-to-console" ),
	uuid = process.argv[ 2 ],
	device = require( "../../../index" )( "server" );

console.log( JSON.stringify( { assertionCount: 5 } ) );

utils.assert( "ok", true, "Server: device configured successfully" );

device.addEventListener( "updaterequest", function( request ) {
	requestCount++;

	if ( request.res.op === "cyclePresence" ) {
		setTimeout( function() {
			device.disablePresence().then(
				function() {
					utils.assert( "ok", true,
						"Server: device.disablePresence() successful" );
				},
				function( error ) {
					utils.assert( "ok", false,
						"Server: device.disablePresence() failed with: " + error );
				} );
			setTimeout( function() {
				device.enablePresence().then(
					function() {
						utils.assert( "ok", true,
							"Server: device.enablePresence() successful" );
					},
					function( error ) {
						utils.assert( "ok", false,
							"Server: device.enablePresence() failed with: " + error );
					} );
			}, 5000 );
		}, 5000 );
	}

	request.sendResponse( null );
} );

device.enablePresence().then(
	function() {
		device.registerResource( {
			id: { path: "/a/" + uuid },
			deviceId: uuid,
			resourceTypes: [ "core.light" ],
			interfaces: [ "oic.if.baseline" ],
			discoverable: true,
			properties: { someValue: 0 }
		} ).then(
			function( resource ) {
				theResource = resource;
				utils.assert( "ok", true, "Server: device.registerResource() successful" );

				// Signal to the test suite that we're ready for the client
				console.log( JSON.stringify( { ready: true } ) );
			},
			function( error ) {
				utils.assert( "ok", false,
					"Server: device.registerResource() failed with: " + error );
			} );
	}, function( error ) {
		utils.assert( "ok", false,
			"Server: device.enablePresence() failed with: " + error );
	} );

// Cleanup on SIGINT
process.on( "SIGINT", function() {
	device.unregisterResource( theResource ).then(
		function() {
			utils.assert( "ok", true, "Server: device.unregisterResource() successful" );
			process.exit( 0 );
		},
		function( error ) {
			utils.assert( "ok", false,
				"Server: device.unregisterResource() failed with: " + error );
			process.exit( 0 );
		} );
} );

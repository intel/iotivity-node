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

module.exports = function( serverIndex ) {

var theResource,
	requestCount = 0,
	utils = require( "../../assert-to-console" ),
	uuid = process.argv[ 2 ],
	device = require( "../../../index" )( "server" );

console.log( JSON.stringify( { assertionCount: 8 } ) );

utils.assert( "ok", true, "Server " + serverIndex + ": device configured successfully" );

device.addEventListener( "updaterequest", function( request ) {
	requestCount++;

	function sendResponse( error ) {
		utils.assert( "strictEqual", ( error ? ( "" + error ) : "" ), "", "Server " +
			serverIndex + ": " + request.res.op + "()" );
		request[ error ? "sendError" : "sendResponse" ]( error || null )
			.catch( function( error ) {
				utils.die( "Server " + serverIndex + ": Failed to deliver update response: " +
				( "" + error ) + ", and result " + error.result );
			} );
	}

	device[ request.res.op ]().then( sendResponse, sendResponse );
} );

device.enablePresence().then(
	function() {
		device.register( {
			id: { path: "/a/" + uuid },
			deviceId: uuid,
			resourceTypes: [ "core.light" ],
			interfaces: [ "oic.if.baseline" ],
			discoverable: true,
			properties: { someValue: 0 }
		} ).then(
			function( resource ) {
				theResource = resource;
				utils.assert( "ok", true, "Server " + serverIndex +
					": device.register() successful" );

				// Signal to the test suite that we're ready for the client
				console.log( JSON.stringify( { ready: true } ) );
			},
			function( error ) {
				utils.assert( "ok", false,
					"Server " + serverIndex + ": device.register() failed with: " + error );
			} );
	}, function( error ) {
		utils.assert( "ok", false,
			"Server " + serverIndex + ": device.enablePresence() failed with: " + error );
	} );

// Cleanup on SIGINT
process.on( "SIGINT", function() {
	device.unregister( theResource ).then(
		function() {
			utils.assert( "ok", true, "Server " + serverIndex +
				": device.unregister() successful" );
			process.exit( 0 );
		},
		function( error ) {
			utils.assert( "ok", false,
				"Server " + serverIndex + ": device.unregister() failed with: " + error );
			process.exit( 0 );
		} );
} );

};

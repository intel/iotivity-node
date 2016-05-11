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
	totalRequests = 0,
	utils = require( "../../assert-to-console" ),
	uuid = process.argv[ 2 ],
	device = require( "../../../index" )( "server" );

console.log( JSON.stringify( { assertionCount: 8 } ) );

function resourceOnRequest( request ) {
	totalRequests++;
	utils.assert( "strictEqual", request.type, "retrieverequest",
		"Server: Request is of type retrieve" );
	if ( request.type === "retrieverequest" ) {
		if ( totalRequests === 1 ) {
			request.sendResponse( request.target ).then(
				function() {
					utils.assert( "ok", true,
						"Server: Successfully responded to retrieve request" );
					device.unregister( theResource ).then(
						function() {
							utils.assert( "ok", true,
								"Server: device.unregister() successful" );
							theResource = null;
						},
						function( error ) {
							utils.die(
								"Server: device.unregister() failed with: " + error +
								" and result " + error.result );
						} );
				},
				function( error ) {
					utils.die( "Server: Failed to send response with error " + error +
						" and result " + error.result );
				} );
		} else {
			request.sendError( new Error( "resource not found" ) ).then(
				function() {
					utils.assert( "ok", true,
						"Server: Successfully reported error to client" );
				},
				function( error ) {
					utils.die( "Server: Failed to report error to client: " + error +
						" and result " + error.result );
				} );
		}
	}
}

utils.assert( "ok", true, "Server: device.configure() successful" );

device.register( {
	id: { path: "/a/" + uuid },
	resourceTypes: [ "core.light" ],
	interfaces: [ "oic.if.baseline" ],
	discoverable: true,
	properties: {
		"How many angels can dance on the head of a pin?": "As many as wanting."
	}
} ).then(
	function( resource ) {
		theResource = resource;
		utils.assert( "ok", true, "Server: device.register() successful" );
		device.addEventListener( "retrieverequest", resourceOnRequest );

		// Signal to the test suite that we're ready for the client
		console.log( JSON.stringify( { ready: true } ) );
	},
	function( error ) {
		utils.die( "Server: device.register() failed with: " + error +
			" and result " + error.result );
	} );

// Cleanup on SIGINT
process.on( "SIGINT", function() {
	utils.assert( "strictEqual", totalRequests, 2,
		"Server: There have been exactly two requests" );
	process.exit( 0 );
} );

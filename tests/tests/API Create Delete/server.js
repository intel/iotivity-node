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

var utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )( "server" ),
	uuid = process.argv[ 2 ];

console.log( JSON.stringify( { assertionCount: 5 } ) );

device.register( {
	id: {
		path: "/a/" + uuid
	},
	resourceTypes: [ "core.light" ],
	interfaces: [ "oic.if.baseline" ],
	discoverable: true,
	properties: {
		someProperty: "someValue"
	}
} ).then( function( resource ) {
	var done,
		requestIndex = 0,
		requestHandler = function( request ) {
			switch ( requestIndex ) {

			// Both the initial "createrequest" event and the subsequent duplicate
			// "createrequest" event are handled by the same code. The expected behaviour is
			// asserted by the client.
			case 0:
			case 1:
				utils.assert( "strictEqual", request.type, "createrequest",
					"Server: " + ( requestIndex === 0 ? "First" : "Second" ) +
					" request is 'create'" );
				utils.assert( "deepEqual", request.res, {
					discoverable: true,
					id: {
						deviceId: device.device.uuid,
						path: "/a/new-resource"
					},
					resourceTypes: [ "core.light" ],
					interfaces: [ "oic.if.baseline" ],
					properties: {
						someKey: "someValue"
					}
				}, "Server: Resource signature is as expected" );
				device.register( request.res ).then(
					function( theResource ) {
						resource = theResource;
						request.sendResponse( null ).catch( done );
					},
					function( error ) {
						request.sendError( error ).catch( done );
					} );
				break;

			case 2:
				utils.assert( "strictEqual", request.type, "deleterequest",
					"Server: Third request is 'delete'" );
				device.unregister( resource ).then(
					function() {
						request.sendResponse( null ).then( done, done );
					},
					function( error ) {
						request.sendError( error ).then( done, done );
					} );
				break;

			default:
				done( new Error( "Unexpected request" ) );
				break;
			}
			requestIndex++;
		};
	done = function( error ) {
		device.removeEventListener( "createrequest", requestHandler );
		device.removeEventListener( "deleterequest", requestHandler );
		return Promise.reject( error );
	};
	device.addEventListener( "createrequest", requestHandler );
	device.addEventListener( "deleterequest", requestHandler );
	console.log( JSON.stringify( { ready: true } ) );
}, function( error ) {
	utils.die( "Server: Error: " + error.message + " and result " + error.result );
} );

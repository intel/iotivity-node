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
	_ = require( "lodash" ),
	async = require( "async" ),
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )(),
	uuid = process.argv[ 2 ];

console.log( JSON.stringify( { assertionCount: 2 } ) );

async.series( [
	function configureTheDevice( callback ) {
		device.configure( { role: "server" } ).then( callback, callback );
	},

	function registerTheResource( callback ) {
		device.registerResource( {
			id: { path: "/a/" + uuid },
			resourceTypes: [ "core.light" ],
			interfaces: [ "oic.if.baseline" ],
			discoverable: true,
			properties: { someValue: "stringular" }
		} ).then( function( resource ) {
			theResource = resource;
			callback();
		}, callback );
	},

	function handleRequests( callback ) {
		var requestCount = 0,
			requestSequence = [],
			teardown,
			requestHandler = function( request ) {
				requestSequence.push( requestCount );
				if ( requestCount === 0 ) {
					request.sendError( null ).then(
						function() {
							requestCount++;
						}, teardown );
				} else {
					request.sendError( _.extend( new Error( "Something went horribly wrong!" ), {
						excuses: [ "Tired", "Hungry", "Sleepy", "Sick" ]
					} ) ).then( function() {
						utils.assert( "deepEqual", requestSequence, [ 0, 1 ],
							"Requests arrived in the right sequence" );
						teardown();
					}, teardown );
				}
			};
		teardown = function( error ) {
			device.removeEventListener( "request", requestHandler );
			callback( error );
		};

		device.addEventListener( "request", requestHandler );

		console.log( JSON.stringify( { ready: true } ) );
	}
], function( error ) {
	if ( error ) {
		utils.die( error );
	}
} );

// Cleanup on SIGINT
process.on( "SIGINT", function() {
	device.unregisterResource( theResource ).then(
		function() {
			utils.assert( "ok", true, "Server: device.unregisterResource() successful" );
			process.exit( 0 );
		},
		function( error ) {
			utils.die( "Server: device.unregisterResource() failed with: " + error +
				" and result " + error.result );
			process.exit( 0 );
		} );
} );

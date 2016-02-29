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

console.log( JSON.stringify( { assertionCount: 5 } ) );

function discoverTheResource() {
	var eventHandler,
		removeListener = function() {
			if ( eventHandler ) {
				device.removeEventListener( "resourcefound", eventHandler );
			}
		};

	return Promise.all( [
		new Promise( function( fulfill ) {
			eventHandler = function( event ) {
				var index,
					count = 0,
					url = "/a/" + uuid;

				if ( event.resource.id.path === url ) {
					utils.assert( "ok", true, "Client: Resource found" );

					for ( index in device._resources ) {
						if ( device._resources[ index ].id.path === url ) {
							count++;
						}
					}

					utils.assert( "strictEqual", count, 1,
						"Client: Resource present exactly once among resources" );
					fulfill( event.resource );
				}
			};

			device.addEventListener( "resourcefound", eventHandler );
		} ),
		device.findResources().then(
			function() {
				utils.assert( "ok", true, "Client: device.findResources() successful" );
			} )
	] ).then( function( results ) {
		removeListener();
		return results[ 0 ];
	}, removeListener );
}

async.series( [

	function configureTheDevice( callback ) {
		device.configure( {
			role: "client"
		} ).then(
			function() {
				callback( null );
			},
			function( error ) {
				callback( _.extend( error, { step: "device.configure()" } ) );
			} );
	},

	function resourceDiscovery( callback ) {
		discoverTheResource().then(
			function( resource ) {
				theResource = resource;
				utils.assert( "strictEqual", !!device._handles[ resource.id.deviceId ], true,
					"Client: A presence handle is saved for the resource upon discovery" );
				callback( null );
			},
			function( error ) {
				callback( _.extend( error, { step: "first discovery" } ) );
			} );
	},

	function checkPresenceResponse( callback ) {
		var sequence = [],
			handler,
			cleanup = function( error ) {
				theResource.removeEventListener( "delete", handler );
				device.addEventListener( "resourcefound", handler );
				utils.assert( "deepEqual", sequence, [
					"delete",
					"resourcefound:" + theResource.id.deviceId + ":" + theResource.id.path
				], "Client: presence response sequence is as expected" );
				callback( error );
			};

		handler = function( event ) {
			if ( event.type === "resourcefound" ) {
				if ( event.resource.id.deviceId === theResource.id.deviceId &&
						event.resource.id.path === theResource.id.path ) {
					sequence.push( event.type + ":" + theResource.id.deviceId + ":" +
						theResource.id.path );
				}
			} else {
				sequence.push( event.type );
			}
			if ( sequence.length >= 2 ) {
				cleanup();
			}
		};

		theResource.addEventListener( "delete", handler );
		device.addEventListener( "resourcefound", handler );

		// Start the presence cycling
		theResource.properties.op = "cyclePresence";
		device.updateResource( theResource ).catch( cleanup );
	}

], function( error ) {
	if ( error ) {
		utils.die( "Client: " + error.step + " failed with " + error + " and result " +
			error.result );
	} else {
		console.log( JSON.stringify( { killPeer: true } ) );
		process.exit( 0 );
	}
} );

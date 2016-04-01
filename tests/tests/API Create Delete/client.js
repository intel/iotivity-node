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

var remoteResource,
	_ = require( "lodash" ),
	async = require( "async" ),
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )(),
	uuid = process.argv[ 2 ],
	newRemoteResourceTemplate = {
		id: {
			path: "/a/new-resource"

		// We will set deviceId once we know the id of the remote device
		},
		discoverable: true,
		properties: {
			someKey: "someValue"
		},
		resourceTypes: [ "core.light" ],
		interfaces: [ "oic.if.baseline" ]
	};

console.log( JSON.stringify( { assertionCount: 5 } ) );

function findResourceByUrl( url, deviceId ) {
	return Promise.all( [
		new Promise( function( fulfill ) {
			var resourceFound = function( event ) {
				if ( event.resource.id.path === url ) {
					device.removeEventListener( "resourcefound", resourceFound );
					fulfill( event.resource );
				}
			};
			device.addEventListener( "resourcefound", resourceFound );
		} ),
		device.findResources( _.extend( {}, deviceId ? { deviceId: deviceId } : {} ) )
	] ).then( function( results ) {
		return results[ 0 ];
	} );
}

async.series( [
	function initDevice( callback ) {
		device.configure( {
			role: "client"
		} ).then( callback, callback );
	},

	function findTestServer( callback ) {
		findResourceByUrl( "/a/" + uuid ).then(
			function( resource ) {
				remoteResource = resource;

				// We know the id of the test server, so set it on the resource template
				newRemoteResourceTemplate.id.deviceId = resource.id.deviceId;
				callback();
			}, callback );
	},

	function createRemoteResource( callback ) {
		device.createResource( newRemoteResourceTemplate ).then(
			function( resource ) {

				// Use the newly created remote resource instead of the initial resource that we
				// used for locating the test server.
				remoteResource = resource;
				utils.assert( "ok", true, "Client: Creating remote resource has succeeded" );
				callback();
			}, callback );
	},

	function findRemoteResource( callback ) {
		findResourceByUrl( "/a/new-resource", remoteResource.deviceId ).then(
			function( resource ) {
				utils.assert( "ok", true, "Client: Discovering remote resource has succeeded" );
				utils.assert( "deepEqual", resource.id, remoteResource.id,
					"Client: Discovered remote resource has the same id as the created resource" );
				callback();
			}, callback );
	},

	function createDuplicateRemoteResource( callback ) {
		device.createResource( newRemoteResourceTemplate ).then(
			function() {
				callback( new Error( "Server created duplicate resource" ) );
			}, function() {
				utils.assert( "ok", true,
					"Client: Creating a duplicate remote resource is not possible" );
				callback();
			} );
	},

	function deleteRemoteResource( callback ) {
		device.deleteResource( remoteResource.id ).then(
			function() {
				utils.assert( "ok", true, "Client: Deleting the remote resource has succeeded" );
				console.log( JSON.stringify( { killPeer: true } ) );
				process.exit( 0 );
			}, callback );
	}
], function( error ) {
	if ( error ) {
		utils.die( "Client: Error: " + error.message + " and result " + error.result );
	}
} );

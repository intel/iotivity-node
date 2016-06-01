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
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )( "client" ),
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

console.log( JSON.stringify( { assertionCount: 7 } ) );

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

device.create( { id: {} } )
	.then( function() {
		utils.assert( "ok", false, "Client: Creating malformed resource has succeeded" );
	}, function( error ) {
		utils.assert( "strictEqual", ( "" + error ),
			"Error: create: Invalid OicResourceInit",
			"Client: Creating malformed resource has errored locally with the correct message" );
		return findResourceByUrl( "/a/" + uuid );
	} )
	.then( function destinationDeviceFound( resource ) {
		newRemoteResourceTemplate.id.deviceId = resource.id.deviceId;
		return newRemoteResourceTemplate;
	} )
	.then( function createRemoteMalformedResource( template ) {
		return new Promise( function( fulfill, reject ) {
			var lowlevel = require( "bindings" )( "iotivity" );
			var receptacle = {};
			var result = lowlevel.OCDoResource( receptacle, lowlevel.OCMethod.OC_REST_POST,
				"/a/crazy", device._devices[ template.id.deviceId ].address, null,
				lowlevel.OCConnectivityType.CT_DEFAULT, lowlevel.OCQualityOfService.OC_HIGH_QOS,
				function( handle, response ) {
					utils.assert( "strictEqual", response.result,
						lowlevel.OCStackResult.OC_STACK_ERROR,
						"Client: Server sent OC_STACK_ERROR when creating invalid resource" );
					fulfill( template );
					return lowlevel.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
				}, null, 0 );

			if ( result !== lowlevel.OCStackResult.OC_STACK_OK ) {
				reject( new Error( "Client: Request to create malformed resource failed" ) );
			}
		} );
	} )
	.then( function createRemoteResource( template ) {
		return device.create( template );
	} )
	.then( function discoverNewlyCreatedResource( resource ) {
		utils.assert( "ok", true, "Client: Creating remote resource has succeeded" );
		remoteResource = resource;
		return findResourceByUrl( "/a/new-resource", resource.deviceId );
	} )
	.then( function attemptToCreateDuplicateResource( resource ) {
		utils.assert( "ok", true, "Client: Discovering remote resource has succeeded" );
		utils.assert( "deepEqual", resource.id, remoteResource.id,
			"Client: Discovered remote resource has the same id as the created resource" );
		return device.create( newRemoteResourceTemplate );
	} )
	.then( function creatingDuplicateResourceHasSucceeded() {
		return Promise.reject( new Error( "Server created duplicate resource" ) );
	},
	function deleteRemoteResource() {
		utils.assert( "ok", true, "Client: Creating a duplicate remote resource is not possible" );
		return device.delete( remoteResource.id );
	} )
	.then( function remoteResourceDeleted() {
		utils.assert( "ok", true, "Client: Deleting the remote resource has succeeded" );
		console.log( JSON.stringify( { killPeer: true } ) );
		process.exit( 0 );
	} )
	.catch( function catchErrors( error ) {
		utils.die( "Client: Error: " + error.message + " and result " + error.result );
	} );

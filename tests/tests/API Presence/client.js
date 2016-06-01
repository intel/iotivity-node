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
	device = require( "../../../index" )( "client" ),
	uuid = process.argv[ 2 ];

console.log( JSON.stringify( { assertionCount: 8 } ) );

// Tell the resource's server to perform an operation (either disablePresence or enablePresence)
// or, if op is undefined, then do not communicate with the server. Either way, wait five seconds
// for the ensuing events (or lack thereof).
function countEvents( resource, op ) {
	return new Promise( function( fulfill, reject ) {
		var events = [];

		function devicefound( event ) {
			if ( event.device.uuid === resource.id.deviceId ) {
				events.push( JSON.stringify( {
					event: event.type,
					source: { deviceId: event.device.uuid }
				} ) );
			}
		}

		function resourcefound( event ) {
			if ( event.resource.id.path === resource.id.path &&
				event.resource.id.deviceId === resource.id.deviceId ) {
				events.push( JSON.stringify( {
					event: event.type,
					source: event.resource.id
				} ) );
			}
		}

		function deleteListener( event ) {
			events.push( JSON.stringify( {
				event: event.type,
				source: resource.id
			} ) );
		}

		function devicechange( event ) {
			if ( event.device.uuid === resource.id.deviceId ) {
				events.push( JSON.stringify( {
					event: event.type,
					changeType: event.changeType,
					source: { deviceId: event.device.uuid }
				} ) );
			}
		}

		function waitForEvents() {

			// Collect events for 5 more seconds before fulfilling the promise
			setTimeout( function() {
				doListeners( "remove" );
				fulfill( { resource: resource, events: events.sort() } );
			}, 2000 );
		}

		function doListeners( listenerOp ) {
			if ( op === "enablePresence" ) {
				device[ listenerOp + "EventListener" ]( "devicefound", devicefound );
				device[ listenerOp + "EventListener" ]( "resourcefound", resourcefound );
			} else {
				resource[ listenerOp + "EventListener" ]( "delete", deleteListener );
			}
			device[ listenerOp + "EventListener" ]( "devicechange", devicechange );
		}

		doListeners( "add" );

		if ( op ) {

			// Issue an update() to trigger the desired presence op
			resource.properties.op = op;
			device.update( resource ).then( waitForEvents, reject );
		} else {
			waitForEvents();
		}
	} );
}

function discoverResources() {
	return new Promise( function( fulfill, reject ) {
		var resourcefound,
			resources = [],
			teardown = function( error ) {
				device.removeEventListener( "resourcefound", resourcefound );
				if ( error ) {
					reject( error );
				} else {

					// Sort the resources to make sure they always appear in the same order
					fulfill( resources.sort( function( leftResource, rightResource ) {
						return leftResource.id.deviceId.localeCompare( rightResource.id.deviceId );
					} ) );
				}
			};
			resourcefound = function resourcefound( event ) {
				if ( event.resource.id.path === "/a/" + uuid ) {
					resources.push( event.resource );
					if ( resources.length >= 2 ) {
						teardown();
					}
				}
			};

			device.addEventListener( "resourcefound", resourcefound );
			device.findResources( { resourcePath: "/a/" + uuid } ).catch( teardown );
	} );
}

function performStep( resourceOp, expectation, message ) {
	return function( resources ) {

		return Promise.all( [
			countEvents( resources[ 0 ], resourceOp ),
			countEvents( resources[ 1 ], resourceOp )
		] ).then( function( result ) {
			utils.assert( "deepEqual",
				[ result[ 0 ].events, result[ 1 ].events ], expectation( resources ), message );

			return discoverResources();
		} );
	};
}

function performSubscription( operation, resourceIndex ) {
	return function issueSubscribeOperation( resources ) {
		return Promise.all( [
			Promise.resolve( resources ),
			device[ operation ].apply( device,
				( resourceIndex === undefined ? [] : [ resources[ resourceIndex ].id.deviceId ] ) )
		] )
		.then( function passOnResources( result ) {
			return Promise.resolve( result[ 0 ] );
		} );
	};
}

discoverResources()
	.then( performStep( "disablePresence",
		function expectationForDisablePresenceWhileUnsubscribedFromAll() {
			return [ [], [] ];
		}, "Client: No events upon disablePresence() while not subscribed" ) )
	.then( performStep( "enablePresence",
		function expectationForEnablePresenceWhileUnsubscribedFromAll() {
			return [ [], [] ];
		}, "Client: No events upon enablePresence() while not subscribed" ) )
	.then( performSubscription( "subscribe" ) )
	.then( performStep( "disablePresence",
		function expectationForDisablePresenceWhileSubscribedToAll( resources ) {
			return [
				[
					JSON.stringify( { event: "devicechange", changeType: "changed",
						source: { deviceId: resources[ 0 ].id.deviceId }
					} ),
					JSON.stringify( { event: "devicechange", changeType: "deleted",
						source: { deviceId: resources[ 0 ].id.deviceId }
					} ),
					JSON.stringify( { event: "delete", source: resources[ 0 ].id } )
				].sort(),
				[
					JSON.stringify( { event: "devicechange", changeType: "changed",
						source: { deviceId: resources[ 1 ].id.deviceId }
					} ),
					JSON.stringify( { event: "devicechange", changeType: "deleted",
						source: { deviceId: resources[ 1 ].id.deviceId }
					} ),
					JSON.stringify( { event: "delete", source: resources[ 1 ].id } )
				].sort()
			];
		}, "Client: Events upon disablePresence() of both servers while subscribed are as " +
			"expected" ) )
	.then( performStep( "enablePresence",
		function expectationForEnablePresenceWhileSubscribedToAll( resources ) {
			return [
				[
					JSON.stringify( { event: "devicefound",
						source: { deviceId: resources[ 0 ].id.deviceId }
					} ),
					JSON.stringify( { event: "devicechange", changeType: "added",
						source: { deviceId: resources[ 0 ].id.deviceId }
					} ),
					JSON.stringify( { event: "resourcefound", source: resources[ 0 ].id } )
				].sort(),
				[
					JSON.stringify( { event: "devicefound",
						source: { deviceId: resources[ 1 ].id.deviceId }
					} ),
					JSON.stringify( { event: "devicechange", changeType: "added",
						source: { deviceId: resources[ 1 ].id.deviceId }
					} ),
					JSON.stringify( { event: "resourcefound", source: resources[ 1 ].id } )
				].sort()
			];
		}, "Client: Events upon enablePresence() of both servers while subscribed are as " +
			"expected" ) )
	.then( performSubscription( "unsubscribe", 0 ) )
	.then( performStep( "disablePresence",
		function expectationForDisablePresenceWithOneUnsubscribed( resources ) {
			return [
				[],
				[
					JSON.stringify( { event: "devicechange", changeType: "deleted",
						source: { deviceId: resources[ 1 ].id.deviceId }
					} ),
					JSON.stringify( { event: "delete", source: resources[ 1 ].id } )
				].sort()
			];
		}, "Client: Events upon disablePresence() with one server not subscribed are as " +
			"expected" ) )
	.then( performStep( "enablePresence",
		function expectationForEnablePresenceWithOneUnsubscribed( resources ) {
			return [
				[],
				[
					JSON.stringify( { event: "devicefound",
						source: { deviceId: resources[ 1 ].id.deviceId }
					} ),
					JSON.stringify( { event: "devicechange", changeType: "added",
						source: { deviceId: resources[ 1 ].id.deviceId }
					} ),
					JSON.stringify( { event: "resourcefound", source: resources[ 1 ].id } )
				].sort()
			];
		}, "Client: Events upon enablePresence() with one server not subscribed are as " +
			"expected" ) )
	.then(
		function() {
			console.log( JSON.stringify( { killPeer: true } ) );
			process.exit( 0 );
		},
		function( error ) {
		utils.die( ( "" + error ) );
		} );

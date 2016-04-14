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

var lightResource,
	onRequestCalls = 0,
	uuid = process.argv[ 2 ],
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )( "server" );

console.log( JSON.stringify( { assertionCount: 5 } ) );

function cleanup() {
	utils.assert( "ok", onRequestCalls > 0, "Server: device.onrequest has been called" );
	utils.assert( "strictEqual", lightResource.properties.increment, 11,
		"Server: Exactly 11 notifications were delivered (the last one with noObservers)" );
	console.log( JSON.stringify( { killPeer: true } ) );
	process.exit( 0 );
}

function lightResourceOnRequest( request ) {
	if ( request.type === "retrieve" || request.type === "observe" ) {
		request.sendResponse( null );
	}
}

function testOnRequest() {
	onRequestCalls++;
}

// After we've sent out ten notifications, we wait for the client to remove itself from the list of
// observers, and then we send out a final notification which we expect to fail in a certain way.
function assertNotifyFailure() {
	var waitForObserverListToEmpty = setInterval( function() {
		if ( !lightResource._observationHandle ) {
			clearInterval( waitForObserverListToEmpty );
			lightResource.properties.increment++;
			device.notify( lightResource ).catch(
				function( error ) {
					utils.assert( "strictEqual", error.noObservers, true,
						"Server: device.notify() fails with noObservers when there are " +
						"no observers" );
					cleanup();
				} );
		}
	}, 500 );
}

utils.assert( "ok", true, "device configured successfully" );

device.registerResource( {
	id: { path: "/a/" + uuid },
	deviceId: uuid,
	resourceTypes: [ "core.light" ],
	interfaces: [ "oic.if.baseline" ],
	discoverable: true,
	observable: true,
	properties: { increment: 0 }
} ).then(
	function( resource ) {
		utils.assert( "ok", true, "registerResource() successful" );
		lightResource = resource;
		device.addEventListener( "retrieverequest", lightResourceOnRequest );
		device.addEventListener( "observerequest", lightResourceOnRequest );
		device.onobserverequest = testOnRequest;
		console.log( JSON.stringify( { ready: true } ) );
	},
	function( error ) {
		utils.die( "registerResource() failed with " + error +
			" and result " + error.result );
	} );

// Notify ten times and then stop this interval
var notifyIntervalId = setInterval( function() {
	var haveObservers = false;

	if ( !lightResource ) {
		return;
	}

	device.notify( lightResource ).then(
		function() {

			haveObservers = true;
			lightResource.properties.increment++;
			if ( lightResource.properties.increment === 10 ) {

				// After ten notifications we wait for the list of observers to go empty so we can
				// assert that notify() fails with noObservers: true
				clearInterval( notifyIntervalId );
				assertNotifyFailure();
			}
		},
		function( error ) {
			if ( haveObservers ) {
				utils.die( "notify() failed with " + error + " and result " + error.result );
			}
		} );
}, 1000 );

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

var resourceFound = false,
	uuid = process.argv[ 2 ],
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )();

console.log( JSON.stringify( { assertionCount: 9 } ) );

function discoverResources() {
	device.addEventListener( "resourcefound", function( event ) {
		var observationHandle,
			callCount = 0,
			legacyCallCount = 0,
			dummyHandler = function() {},
			legacyResourceUpdate = function() {
				legacyCallCount++;
			},
			resourceUpdate = function( event ) {
				if ( event.resource.properties.increment >= 9 ) {
					event.resource.onupdate = null;
					utils.assert( "deepEqual", event.resource._observationHandle,
						observationHandle,
						"Client: Private resource observation handle is unchanged after " +
						"removing handler via legacy interface" );
					event.resource.removeEventListener( "update", dummyHandler );
					utils.assert( "deepEqual", event.resource._observationHandle,
						observationHandle,
						"Client: Private resource observation handle is unchanged after " +
						"removing dummy handler" );
					event.resource.removeEventListener( "update", resourceUpdate );
					utils.assert( "ok", !event.resource._observationHandle,
						"Client: Private resource observation handle is absent after " +
						"removing the last handler" );
					utils.assert( "strictEqual", callCount, legacyCallCount,
						"Client: Handler called the same number of times as handler attached " +
						"via legacy interface" );
				}
				callCount++;
			};

		if ( !resourceFound && event.resource.id.path === "/a/" + uuid ) {
			resourceFound = true;
			utils.assert( "ok", !event.resource._observationHandle,
				"Client: Private resource observation handle is initially absent" );
			event.resource.addEventListener( "update", resourceUpdate );
			observationHandle = event.resource._observationHandle;
			event.resource.addEventListener( "update", dummyHandler );
			utils.assert( "deepEqual", event.resource._observationHandle, observationHandle,
				"Client: Private resource observation handle is unchanged after adding a second " +
				"handler" );
			event.resource.onupdate = legacyResourceUpdate;
			utils.assert( "deepEqual", event.resource._observationHandle, observationHandle,
				"Client: Private resource observation handle is unchanged after adding a " +
				"handler via the legacy interface" );
		}
	} );
	device.findResources().then(
		function() {
			utils.assert( "ok", true, "Client: findResources() successful" );
		},
		function( error ) {
			utils.die( "Client: findResources() failed with " + error +
				" and result " + error.result );
		} );
}

device.configure( {
	role: "client"
} ).then(
	function() {
		utils.assert( "ok", true, "Client: device.configure() successful" );

		discoverResources();
	},
	function( error ) {
		utils.assert( "ok", false, "Client: device.configure() failed with " + error );
	} );


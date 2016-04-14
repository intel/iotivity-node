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

var testUtils = require( "../assert-to-console" );

console.log( JSON.stringify( { assertionCount: 1 } ) );

var device = require( "../../index" )( "server" );

if ( device.device.uuid ) {
	var deviceId = device.device.uuid;

	device.registerResource( {
		id: { path: "/light/ambience/blue" },
		deviceId: deviceId,
		resourceTypes: [ "core.light" ],
		interfaces: [ "oic.if.baseline" ],
		discoverable: true,
		observable: true,
		properties: { color: "light-blue", dimmer: 0.2 }
	} ).then(
		function( resource ) {
			device.unregisterResource( resource ).then(
				function() {
					testUtils.assert( "ok", true, "Resource unregistered successfully" );
					process.exit( 0 );
				},
				function( error ) {
					testUtils.assert( "ok", false,
						"Resource cannot be unregistered, error with code: " + error.result );
					process.exit( 0 );
				} );
		},
		function( error ) {
			testUtils.assert( "ok", false,
				"Resource not registered while testing unregister, error with code: " +
				error.result );
			process.exit( 0 );
		} );
}

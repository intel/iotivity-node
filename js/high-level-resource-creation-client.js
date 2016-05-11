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

console.log( "Acquiring OCF device" );

var device = require( "iotivity-node" )( "client" );

function throwError( error ) {
	console.error( error.stack ? error.stack : ( error.message ? error.message : error ) );
	process.exit( 1 );
}

new Promise( function( fulfill, reject ) {
	var resourceFoundHandler = function( event ) {
		console.log( "Discovered resource(s) via the following event:\n" +
			JSON.stringify( event, null, 4 ) );

		// We've discovered the resource we were seeking.
		if ( event.resource.id.path === "/a/high-level-resource-creation-example" ) {
			console.log( "Found the test server" );
			device.removeEventListener( "resourcefound", resourceFoundHandler );
			fulfill( event.resource.id.deviceId );
		}
	};

	// Add a listener that will receive the results of the discovery
	device.addEventListener( "resourcefound", resourceFoundHandler );

	console.log( "Issuing discovery request" );
	device.findResources().catch( function( error ) {
		device.removeEventListener( "resourcefound", resourceFoundHandler );
		reject( "findResource() failed: " + error );
	} );
} ).then( function( deviceId ) {
	console.log( "deviceId discovered" );
	return device.create( {
		id: {
			deviceId: deviceId,
			path: "/a/new-resource"
		},
		resourceTypes: [ "core.light" ],
		interfaces: [ "oic.if.baseline" ],
		properties: {
			exampleProperty: 23
		}
	} );
} ).then( function( resource ) {
	console.log( "remote resource successfully created with id " +
		JSON.stringify( resource.id ) );
	return device.delete( resource.id );
} ).then( function() {
	console.log( "remote resource successfully deleted" );
} ).catch( throwError );

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

var observationCount = 0;

console.log( "Acquiring OCF device" );

var client = require( "iotivity-node" ).client;

// Add a listener that will receive the results of the discovery
client.on( "resourcefound", function( resource ) {
	console.log( "Discovered resource:\n" +
		JSON.stringify( resource, null, 4 ) );

	// We've discovered the resource we were seeking.
	if ( resource.resourcePath === "/a/high-level-example" ) {
		var resourceUpdate = function( resource ) {
			console.log( "Received resource update:\n" +
				JSON.stringify( resource, null, 4 ) );

			// Stop observing after having made 10 observations
			if ( ++observationCount >= 10 ) {
				resource.removeListener( "update", resourceUpdate );
			}
		};

		console.log( "This is the resource we want to observe" );

		// Let's start observing the resource.
		resource.on( "update", resourceUpdate );
	}
} );

console.log( "Issuing discovery request" );
client.findResources().catch( function( error ) {
	console.error( error.stack ? error.stack : ( error.message ? error.message : error ) );
	process.exit( 1 );
} );

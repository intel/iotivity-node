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

var observationCount = 0,
	device = require( "iotivity-node" )();

device.configure( {
	role: "client"
} ).then( function() {

	// Add a listener that will receive the results of the discovery
	device.addEventListener( "resourcefound", function( event ) {
		console.log( "Discovered resource(s) via the following event:\n" +
			JSON.stringify( event, null, 4 ) );

		// We've discovered the resource we were seeking.
		if ( event.resource.id.path === "/a/high-level-example" ) {
			var resourceUpdate = function( event ) {
				console.log( "Received resource update event:\n" +
					JSON.stringify( event, null, 4 ) );

				// Stop observing after having made 10 observations
				if ( ++observationCount >= 10 ) {
					event.resource.removeEventListener( "update", resourceUpdate );
				}
			};

			console.log( "This is the resource we want to observe" );

			// Let's start observing the resource.
			event.resource.addEventListener( "update", resourceUpdate );
		}
	} );
	device.findResources();
} );

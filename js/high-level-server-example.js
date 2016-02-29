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
	haveObservers = false,
	sensor = require( "./mock-sensor" )(),
	device = require( "iotivity-node" )(),
	settings = {
		role: "server",
		info: {
			name: "api-server-example",
			manufacturerName: "Intel",
			manufactureDate: "Wed Sep 23 10:04:17 EEST 2015",
			platformVersion: "1.0.1",
			firmwareVersion: "0.0.1",
			supportUrl: "http://example.com/"
		}
	};

device.configure( settings ).then( function() {

	sensor.on( "change", function( newData ) {
		var index;

		if ( !lightResource ) {
			return;
		}

		// Grab the updated data from the sensor and store it in the properties of the resource
		for ( index in newData ) {
			lightResource.properties[ index ] = newData[ index ];
		}

		if ( haveObservers ) {
			device.notify( lightResource ).catch( function( error )  {
				if ( error.message === "notify: There are no observers" ) {
					haveObservers = false;
				}
			} );
		}
	} );

	function lightResourceOnRequest( request ) {
		function handleError( theError ) {
			console.error( theError );
			throw theError;
		}
		console.error( "received request: " + JSON.stringify( request, null, 4 ) );
		if ( request.type === "observe" ) {
			request.sendResponse( null ).then( function() {
				haveObservers = true;
			}, handleError );
		} else if ( request.type === "retrieve" ) {
			request.sendResponse( lightResource ).catch( handleError );
		} else {
			request.sendError( null ).catch( handleError );
		}
	}

	if ( device.settings.info.uuid ) {
		device.registerResource( {
			id: { path: "/a/high-level-example" },
			resourceTypes: [ "core.light" ],
			interfaces: [ "oic.if.baseline" ],
			discoverable: true,
			observable: true,
			properties: { someValue: 0, someOtherValue: "Helsinki" }
		} ).then(
			function( resource ) {
				lightResource = resource;
				device.addEventListener( "request", lightResourceOnRequest );
			},
			function( error ) {
				throw error;
			} );
	}
} ).catch( function( theError ) {
	console.error( theError );
	throw theError;
} );

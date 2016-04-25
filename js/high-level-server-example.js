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
	_ = require( "lodash" ),
	observerCount = 0,
	sensor = require( "./mock-sensor" )(),
	device = require( "iotivity-node" )( "server" );

device.device = _.extend( device.device, {
	name: "api-server-example"
} );
device.platform = _.extend( device.platform, {
	manufacturerName: "Intel",
	manufactureDate: new Date( "Wed Sep 23 10:04:17 EEST 2015" ),
	platformVersion: "1.0.1",
	firmwareVersion: "0.0.1",
	supportUrl: "http://example.com/"
} );

sensor.on( "change", function( newData ) {
	var index;

	if ( !lightResource ) {
		return;
	}

	// Grab the updated data from the sensor and store it in the properties of the resource
	for ( index in newData ) {
		lightResource.properties[ index ] = newData[ index ];
	}

	if ( observerCount > 0 ) {
		device.notify( lightResource ).catch( function( error )  {
			if ( error.message === "notify: There are no observers" ) {
				observerCount = 0;
			}
		} );
	}
} );

function handleError( theError ) {
	console.error( theError );
	process.exit( 1 );
}

var lightResourceRequestHandlers = {
	observe: function( request ) {
		request.sendResponse( null ).then( function() {
			observerCount++;
		}, handleError );
	},
	unobserve: function( request ) {
		request.sendResponse( null ).then( function() {
			observerCount--;
		}, handleError );
	},
	retrieve: function( request ) {
		request.sendResponse( lightResource ).catch( handleError );
	}
};

if ( device.device.uuid ) {
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

			// Add event handlers for each supported request type
			_.each( lightResourceRequestHandlers, function( callback, requestType ) {
				device.addEventListener( requestType + "request", function( request ) {
					console.log( "Received request " + JSON.stringify( request, null, 4 ) );
					callback( request );
				} );
			} );
		},
		function( error ) {
			throw error;
		} );
}

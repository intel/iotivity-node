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

import * as _ from "lodash"
import * as device from "iotivity-node";

var lightResource: device.ServerResource,
	observerCount = 0,
	sensor = require( "./mock-sensor" )();


_.extend( device.device, {
	coreSpecVersion: "res.1.0.0",
	dataModels: [ "something.1.0.0" ],
	name: "api-server-example"
} );
_.extend( device.platform, {
	manufacturerName: "Intel",
	manufactureDate: new Date( "Wed Sep 23 10:04:17 EEST 2015" ),
	platformVersion: "1.1.1",
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
		console.log( "Issuing notification" );
		lightResource.notify().catch( function( error ) {
			console.log( "Error while notifying: " + error.message + "\n" +
				JSON.stringify( error, null, 4 ) );
		} );
	}
} );

function handleError( theError ) {
	console.error( theError );
	process.exit( 1 );
}

var lightResourceRequestHandlers = {
	retrieve: function( request ) {
		request.respond( request.target ).catch( handleError );
		observerCount += ( "observe" in request ) ? ( request.observe ? 1 : -1 ) : 0;
	} as device.RequestHandler
};

if ( device.device.uuid ) {

	console.log( "Registering OCF resource" );

	device.server.register( {
		resourcePath: "/a/high-level-example",
		resourceTypes: [ "core.light" ],
		interfaces: [ "oic.if.baseline" ],
		discoverable: true,
		observable: true,
		properties: { someValue: 0, someOtherValue: "Helsinki" }
	} ).then(
		function( resource ) {
			console.log( "OCF resource successfully registered" );
			lightResource = resource;

			// Add event handlers for each supported request type
			_.each( lightResourceRequestHandlers, function( callback, requestType ) {
				resource[ "on" + requestType ]( function( request ) {
					console.log( "Received request " + JSON.stringify( request, null, 4 ) );
					callback( request );
				} );
			} );
		},
		function( error ) {
			throw error;
		} );
}

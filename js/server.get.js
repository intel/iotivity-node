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

var intervalId,
	handleReceptacle = {},

	// This is the same value as server.get.js
	sampleUri = "/a/iotivity-node-get-sample",
	iotivity = require( "iotivity-node/lowlevel" );

// Start iotivity and set up the processing loop
iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );

iotivity.OCSetDeviceInfo( { deviceName: "server.get" } );
iotivity.OCSetPlatformInfo( {
	platformID: "server.get.sample",
	manufacturerName: "iotivity-node"
} );

intervalId = setInterval( function() {
	iotivity.OCProcess();
}, 1000 );

// Create a new resource
iotivity.OCCreateResource(

	// The bindings fill in this object
	handleReceptacle,

	"core.fan",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	sampleUri,
	function( flag, request ) {
		console.log( "Entity handler called with flag = " + flag + " and the following request:" );
		console.log( JSON.stringify( request, null, 4 ) );

		// If we find the magic question in the request, we return the magic answer
		if ( request && request.payload && request.payload.values &&
				request.payload.values.question ===
				"How many angels can dance on the head of a pin?" ) {

			iotivity.OCDoResponse( {
				requestHandle: request.requestHandle,
				resourceHandle: request.resource,
				ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
				payload: {
					type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
					values: {
						"answer": "As many as wanting."
					}
				},
				resourceUri: sampleUri,
				sendVendorSpecificHeaderOptions: []
			} );

			return iotivity.OCEntityHandlerResult.OC_EH_OK;
		}

		// By default we error out
		return iotivity.OCEntityHandlerResult.OC_EH_ERROR;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE );

// Exit gracefully when interrupted
process.on( "SIGINT", function() {
	console.log( "SIGINT: Quitting..." );

	// Tear down the processing loop and stop iotivity
	clearInterval( intervalId );
	iotivity.OCDeleteResource( handleReceptacle.handle );
	iotivity.OCStop();

	// Exit
	process.exit( 0 );
} );

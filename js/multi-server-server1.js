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
	sampleUri = "/a/iotivity-multi-server-server1",
	iotivity = require( "iotivity-node/lowlevel" );

console.log( "Starting OCF stack in server mode" );

// Start iotivity and set up the processing loop
iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );

iotivity.OCSetPropertyValue( iotivity.OCPayloadType.PAYLOAD_TYPE_DEVICE,
	iotivity.OC_RSRVD_SPEC_VERSION, "res.1.1.0" );
iotivity.OCSetPropertyValue( iotivity.OCPayloadType.PAYLOAD_TYPE_DEVICE,
	iotivity.OC_RSRVD_DATA_MODEL_VERSION, "test.0.0.1" );
iotivity.OCSetPropertyValue( iotivity.OCPayloadType.PAYLOAD_TYPE_DEVICE,
	iotivity.OC_RSRVD_DEVICE_NAME, "server.multi-server1" );

iotivity.OCSetPropertyValue( iotivity.OCPayloadType.PAYLOAD_TYPE_PLATFORM,
	iotivity.OC_RSRVD_MFG_NAME, "iotivity-node" );

intervalId = setInterval( function() {
	iotivity.OCProcess();
}, 1000 );

console.log( "Registering resource" );

iotivity.OCCreateResource(
	handleReceptacle,
	"core.fan",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	sampleUri,
	function( flag, request ) {
		if ( request && request.method === iotivity.OCMethod.OC_REST_GET ) {
			iotivity.OCDoResponse( {
				requestHandle: request.requestHandle,
				resourceHandle: request.resource,
				ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
				payload: {
					type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
					values: {
						answer: "Multi-server sample server 1"
					}
				},
				resourceUri: sampleUri,
				sendVendorSpecificHeaderOptions: []
			} );
		}
		return iotivity.OCEntityHandlerResult.OC_EH_OK;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE );

console.log( "Server ready" );

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

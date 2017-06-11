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
	sampleUri = "/a/iotivity-node-delete-sample",
	iotivity = require( "iotivity-node/lowlevel" );

console.log( "Starting OCF stack in server mode" );

// Start iotivity and set up the processing loop
iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );

iotivity.OCSetPropertyValue( iotivity.OCPayloadType.PAYLOAD_TYPE_DEVICE,
	iotivity.OC_RSRVD_SPEC_VERSION, "res.1.1.0" );
iotivity.OCSetPropertyValue( iotivity.OCPayloadType.PAYLOAD_TYPE_DEVICE,
	iotivity.OC_RSRVD_DATA_MODEL_VERSION, "test.0.0.1" );
iotivity.OCSetPropertyValue( iotivity.OCPayloadType.PAYLOAD_TYPE_DEVICE,
	iotivity.OC_RSRVD_DEVICE_NAME, "server.delete" );

iotivity.OCSetPropertyValue( iotivity.OCPayloadType.PAYLOAD_TYPE_PLATFORM,
	iotivity.OC_RSRVD_MFG_NAME, "iotivity-node" );

intervalId = setInterval( function() {
	iotivity.OCProcess();
}, 1000 );

console.log( "Registering resource" );

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
		if ( request && request.method === iotivity.OCMethod.OC_REST_DELETE ) {

			var result = iotivity.OCDeleteResource( handleReceptacle.handle );

			console.log( "OCDeleteResource() has resulted in " + result );

			if ( result === iotivity.OCStackResult.OC_STACK_OK ) {
				handleReceptacle = null;
			}

			iotivity.OCDoResponse( {
				requestHandle: request.requestHandle,
				resourceHandle: null,
				ehResult: result ?
					iotivity.OCEntityHandlerResult.OC_EH_ERROR :
					iotivity.OCEntityHandlerResult.OC_EH_RESOURCE_DELETED,
				payload: null,
				resourceUri: sampleUri,
				sendVendorSpecificHeaderOptions: []
			} );

			return iotivity.OCEntityHandlerResult.OC_EH_OK;
		}

		// By default we error out
		return iotivity.OCEntityHandlerResult.OC_EH_ERROR;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE );

console.log( "Server ready" );

// Exit gracefully when interrupted
process.on( "SIGINT", function() {
	console.log( "SIGINT: Quitting..." );

	// Tear down the processing loop and stop iotivity
	clearInterval( intervalId );
	if ( handleReceptacle ) {
		iotivity.OCDeleteResource( handleReceptacle.handle );
	}
	iotivity.OCStop();

	// Exit
	process.exit( 0 );
} );

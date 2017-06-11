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
	iotivity = require( "iotivity-node/lowlevel" );

console.log( "Starting OCF stack in server mode" );

// Start iotivity and set up the processing loop
iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );

intervalId = setInterval( function() {
	iotivity.OCProcess();
}, 1000 );

iotivity.OCSetPropertyValue( iotivity.OCPayloadType.PAYLOAD_TYPE_DEVICE,
	iotivity.OC_RSRVD_SPEC_VERSION, "res.1.1.0" );
iotivity.OCSetPropertyValue( iotivity.OCPayloadType.PAYLOAD_TYPE_DEVICE,
	iotivity.OC_RSRVD_DATA_MODEL_VERSION, "test.0.0.1" );
iotivity.OCSetPropertyValue( iotivity.OCPayloadType.PAYLOAD_TYPE_DEVICE,
	iotivity.OC_RSRVD_DEVICE_NAME, "server.discoverable" );

iotivity.OCSetPropertyValue( iotivity.OCPayloadType.PAYLOAD_TYPE_PLATFORM,
	iotivity.OC_RSRVD_MFG_NAME, "iotivity-node" );

console.log( "Registering resources" );

// Create a new resource
iotivity.OCCreateResource(

	// The bindings fill in this object
	handleReceptacle,

	"core.fan",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	"/a/fan",
	function( flag, request ) {
		console.log( "Entity handler called with flag = " + flag + " and the following request:" );
		console.log( JSON.stringify( request, null, 4 ) );
		return iotivity.OCEntityHandlerResult.OC_EH_OK;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE );

// Create a new resource
iotivity.OCCreateResource(

	// The bindings fill in this object
	handleReceptacle,

	"core.light",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	"/a/light",
	function( flag, request ) {
		console.log( "Entity handler called with flag = " + flag + " and the following request:" );
		console.log( JSON.stringify( request, null, 4 ) );
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

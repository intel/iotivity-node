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

	// This is the same value as client.presence.js
	sampleUri = "/a/iotivity-node-presence-sample",
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
	iotivity.OC_RSRVD_DEVICE_NAME, "server.presence" );

iotivity.OCSetPropertyValue( iotivity.OCPayloadType.PAYLOAD_TYPE_PLATFORM,
	iotivity.OC_RSRVD_MFG_NAME, "iotivity-node" );

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
		return iotivity.OCEntityHandlerResult.OC_EH_OK;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE );

// Read keystrokes from stdin
var stdin = process.stdin;

stdin.setRawMode( true );
stdin.resume();
stdin.setEncoding( "utf8" );
stdin.on( "data", function( key ) {
	var result;

	switch ( key ) {

	case "p":
		result = iotivity.OCStartPresence( 0 );
		console.log( "OCStartPresence: " + result );
		break;

	case "s":
		result = iotivity.OCStopPresence();
		console.log( "OCStopPresence: " + result );
		break;

	// ^C
	case "\u0003":

		// Tear down the processing loop and stop iotivity
		clearInterval( intervalId );
		iotivity.OCDeleteResource( handleReceptacle.handle );
		iotivity.OCStopPresence();
		iotivity.OCStop();

		// Exit
		process.exit( 0 );
		break;

	default:
		return;
	}

	console.log( "Press 'p' to turn on presence\nPress 's' to turn off presence\n" +
		"Press Ctrl+C to exit" );
} );

console.log( "Press 'p' to turn on presence\nPress 's' to turn off presence\n" +
	"Press Ctrl+C to exit" );

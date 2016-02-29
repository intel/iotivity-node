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

var result,
	requestSequence = 0,
	uuid = process.argv[ 2 ],
	processCallCount = 0,
	processLoop = null,
	resourceHandleReceptacle = {},
	iotivity = require( "../../../lowlevel" ),
	testUtils = require( "../../utils" )( iotivity );

console.log( JSON.stringify( { assertionCount: 13 } ) );

// Initialize
result = iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );
testUtils.stackOKOrDie( "Server", "OCInit", result );

// Set up process loop
processLoop = setInterval( function() {
	var processResult = iotivity.OCProcess();

	if ( processResult === iotivity.OCStackResult.OC_STACK_OK ) {
		processCallCount++;
	} else {
		testUtils.stackOKOrDie(
			"Server",
			"OCProcess(after " + processCallCount + " successful calls)",
			processResult );
	}
}, 100 );

// Create resource
result = iotivity.OCCreateResource(
	resourceHandleReceptacle,
	"core.fan",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	"/a/" + uuid,
	function( flag, request ) {
		var responseResult, presenceResult,
			returnValue = iotivity.OCEntityHandlerResult.OC_EH_ERROR;

		// We expect exactly two PUT requests: The first one to turn on presence and the second one
		// to turn off presence. Anything more and we panic.
		if ( requestSequence < 2 ) {

			// First, we expect a PUT request to turn on presence
			testUtils.assert( "strictEqual",
				testUtils.lookupEnumValueName( "OCMethod", request.method ),
				"OC_REST_PUT",
				"Server: Request " + requestSequence + " is PUT" );
			testUtils.assert( "deepEqual",
				request.payload,
				{
					type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
					values: { presence: requestSequence === 0 ? true : false }
				},
				"Server: PUT request " + requestSequence + " has expected payload" );

			returnValue = iotivity.OCEntityHandlerResult.OC_EH_OK;

			if ( requestSequence === 0 ) {
				presenceResult = iotivity.OCStartPresence( 0 );
			} else {
				presenceResult = iotivity.OCStopPresence();
			}
			testUtils.stackOKOrDie(
				"Server",
				requestSequence === 0 ? "OCStartPresence" : "OCStopPresence",
				presenceResult );

			responseResult = iotivity.OCDoResponse( {
				requestHandle: request.requestHandle,
				resourceHandle: request.resource,
				ehResult: returnValue,
				payload: {
					type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
					values: { result: "ok" }
				},
				sendVendorSpecificHeaderOptions: [],
				resourceUri: "/a/" + uuid
			} );
			testUtils.stackOKOrDie(
				"Server",
				"OCDoResponse(to PUT request " + requestSequence + ")",
				responseResult );

			requestSequence++;
		} else {
			testUtils.die( "Server: Too many PUT requests" );
		}

		return returnValue;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE );
testUtils.stackOKOrDie( "Server", "OCCreateResource", result );

// Report that the server has successfully created its resource(s).
console.log( JSON.stringify( { ready: true } ) );

function cleanup() {
	var cleanupResult;

	if ( processLoop ) {
		clearInterval( processLoop );
		processLoop = null;
	}

	testUtils.assert( "ok", true, "Server: OCProcess succeeded " + processCallCount + " times" );

	cleanupResult = iotivity.OCDeleteResource( resourceHandleReceptacle.handle );
	testUtils.stackOKOrDie( "Server", "OCDeleteResource", result );

	cleanupResult = iotivity.OCStop();
	if ( testUtils.stackOKOrDie( "Server", "OCStop", result ) ) {
		process.exit( 0 );
	}
}

// Exit gracefully when interrupted
process.on( "SIGINT", cleanup );

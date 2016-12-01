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
	uuid = process.argv[ 2 ],
	processCallCount = 0,
	processLoop = null,
	resourceHandleReceptacle = {},
	iotivity = require( process.argv[ 3 ] + "/lowlevel" ),
	testUtils = require( "../../utils" )( iotivity );

console.log( JSON.stringify( { assertionCount: 10 } ) );

// Initialize
result = iotivity.OCRegisterPersistentStorageHandler( require( "../../../lib/StorageHandler" )() );
testUtils.stackOKOrDie( "Server", "OCRegisterPersistentStorageHandler", result );

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
		var responseResult,
			returnValue = iotivity.OCEntityHandlerResult.OC_EH_ERROR;

		testUtils.assert( "ok",
			request.resource === resourceHandleReceptacle.handle,
			"The resource handle in the entity handler request is the same JS object as the " +
			"resource handle returned at creation" );

		testUtils.assert( "deepEqual",
			testUtils.lookupBitfieldValueNames( "OCEntityHandlerFlag", flag ),
			{ OC_REQUEST_FLAG: true },
			"Server: Incoming request has the correct flags" );

		if ( request &&
				request.payload &&
				request.payload.type === iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION &&
				request.payload.values &&
				request.payload.values.question ) {
			testUtils.assert( "strictEqual", request.payload.values.question,
				"How many angels can dance on the head of a pin?",
				"Server: Correct request received" );

			responseResult = iotivity.OCDoResponse( {
				requestHandle: request.requestHandle,
				resourceHandle: request.resource,
				ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
				payload: {
					type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
					values: {
						answer: "As many as wanting."
					}
				},
				resourceUri: "/a/" + uuid,
				sendVendorSpecificHeaderOptions: []
			} );
			testUtils.stackOKOrDie( "Server", "OCDoResponse", responseResult );

			returnValue = iotivity.OCEntityHandlerResult.OC_EH_OK;
		} else {
			testUtils.die( "Server: Unexpected request:\n***\n" +
				JSON.stringify( request, null, 4 ) + "\n***" );
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
	testUtils.stackOKOrDie( "Server", "OCDeleteResource", cleanupResult );

	cleanupResult = iotivity.OCStop();
	if ( testUtils.stackOKOrDie( "Server", "OCStop", cleanupResult ) ) {
		process.exit( 0 );
	}
}

// Exit gracefully when interrupted
process.on( "message", cleanup );

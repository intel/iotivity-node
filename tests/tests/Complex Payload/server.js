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

require( "../../preamble" )( process.argv[ 2 ] );

var result,
	uuid = process.argv[ 2 ],
	processCallCount = 0,
	processLoop = null,
	resourceHandleReceptacle = {},
	iotivity = require( "../../../lowlevel" ),
	testUtils = require( "../../utils" )( iotivity );

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
		console.log( JSON.stringify( { killPeer: true } ) );
		process.exit( 0 );
	}
}

console.log( JSON.stringify( { assertionCount: 9 } ) );

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

var requestSequenceNumber = 0,
	requestSequence = [
		function( flag, request ) {
			if ( request.method !== iotivity.OCMethod.OC_REST_GET ) {
				testUtils.die( "Server: First method was not GET" );
			}
			result = iotivity.OCDoResponse( {
				requestHandle: request.requestHandle,
				resourceHandle: request.resource,
				resourceUri: "/a/" + uuid,
				ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
				payload: {
					type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
					values: {
						primitiveValue: 42,
						objectValue: {
							type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
							values: {
								childValue: 91
							}
						},
						arrayValue: [ 19, 23, 7 ]
					}
				},
				sendVendorSpecificHeaderOptions: []
			} );

			testUtils.stackOKOrDie( "Server", "OCDoResponse(get)", result );

			return iotivity.OCEntityHandlerResult.OC_EH_OK;
		},
		function( flag, request ) {
			if ( request.method !== iotivity.OCMethod.OC_REST_PUT ) {
				testUtils.die( "Server: Second method was not PUT" );
			}
			testUtils.assert( "deepEqual", request.payload, {
				type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
				values: {
					putValue: "A string",
					anotherPutValue: 23.7,
					childValues: {
						type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
						values: {
							putChildValue: false,
							putChildArray: [ [ 2, 3, 5 ], [ 9, 11, 17 ] ]
						}
					}
				}
			}, "Server: received payload is correct" );
			result = iotivity.OCDoResponse( {
				requestHandle: request.requestHandle,
				resourceHandle: request.resource,
				resourceUri: "/a/" + uuid,
				ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
				payload: null,
				sendVendorSpecificHeaderOptions: []
			} );

			testUtils.stackOKOrDie( "Server", "OCDoResponse(put)", result );

			cleanup();

			return iotivity.OCEntityHandlerResult.OC_EH_OK;
		}
	];

// Create resource
result = iotivity.OCCreateResource(
	resourceHandleReceptacle,
	"core.light",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	"/a/" + uuid,
	function( flag, request ) {
		return requestSequence[ requestSequenceNumber++ ]( flag, request );
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE );
testUtils.stackOKOrDie( "Server", "OCCreateResource", result );

// Report that the server has successfully created its resource(s).
console.log( JSON.stringify( { ready: true } ) );

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

var result, notifyObserversTimeoutId, observationId,
	uuid = process.argv[ 2 ],
	notificationCount = 0,
	getRequestCount = 0,
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

	if ( notifyObserversTimeoutId ) {
		clearTimeout( notifyObserversTimeoutId );
		notifyObserversTimeoutId = null;
	}

	testUtils.assert( "ok", true, "Server: OCProcess succeeded " + processCallCount + " times" );

	testUtils.assert( "ok", true,
		"Server: OCNotifyAllObservers succeeded " + notificationCount + " times" );

	cleanupResult = iotivity.OCDeleteResource( resourceHandleReceptacle.handle );
	testUtils.stackOKOrDie( "Server", "OCDeleteResource", cleanupResult );

	cleanupResult = iotivity.OCStop();
	if ( testUtils.stackOKOrDie( "Server", "OCStop", cleanupResult ) ) {
		console.log( JSON.stringify( { killPeer: true } ) );
		process.exit( 0 );
	}
}

console.log( JSON.stringify( { assertionCount: 17 } ) );

// Initialize
result = iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );
testUtils.stackOKOrDie( "Server", "OCInit", result );

// Set up the process loop
processLoop = setInterval( function idlyLoopOCProcess() {
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

// Set up the notification loop
function notifyObservers() {
	var notificationResult;

	if ( resourceHandleReceptacle.handle ) {
		notificationResult = iotivity.OCNotifyAllObservers(
			resourceHandleReceptacle.handle,
			iotivity.OCQualityOfService.OC_HIGH_QOS );

		if ( notificationResult === iotivity.OCStackResult.OC_STACK_OK ) {
			notificationCount++;
		} else {
			testUtils.stackOKOrDie(
				"Server",
				"OCNotifyAllObservers(after " + notificationCount + " successful calls)",
				notificationResult );
		}
	}

	// Run this function again between 200 and 400 ms from now
	notifyObserversTimeoutId = setTimeout( notifyObservers, Math.random() * 200 + 200 );
}

// Create a new resource
result = iotivity.OCCreateResource(

	// The bindings fill in this object
	resourceHandleReceptacle,

	"core.fan",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	"/a/" + uuid,
	function( flag, request ) {
		var responseResult;

		// We expect the first request to be the attachment of an observer
		if ( getRequestCount === 0 ) {
			testUtils.assert( "deepEqual",
				testUtils.lookupBitfieldValueNames( "OCEntityHandlerFlag", flag ),
				{ OC_REQUEST_FLAG: true, OC_OBSERVE_FLAG: true },
				"Server: The first request has both the request and observe flags set" );
			testUtils.assert( "ok", request.obsInfo.obsId !== 0,
				"Server: Observation ID is not zero (" + request.obsInfo.obsId + ")" );
			testUtils.assert( "strictEqual",
				testUtils.lookupEnumValueName( "OCObserveAction", request.obsInfo.action ),
				"OC_OBSERVE_REGISTER",
				"Server: Action is set to OC_OBSERVE_REGISTER" );

			observationId = request.obsInfo.obsId;
			notifyObserversTimeoutId = setTimeout( notifyObservers, 0 );

		// We expect the last request to be the detachment of the observer
		} else if ( getRequestCount === 5 ) {
			testUtils.assert( "deepEqual",
				testUtils.lookupBitfieldValueNames( "OCEntityHandlerFlag", flag ),
				{ OC_REQUEST_FLAG: true, OC_OBSERVE_FLAG: true },
				"Server: The last request has both the request and observe flags set" );
			testUtils.assert( "strictEqual", request.obsInfo.obsId, observationId,
				"Server: Observation ID is the same as during the initial request" );
			testUtils.assert( "strictEqual",
				testUtils.lookupEnumValueName( "OCObserveAction", request.obsInfo.action ),
				"OC_OBSERVE_DEREGISTER",
				"Server: Action is set to OC_OBSERVE_DEREGISTER" );

			// This test is concluded so we clean up
			cleanup();
		}

		responseResult = iotivity.OCDoResponse( {
			requestHandle: request.requestHandle,
			resourceHandle: request.resource,
			ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
			payload: {
				type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
				values: {
					observedValue: uuid + "-" + getRequestCount
				}
			},
			resourceUri: "/a/" + uuid,
			sendVendorSpecificHeaderOptions: []
		} );
		testUtils.stackOKOrDie( "Server", "OCDoResponse", responseResult );

		getRequestCount++;

		return iotivity.OCEntityHandlerResult.OC_EH_OK;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE |
	iotivity.OCResourceProperty.OC_OBSERVABLE );
testUtils.stackOKOrDie( "Server", "OCCreateResource", result );

// Report that the server has successfully created its resource(s).
console.log( JSON.stringify( { ready: true } ) );

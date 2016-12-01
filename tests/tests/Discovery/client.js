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
	staleHandleCheckLoop = null,
	discoverHandleReceptacle = {},
	iotivity = require( process.argv[ 3 ] + "/lowlevel" ),
	testUtils = require( "../../utils" )( iotivity );

function cleanup() {
	var cleanupResult;

	if ( processLoop ) {
		clearInterval( processLoop );
		processLoop = null;
	}

	if ( staleHandleCheckLoop ) {
		clearInterval( staleHandleCheckLoop );
		staleHandleCheckLoop = null;
	}

	testUtils.assert( "ok", true, "Client: OCProcess succeeded " + processCallCount + " times" );

	cleanupResult = iotivity.OCStop();
	if ( testUtils.stackOKOrDie( "Client", "OCStop", cleanupResult ) ) {
		console.log( JSON.stringify( { killPeer: true } ) );
		process.exit( 0 );
	}
}

console.log( JSON.stringify( { assertionCount: 6 } ) );

// Initialize
result = iotivity.OCInit( null, 0, iotivity.OCMode.OC_CLIENT );
testUtils.stackOKOrDie( "Client", "OCInit", result );

// Set up OCProcess loop
processLoop = setInterval( function() {
	var processResult = iotivity.OCProcess();

	if ( processResult === iotivity.OCStackResult.OC_STACK_OK ) {
		processCallCount++;
	} else {
		testUtils.stackOKOrDie(
			"Client",
			"OCProcess(after " + processCallCount + " successful calls)",
			processResult );
	}
}, 100 );

staleHandleCheckLoop = setInterval( function() {
	if ( discoverHandleReceptacle.handle.stale ) {
		testUtils.assert( "ok", true, "Client: Handle found stale after deleting transaction" );
		cleanup();
	}
}, 500 );

// Discover
result = iotivity.OCDoResource(
	discoverHandleReceptacle,
	iotivity.OCMethod.OC_REST_DISCOVER,
	iotivity.OC_MULTICAST_DISCOVERY_URI,
	null,
	null,
	iotivity.OCConnectivityType.CT_DEFAULT,
	iotivity.OCQualityOfService.OC_HIGH_QOS,
	function( handle, response ) {

		// We retain the discovery callback until we've found the resource identified by the uuid.
		var returnValue = iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;

		if ( testUtils.findResource( response, uuid ) ) {

			// We've successfully completed the test so let's kill the server and test the cleanup.
			testUtils.assert( "ok", true, "Client: Resource found" );
			returnValue = iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;

		}

		return returnValue;
	},
	null );
testUtils.stackOKOrDie( "Client", "OCDoResource(discovery)", result );

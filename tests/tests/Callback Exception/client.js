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
	processCallCount = 0,
	processLoop = null,
	discoverHandleReceptacle = {},
	iotivity = require( "../../../lowlevel" ),
	testUtils = require( "../../utils" )( iotivity );

function cleanup() {
	var cleanupResult;

	if ( processLoop ) {
		clearInterval( processLoop );
		processLoop = null;
	}

	testUtils.assert( "ok", true, "Client: OCProcess succeeded " + processCallCount + " times" );

	cleanupResult = iotivity.OCStop();
	if ( testUtils.stackOKOrDie( "Client", "OCStop", cleanupResult ) ) {
		console.log( JSON.stringify( { killPeer: true } ) );
		process.exit( 0 );
	}
}

process.on( "uncaughtException", function( exception ) {
	testUtils.assert( "strictEqual", exception.message, "Something",
		"Client: Caught the correct exception" );
	cleanup();
} );

console.log( JSON.stringify( { assertionCount: 5 } ) );

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

// Discover
result = iotivity.OCDoResource(
	discoverHandleReceptacle,
	iotivity.OCMethod.OC_REST_DISCOVER,
	iotivity.OC_MULTICAST_DISCOVERY_URI,
	null,
	null,
	iotivity.OCConnectivityType.CT_DEFAULT,
	iotivity.OCQualityOfService.OC_HIGH_QOS,
	function() {
		throw new Error( "Something" );
	},
	null );

testUtils.stackOKOrDie( "Client", "OCDoResource(discovery)", result );

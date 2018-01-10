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
	iotivity = require( process.argv[ 3 ] + "/lowlevel" ),
	testUtils = require( "../../utils" )( iotivity ),
	cleanupRequest = require( "./cleanupRequest" );

function cleanup() {
	if ( processLoop ) {
		clearInterval( processLoop );
		processLoop = null;
	}
	testUtils.assert( "ok", true, "Client: OCProcess succeeded " + processCallCount + " times" );
	testUtils.stackOKOrDie( "Client", "OCStop", iotivity.OCStop() );
	process.exit( 0 );
}

console.log( JSON.stringify( { assertionCount: 7 } ) );

// Initialize
result = iotivity.OCRegisterPersistentStorageHandler( require( "../../../lib/StorageHandler" )() );
testUtils.stackOKOrDie( "Client", "OCRegisterPersistentStorageHandler", result );

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

var rdDevAddr;

// Keep retrieving /oic/rd from the resource directory in an idle loop until it returns the
// expected resource.
function doOneTargetedDiscovery() {
	iotivity.OCDoResource(
		{},
		iotivity.OCMethod.OC_REST_GET,
		iotivity.OC_RSRVD_WELL_KNOWN_URI,
		rdDevAddr,
		null,
		iotivity.OCConnectivityType.CT_DEFAULT,
		iotivity.OCQualityOfService.OC_HIGH_QOS,
		function retrieveRDOicResResponse( handle, response ) {
			var payload;

			for ( payload = response.payload; payload; payload = payload.next ) {
				if ( payload.uri === "/a/" + uuid ) {
					break;
				}
			}

			if ( payload ) {
				cleanupRequest( "Client", iotivity, testUtils, response.addr,
					"/a/" + uuid + "-xyzzy" );
			} else {
				setTimeout( doOneTargetedDiscovery, 0 );
			}

			return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
		},
		null );
}

function doOneDiscovery() {
	testUtils.stackOKOrDie( "Client", "OCRDDiscover",
		iotivity.OCRDDiscover(
			{},
			iotivity.OCConnectivityType.CT_DEFAULT,
			function OCRDDiscoverResponse( handle, response ) {
				testUtils.stackOKOrDie( "Client", "OCRDDiscover response", response.result );

				rdDevAddr = response.addr;

				setTimeout( doOneTargetedDiscovery, 0 );
				return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
			},
			iotivity.OCQualityOfService.OC_HIGH_QOS ) );
}

setTimeout( doOneDiscovery, 0 );

process.on( "message", cleanup );

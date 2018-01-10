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
	testUtils.assert( "ok", true, "Server: OCProcess succeeded " + processCallCount + " times" );
	testUtils.stackOKOrDie( "Server", "OCStop", iotivity.OCStop() );
	process.exit( 0 );
}

console.log( JSON.stringify( { assertionCount: 8 } ) );

// Initialize
result = iotivity.OCInit( null, 0, iotivity.OCMode.OC_CLIENT_SERVER );
testUtils.stackOKOrDie( "Server", "OCInit", result );

// Set up OCProcess loop
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

// Discover
result = iotivity.OCRDDiscover(
	{},
	iotivity.OCConnectivityType.CT_DEFAULT,
	function OCRDDiscoverResponse( handle, response ) {
		var resourceHandleReceptacle = {};

		testUtils.stackOKOrDie( "Server", "OCCreateResource",
			iotivity.OCCreateResource( resourceHandleReceptacle, "core.light", "oic.if.baseline",
				"/a/" + uuid, function ResourceEntityHandler() {
					return iotivity.OCEntityHandlerResponse.OC_EH_OK;
				}, iotivity.OCResourceProperty.OC_DISCOVERABLE ) );

		testUtils.stackOKOrDie( "Server", "OCRDPublish",
			iotivity.OCRDPublish(
				{},
				response.addr.addr + ":" + response.addr.port,
				iotivity.OCConnectivityType.CT_DEFAULT,
				[ resourceHandleReceptacle.handle ],
				86400,
				function OCRDPublishResponse( handle, response ) {
					var index;
					var links = response.payload.values.links;

					for ( index in links ) {
						if ( links[ index ].href === "/a/" + uuid ) {
							break;
						}
					}

					testUtils.assert( "ok", index < links.length,
						"Server: Posted resource found in OCRDPublish response" );

					cleanupRequest( "Server", iotivity, testUtils, response.addr,
						"/a/" + uuid + "-xyzzy" );

					return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
				},
				iotivity.OCQualityOfService.OC_HIGH_QOS ) );

		return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
	},
	iotivity.OCQualityOfService.OC_HIGH_QOS );
testUtils.stackOKOrDie( "Server", "OCRDDiscover", result );

process.on( "message", cleanup );

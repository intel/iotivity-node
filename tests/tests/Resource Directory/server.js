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
	cleanupPrerequisiteCount = 0,
	uuid = process.argv[ 2 ],
	terminationResourceHandleReceptacle = {},
	processCallCount = 0,
	processLoop = null,
	iotivity = require( process.argv[ 3 ] + "/lowlevel" ),
	testUtils = require( "../../utils" )( iotivity );

function cleanup() {
	if ( ++cleanupPrerequisiteCount < 2 ) {
		return;
	}

	if ( processLoop ) {
		clearInterval( processLoop );
		processLoop = null;
	}

	testUtils.assert( "ok", true, "RD Server: OCProcess succeeded " + processCallCount +
		" times" );

	if ( terminationResourceHandleReceptacle.handle &&
			!terminationResourceHandleReceptacle.handle.stale ) {
		testUtils.stackOKOrDie( "RD Server", "OCDeleteResource",
			iotivity.OCDeleteResource( terminationResourceHandleReceptacle.handle ) );
	}

	testUtils.stackOKOrDie( "RD Server", "OCRDStop", iotivity.OCRDStop() );
	testUtils.stackOKOrDie( "RD Server", "OCStop", iotivity.OCStop() );
	console.log( JSON.stringify( { killPeer: true } ) );
	process.exit( 0 );
}

console.log( JSON.stringify( { assertionCount: 8 } ) );

// Initialize
result = iotivity.OCRegisterPersistentStorageHandler( require( "../../../lib/StorageHandler" )() );
testUtils.stackOKOrDie( "RD Server", "OCRegisterPersistentStorageHandler", result );

result = iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );
testUtils.stackOKOrDie( "RD Server", "OCInit", result );

result = iotivity.OCRDStart();
testUtils.stackOKOrDie( "RD Server", "OCRDStart", result );

// Set up process loop
processLoop = setInterval( function() {
	var processResult = iotivity.OCProcess();

	if ( processResult === iotivity.OCStackResult.OC_STACK_OK ) {
		processCallCount++;
	} else {
		testUtils.stackOKOrDie(
			"RD Server",
			"OCProcess(after " + processCallCount + " successful calls)",
			processResult );
	}
}, 100 );

result = iotivity.OCCreateResource( terminationResourceHandleReceptacle,
	"core.light", "oic.if.baseline", "/a/" + uuid + "-xyzzy",
	function terminationResourceEntityHandler( flag, request ) {
		if ( request.resource === terminationResourceHandleReceptacle.handle &&
				request.method === iotivity.OCMethod.OC_REST_POST &&
				request.payload &&
				request.payload.type === iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION &&
				request.payload.values.killPeer === true ) {
			cleanup();
		}
		return iotivity.OCEntityHandlerResult.OC_EH_OK;
	}, iotivity.OCResourceProperty.OC_DISCOVERABLE );
testUtils.stackOKOrDie( "RD Server", "OCCreateResource", result );

// Report that the server has successfully created its resource(s).
console.log( JSON.stringify( { ready: true } ) );

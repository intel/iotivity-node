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

var messagePrefix = "Client 2",
	iAmSpecial = false,
	firstResponse = true,
	observeCount = 0,
	uuid = process.argv[ 2 ],
	processCallCount = 0,
	processLoop = null,
	discoverHandleReceptacle = {},
	iotivity = require( "../../../lowlevel" ),
	result = iotivity.OCStackResult.OC_STACK_ERROR,
	testUtils = require( "../../utils" )( iotivity );

function cleanup() {
	var cleanupResult;

	if ( processLoop ) {
		clearInterval( processLoop );
		processLoop = null;
	}

	testUtils.assert( "ok", true,
		messagePrefix + ": OCProcess succeeded " + processCallCount + " times" );

	cleanupResult = iotivity.OCStop();
	testUtils.stackOKOrDie( messagePrefix, "OCStop", cleanupResult );
}

function doObserveRequest( destination ) {
	var observeResult,
		observeHandleReceptacle = {};

	observeResult = iotivity.OCDoResource(
		observeHandleReceptacle,
		iotivity.OCMethod.OC_REST_OBSERVE,
		"/a/" + uuid,
		destination,
		null,
		iotivity.OCConnectivityType.CT_DEFAULT,
		iotivity.OCQualityOfService.OC_HIGH_QOS,
		function( handle, response ) {
			var putResult,
				putHandleReceptacle = {};

			if ( firstResponse ) {

				// The first observe request to reach the server will be given 10 notifications,
				// not 5, so this affects how many notifications we expect.
				testUtils.assert( "ok",
					( response.payload.values.message === "You are special." ||
					response.payload.values.message === "You are not special." ),
					messagePrefix + ": First response is as expected: " +
					"'" + response.payload.values.message + "'" );
				firstResponse = false;
				iAmSpecial = ( response.payload.values.message === "You are special." );
				console.log( JSON.stringify( {
					assertionCount: ( iAmSpecial ? 18 : 13 )
				} ) );
			} else {
				testUtils.assert( "deepEqual",
					response.payload,
					{
						type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
						values: { observedValue: uuid + "-" + observeCount }
					},
					messagePrefix + ": Observed value is as expected" );
				observeCount++;

				// If the requisite number of observations have arrived, inform the server that
				// we're done.
				if ( observeCount >= ( iAmSpecial ? 10 : 5 ) ) {
					putResult = iotivity.OCDoResource(
						putHandleReceptacle,
						iotivity.OCMethod.OC_REST_PUT,
						"/a/" + uuid,
						response.addr,
						{
							type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
							values: { done: true }
						},
						iotivity.OCConnectivityType.CT_DEFAULT,
						iotivity.OCQualityOfService.OC_HIGH_QOS,
						function() {
							return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
						},
						null );
					testUtils.stackOKOrDie( messagePrefix, "OCDoResource(put)", putResult );
				}
			}

			return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
		},
		null );
	testUtils.stackOKOrDie( messagePrefix, "OCDoResource(observation)", observeResult );
}

// Initialize
result = iotivity.OCInit( null, 0, iotivity.OCMode.OC_CLIENT );
testUtils.stackOKOrDie( messagePrefix, "OCInit", result );

// Set up OCProcess loop
processLoop = setInterval( function() {
	var processResult = iotivity.OCProcess();

	if ( processResult === iotivity.OCStackResult.OC_STACK_OK ) {
		processCallCount++;
	} else {
		testUtils.stackOKOrDie(
			messagePrefix,
			"OCProcess(after " + processCallCount + " successful calls)",
			processResult );
	}
}, 100 );

// Discover resources and list them
result = iotivity.OCDoResource(

	// The bindings fill in this object
	discoverHandleReceptacle,

	iotivity.OCMethod.OC_REST_DISCOVER,

	// Standard path for discovering resources
	iotivity.OC_MULTICAST_DISCOVERY_URI,

	// There is no destination
	null,

	// There is no payload
	null,
	iotivity.OCConnectivityType.CT_DEFAULT,
	iotivity.OCQualityOfService.OC_HIGH_QOS,
	function( handle, response ) {

		// We retain the discovery callback until we've found the resource identified by the uuid.
		var returnValue = iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;

		if ( testUtils.findResource( response, uuid ) ) {

			// We've successfully found the resource so let's issue a GET request on it.
			testUtils.assert( "ok", true, messagePrefix + ": Resource found" );
			doObserveRequest( response.addr );
			returnValue = iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
		}

		return returnValue;
	},

	// There are no header options
	null );
testUtils.stackOKOrDie( messagePrefix, "OCDoResource(discovery)", result );

// Exit gracefully when interrupted
process.on( "SIGINT", cleanup );

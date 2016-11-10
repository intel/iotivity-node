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

var intervalId,
	handleReceptacle = {},
	iotivity = require( "iotivity-node/lowlevel" );

// Start iotivity and set up the processing loop
iotivity.OCInit( null, 0, iotivity.OCMode.OC_CLIENT );

intervalId = setInterval( function() {
	iotivity.OCProcess();
}, 1000 );

function listenForPresence() {

	// Request resource presence notifications
	console.log( "OCDoResource(presence): " + iotivity.OCDoResource(

			// The bindings fill in this object
			handleReceptacle,

			iotivity.OCMethod.OC_REST_DISCOVER,
			iotivity.OC_RSRVD_PRESENCE_URI,

			null,
			null,
			iotivity.OCConnectivityType.CT_DEFAULT,
			iotivity.OCQualityOfService.OC_HIGH_QOS,
			function( handle, response ) {
				var returnValue = iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;

				console.log( "Received response to PRESENCE request:" );
				console.log( JSON.stringify( response, null, 4 ) );

				if ( response.result === iotivity.OCStackResult.OC_STACK_COMM_ERROR ) {

					console.log( "Request failed. Trying again." );
					setTimeout( listenForPresence, 0 );
					returnValue = iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
				}

				return returnValue;
			},

			// There are no header options
			null ) );
}

listenForPresence();

// Exit gracefully when interrupted
process.on( "SIGINT", function() {
	console.log( "SIGINT: Quitting..." );

	// Tear down the processing loop and stop iotivity
	clearInterval( intervalId );
	iotivity.OCStop();

	// Exit
	process.exit( 0 );
} );

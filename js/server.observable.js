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

var intervalId, sensor,
	handleReceptacle = {},
	observerIds = [],

	// This is the same value as server.get.js
	sampleUri = "/a/iotivity-node-observe-sample",
	iotivity = require( "iotivity-node/lowlevel" );

console.log( "Starting OCF stack in server mode" );

// Start iotivity and set up the processing loop
iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );

iotivity.OCSetDeviceInfo( {
	specVersion: "res.1.0.0",
	dataModelVersions: [ "abc.0.0.1" ],
	deviceName: "server.observable",
	types: []
} );
iotivity.OCSetPlatformInfo( {
	platformID: "server.observe.sample",
	manufacturerName: "iotivity-node"
} );

intervalId = setInterval( function() {
	iotivity.OCProcess();
}, 1000 );

console.log( "Local device ID: " + iotivity.OCGetServerInstanceIDString() );

var sensor = require( "./mock-sensor" )()
	.on( "change", function( data ) {

		console.log( "Sensor data has changed. " +
			( observerIds.length > 0 ?
				"Notifying " + observerIds.length + " observers." :
				"No observers in list." ) );
		if ( observerIds.length > 0 ) {
			iotivity.OCNotifyListOfObservers(
				handleReceptacle.handle,
				observerIds,
				{
					type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
					values: data
				},
				iotivity.OCQualityOfService.OC_HIGH_QOS );
		}
	} );

console.log( "Registering resource" );

// Create a new resource
iotivity.OCCreateResource(

	// The bindings fill in this object
	handleReceptacle,

	"core.fan",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	sampleUri,
	function( flag, request ) {
		var observerIdIndex;

		console.log( "Entity handler called with flag = " + flag + " and the following request:" );
		console.log( JSON.stringify( request, null, 4 ) );

		if ( flag & iotivity.OCEntityHandlerFlag.OC_OBSERVE_FLAG ) {
			if ( request.obsInfo.obsId !== 0 ) {
				if ( request.obsInfo.action === iotivity.OCObserveAction.OC_OBSERVE_REGISTER ) {

					// Add new observer to list.
					observerIds.push( request.obsInfo.obsId );
				} else if ( request.obsInfo.action ===
						iotivity.OCObserveAction.OC_OBSERVE_DEREGISTER ) {

					// Remove requested observer from list.
					observerIdIndex = observerIds.indexOf( request.obsInfo.obsId );
					if ( observerIdIndex >= 0 ) {
						observerIds.splice( observerIdIndex, 1 );
					}
				}
			}
		}

		if ( request.requestHandle ) {
			iotivity.OCDoResponse( {
				requestHandle: request.requestHandle,
				resourceHandle: request.resource,
				ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
				payload: {
					type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
					values: sensor.currentData()
				},
				resourceUri: sampleUri,
				sendVendorSpecificHeaderOptions: []
			} );
		}

		return iotivity.OCEntityHandlerResult.OC_EH_OK;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE |
	iotivity.OCResourceProperty.OC_OBSERVABLE );

console.log( "Server ready" );

// Exit gracefully when interrupted
process.on( "SIGINT", function() {
	console.log( "SIGINT: Quitting..." );

	// Tear down the processing loop and stop iotivity
	clearInterval( intervalId );
	iotivity.OCDeleteResource( handleReceptacle.handle );
	iotivity.OCStop();

	// Exit
	process.exit( 0 );
} );

/** Copyright 2017 Vprime
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.*/

/** @author Rami Alshafi <ralshafi@vprime.com> */

var OCProcessInterval;
var ValidInput = [ "0", "1", "2", "3", "4", "5", "6" ];
var iotivity = require( "iotivity-node/lowlevel" );
var envirophat = {
	uri: "/a/envirophat",
	destination: {}
};

function payloadCreate( status ) {
	console.log( "[payloadCreate] state: " + status );
	return {
		type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
		values: {
			command: status
		}
	};
}

function InterfaceConfig( ) {
	console.log( "[InterfaceConfig] In" );
	process.stdin.setRawMode( true );
	process.stdin.resume( );
	process.stdin.setEncoding( "utf8" );
	process.stdin.on( "data", ProcessKey );
}

function iotProcess( ) {
	iotivity.OCProcess( );
}
function InitPostRequest( currentKey ) {
	console.log( "[InitPostRequest] key: " + currentKey );
	var payload = payloadCreate( currentKey );
	invokeOCDoResource( envirophat.uri,
						payload,
						iotivity.OCMethod.OC_REST_POST,
						responseCallback,
						envirophat.destination );
}

function InitGetRequest( currentKey ) {
	console.log( "[InitGetRequest] key: " + currentKey );
	var payload = payloadCreate( currentKey );
	invokeOCDoResource( envirophat.uri,
						payload,
						iotivity.OCMethod.OC_REST_GET,
						responseCallback,
						envirophat.destination );
}

function responseCallback( handle, response ) {
	var result = response.result;
	console.log( "[responseCallback] Received response to request with result: " + result );
	console.log( JSON.stringify( response.payload, null, 4 ) );
	return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
}

function discoveryReqCB( handle, response ) {
	console.log( "[discoveryReqCB] Received response to DISCOVER request:" );
	var resources = response && response.payload && response.payload.resources;
	var resourceCount = resources ? resources.length : 0;
	for ( var index = 0; index < resourceCount; index++ ) {
		if ( resources[ index ].uri === envirophat.uri ) {
			console.log( "[discoveryReqCB]: found led resource" );
			envirophat.destination = response.addr;
			console.log( "[discoveryReqCB]: enter 0,1,2,3,4 or Ctrl+c" );
			}
		}
	return iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
	}

function invokeOCDoResource( query, payload, method, callback, address ) {
	var handle = {};
	var connectivityType = iotivity.OCConnectivityType.CT_ADAPTER_IP;
	var QoS = iotivity.OCQualityOfService.OC_LOW_QOS;
	iotivity.OCDoResource( handle,
		method,
		query,
		address,
		payload,
		connectivityType,
		QoS,
		callback,
		null,
		null );
}

function ProcessKey( key ) {
	console.log( "[ProcessKey] key = " + key );
	if ( key === "\u0003" ) {
		console.log( "[ProcessKey] SIGINT: Quitting the app gracefully..." );
		clearInterval( OCProcessInterval );
		iotivity.OCStop( );
		process.exit( 0 );
	}
	if ( key in ValidInput ) {
		if ( key === "1" || key === "0" ) {
			InitPostRequest( key );
		} else {
			InitGetRequest( key );
		}
	} else {
		console.log( "[ProcessKey]: This key ( " + key + " ) is not supported! try again\n" );
	}
}

function discoverResources( ) {
	console.log( "[discoverResources]: In" );
	invokeOCDoResource( iotivity.OC_MULTICAST_DISCOVERY_URI,
						null,
						iotivity.OCMethod.OC_REST_DISCOVER,
						discoveryReqCB,
						null );
}

function main( ) {
	InterfaceConfig( );
	console.log( "[main] Starting OCF stack in client mode" );
	iotivity.OCInit( null, 0, iotivity.OCMode.OC_CLIENT );
	console.log( "[main] Issuing discovery request" );
	discoverResources( );
	OCProcessInterval = setInterval( iotProcess, 1000 );
}

main( );

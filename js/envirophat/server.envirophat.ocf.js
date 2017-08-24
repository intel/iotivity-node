/** Copyright 2017 Vprime
*
* Licensed under the Apache License, Version 2.0 (the "License" );
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


var iotivity = require( "iotivity-node/lowlevel" );
var execute = require( "child_process" ).exec;
var OCDiscoverable = iotivity.OCResourceProperty.OC_DISCOVERABLE;
var OCObservable = iotivity.OCResourceProperty.OC_OBSERVABLE;
var PythonFile = "enviro_phat.py";
var ValidInput = [ "0", "1", "2", "3", "4", "5" ];
var OCProcessInterval;
var envirophat = {
	uri: "/a/envirophat",
	handle: { },
	light: { rgb: [ 0, 0, 0 ] },
	motion: { accelerometer: [ 0, 0, 0 ] },
	weather: { temperature: 0 },
	leds: { status: null }
};
var deviceInfo = {
	specVersion: "res.1.1.0",
	dataModelVersions: [ "abc.0.0.1" ],
	deviceName: "server.post",
	types: [ ]
};
var platformInfo = {
	platformID: "server.post.sample",
	manufacturerName: "iotivity-node"
};
process.on( "SIGINT", iotStop );

function CaptureOutput( error, data ) {
	var result = data.split( ":" );
	console.log( "[CaptureOutput]:updating envirophat object w/captured data" + data.split( ":" ) );
	console.log( "temperature before the update" + envirophat.weather.temperature );
	console.log( "accelerometer before the update" + envirophat.motion.accelerometer );
	console.log( "light rgb before the update" + envirophat.light.rgb );
	if ( result[ 0 ] === "weather" ) {
		if ( result[ 1 ] === "temperature" ) {
			envirophat.weather.temperature = result[ 2 ].replace( /\n/g, "" );
		} else {
			console.log( "error" );
		}
	} else if ( result[ 0 ] === "motion" ) {
		if ( result[ 1 ] === "accelerometer" ) {
			envirophat.motion.accelerometer = result[ 2 ].replace( /\n/g, "" );
		} else {
		console.log( "error" );
		}
	} else if ( result[ 0 ] === "light" ) {
		if ( result[ 1 ] === "rgb" ) {
			envirophat.light.rgb = result[ 2 ].replace( /\n/g, "" );
		} else {
		console.log( "error" );
		}
	} else if ( result[ 0 ] === "leds" ) {
		if ( result[ 1 ] === "status" ) {
			envirophat.leds.status = result[ 2 ].replace( /\n/g, "" );
		} else {
		console.log( "error" );
		}
	} else {
		console.log( "error" );
	}
	console.log( "temperature after the update" + envirophat.weather.temperature );
	console.log( "accelerometer after the update" + envirophat.motion.accelerometer );
	console.log( "light rgb after the update" + envirophat.light.rgb );
}
function write( command ) {
	var FullCommand = "python " + PythonFile + " " + command;
	console.log( "[ write ]: executing this command : " + FullCommand );
	execute( "python " + PythonFile + " " + command, CaptureOutput );
}

function iotProcess( ) {
	iotivity.OCProcess( );
}

function getResponsePayload( ) {
	console.log( "[ getResponsePayload ]: constructing response for envirophat" );
	console.log( envirophat );
	return {
		type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
		values: {
			uri: envirophat.uri,
			light: envirophat.light.rgb,
			motion: envirophat.motion.accelerometer,
			weather: envirophat.weather.temperature,
			leds: envirophat.leds.status
		}
	};
}
function processPost( command ) {
	if ( command === "0" || command === "1" ) {
		write( command );
	} else {
		console.log( "[ ProcessPost ]: Error: received a post request" );
	}
}
function processGet( command ) {
	if ( !( command === "0" || command === "1" ) ) {
		envirophat.leds.status = "False"; // for some reason the leds turn off.
		write( command );
	} else {
		console.log( "[ processGet ]: Error: received a get request for tunring leds or or off" );
	}
}
function constructResponse( request ) {
	var responseResult = iotivity.OCEntityHandlerResult.OC_EH_ERROR;
	var command = request.payload.values.command;
	if ( command && ( command in ValidInput ) ) {
		responseResult = iotivity.OCEntityHandlerResult.OC_EH_OK;
		if ( request.method === iotivity.OCMethod.OC_REST_POST ) {
			processPost( command );
		} else if ( request.method === iotivity.OCMethod.OC_REST_GET ) {
			processGet( command );
		}
	}
	var responsePayload = getResponsePayload( );
	return {
		requestHandle: request.requestHandle,
		resourceHandle: request.resource,
		ehResult: responseResult,
		payload: responsePayload,
		resourceUri: envirophat.uri,
		sendVendorSpecificHeaderOptions: [ ]
	};
}

function ocEntityHandler( flag, request ) {
	console.log( "[ ocEntityHandler ]: Entity handler called with flag = " + flag );
	var response = constructResponse( request );
	var res = iotivity.OCDoResponse( response );
	console.log( "[ ocEntityHandler ]: Response sent with result :" + res );
	return res;
}

function iotStop( ) {
	console.log( "[ iotStop ]: SIGINT: Quitting..." );
	clearInterval( OCProcessInterval );
	iotivity.OCStop( );
	process.exit( 0 );
}

function createResources( ) {
	var resourceProperties = OCDiscoverable | OCObservable;
	var res = iotivity.OCCreateResource(
		envirophat.handle,
		"core.led",
		iotivity.OC_RSRVD_INTERFACE_DEFAULT,
		envirophat.uri,
		ocEntityHandler,
		resourceProperties );
	console.log( "[ createResources ]: Created resource with result: " + res );

}
function main( ) {
	console.log( "[ main ]: Starting OCF stack in server mode" );
	iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );

	console.log( "[ main ]: Registerign device and platform information" );
	iotivity.OCSetDeviceInfo( deviceInfo );
	iotivity.OCSetPlatformInfo( platformInfo );

	console.log( "[ main ]: creating resources" );
	createResources( );
	OCProcessInterval = setInterval( iotProcess, 1000 );
	console.log( "[ main ]: Server ready" );
}
main( );


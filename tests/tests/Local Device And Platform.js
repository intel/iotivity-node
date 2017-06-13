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

var iotivity = require( process.argv[ 3 ] + "/lowlevel" );
var testUtils = require( "../utils" )( iotivity );
var isequal = require( "lodash.isequal" );

// Report assertion count
console.log( JSON.stringify( { assertionCount: 22 } ) );

// Init
testUtils.stackOKOrDie( "OCInit", iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER ) );

function getAndSet( message, payloadType, property, value ) {
	var result = {};
	testUtils.stackOKOrDie( "Set " + message + " to " + JSON.stringify( value ),
		iotivity.OCSetPropertyValue( iotivity.OCPayloadType[ payloadType ],
			iotivity[ property ], value ) );
	testUtils.stackOKOrDie( "Get " + message,
		iotivity.OCGetPropertyValue( iotivity.OCPayloadType[ payloadType ],
			iotivity[ property ], result ) );

	return result.value;
}

function testProperty( message, payloadType, property, value ) {
	testUtils.assert( "ok", isequal( getAndSet( message, payloadType, property, value ), value ),
		message + ": retrieved value is strictly equal to set value" );
}

// string properties
testProperty( "platform manufacturer url", "PAYLOAD_TYPE_PLATFORM", "OC_RSRVD_MFG_URL",
	"http://" + Math.round( Math.random() * 1048576 ) );

testProperty( "device name", "PAYLOAD_TYPE_DEVICE", "OC_RSRVD_DEVICE_NAME",
	"" + Math.round( Math.random() * 1048576 ) );

testProperty( "spec version", "PAYLOAD_TYPE_DEVICE", "OC_RSRVD_SPEC_VERSION",
	Math.round( Math.random() * 9 ) + "." +
	Math.round( Math.random() * 9 ) + "." +
	Math.round( Math.random() * 9 ) );

testProperty( "manufacturer name", "PAYLOAD_TYPE_PLATFORM", "OC_RSRVD_MFG_NAME",
	[ "Abra", "Cadabra", "Csiribu", "Csiriba", "Simsalabim" ][ Math.round( Math.random() * 4 ) ] );

var sampleDMV = "res." +
	Math.round( Math.random() * 9 ) + "." +
	Math.round( Math.random() * 9 ) + "." +
	Math.round( Math.random() * 9 );
var dmvSetResult = getAndSet( "data model version", "PAYLOAD_TYPE_DEVICE",
	"OC_RSRVD_DATA_MODEL_VERSION", sampleDMV );

testUtils.assert( "strictEqual", sampleDMV, dmvSetResult.join( "," ),
	"data model version: retrieved value is strictly equal to set value" );

// date properties
testProperty( "manufacture date", "PAYLOAD_TYPE_PLATFORM", "OC_RSRVD_MFG_DATE",
	new Date( 1299752880000 ) );

testProperty( "system time", "PAYLOAD_TYPE_PLATFORM", "OC_RSRVD_SYSTEM_TIME",

	// Looks like iotivity doesn't have millisecond resolution
	new Date( Math.round( Date.now() / 1000 ) * 1000 ) );

process.exit( 0 );

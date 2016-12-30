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

var csdk = require( process.argv[ 3 ] + "/lib/csdk" );
var client = require( process.argv[ 3 ] ).client;
var util = require( "../assert-to-console" );
var handles = require( process.argv[ 3 ] + "/lib/ClientHandles" );

var eventName;

function devicelost() {}

console.log( JSON.stringify( { assertionCount: 3 } ) );

handles.on( "newListener", function( event ) {
	util.assert( "deepEqual", JSON.parse( event ), {
		requestUri: csdk.OC_RSRVD_PRESENCE_URI,
		method: "OC_REST_DISCOVER"
	}, "Global presence listener is correct" );
	eventName = event;
} );

client.on( "devicelost", devicelost );

util.assert( "strictEqual", handles.listeners( eventName ).length, 1,
	"One global presence listener present when first 'devicelost' listener is added" );

client.removeListener( "devicelost", devicelost );

util.assert( "strictEqual", handles.listeners( eventName ).length, 0,
	"No global presence listeners present when last 'devicelost' listener is removed" );

process.exit( 0 );

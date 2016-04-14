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

var _ = require( "lodash" );
var utils = require( "../../assert-to-console" );
var oic = require( "../../../index" )( "server" );
var uuid = process.argv[ 2 ];
var theError = null;
var defaultPlatformInfo = {
		osVersion: "osVersion " + uuid,
		model: "model " + uuid,
		manufacturerName: "manufacturerName " + uuid,
		manufacturerUrl: "manufacturerUrl:" + uuid + "/something/very/long/to/make/sure/it/breaks",
		manufactureDate: new Date( 1299752880000 ),
		platformVersion: "platformVersion " + uuid,
		firmwareVersion: "firmwareVersion " + uuid,
		supportUrl: "supportUrl:" + uuid
	};
var theResource;

console.log( JSON.stringify( { assertionCount: 4 } ) );

try {
	oic.device = _.extend( {}, oic.device, {
		name: "API Device Info Server " + uuid
	} );
} catch ( anError ) {
	theError = anError;
}
utils.assert( "deepEqual",
	theError ? {
		message: theError.message,
		result: theError.result
	} : null,
	null,
	"Server: Setting device info succeeded" );

theError = null;
try {
	oic.platform = _.extend( {}, oic.platform, defaultPlatformInfo );
} catch ( anError ) {
	theError = anError;
}
utils.assert( "strictEqual", theError.message.substr( 0, 40 ),
	"manufacturer name length must not exceed",
		"Server: Setting the platform info failed because of the manufacturer name's length" );

defaultPlatformInfo.manufacturerName = "Random";

theError = null;
try {
	oic.platform = _.extend( {}, oic.platform, defaultPlatformInfo );
} catch ( anError ) {
	theError = anError;
}
utils.assert( "strictEqual", theError.message.substr( 0, 39 ),
	"manufacturer URL length must not exceed",
	"Server: Setting the platform info failed because of the manufacturer url's length" );

defaultPlatformInfo.manufacturerUrl = "http://example.com/";

theError = null;
try {
	oic.platform = _.extend( {}, oic.platform, defaultPlatformInfo );
} catch ( anError ) {
	theError = anError;
}
utils.assert( "strictEqual", theError ? {
		message: theError.message,
		result: theError.result
	} : null,
	null,
	"Server: Setting the platform info succeeded" );

oic.registerResource( {
	id: { path: "/a/" + uuid },
	resourceTypes: [ "core.light" ],
	interfaces: [ "oic.if.baseline" ],
	discoverable: true
} ).then(
	function( resource ) {
		theResource = resource;
		console.log( JSON.stringify( { ready: true } ) );
	},
	function( error ) {
		utils.die( "Server: Failed to register resource: " +
			error.message + ", result: " + error.result );
	} );

process.on( "SIGINT", function() {
	oic.unregisterResource( theResource ).then(
		function() {
			process.exit( 0 );
		},
		function( error ) {
			utils.die( "Server: Failed to unregister resource: " +
				error.message + ", result: " + error.result );
		} );
} );

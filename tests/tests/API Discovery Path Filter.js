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

// This is a unit test for the discovery path filter

var _ = require( "lodash" ),
	utils = require( "../assert-to-console" ),
	device = require( "../../index" )( "client" ),
	reportedResources = [];

console.log( JSON.stringify( { assertionCount: 2 } ) );

device.addEventListener( "resourcefound", function( event ) {
	reportedResources.push( event.resource );
} );

// Mock out _addDevice()
device._addDevice = function() {};

device._processResources( {
	devAddr: {},
	addr: {},
	payload: {
		type: 1,
		sid: _.fill( new Array( 16 ), 0 ),
		resources: [
			{
				uri: "/a/xyzzy",
				types: [ "core.light" ],
				interfaces: [ "oic.if.baseline" ],
				bitmap: 3,
				secure: false,
				port: 0
			},
			{
				uri: "/b/xyzzy",
				types: [ "core.light" ],
				interfaces: [ "oic.if.baseline" ],
				bitmap: 3,
				secure: false,
				port: 0
			}
		]
	},
	connType: 0,
	result: 0,
	sequenceNumber: 0,
	resourceUri: "/oic/res"
}, "/a/xyzzy" );

utils.assert( "strictEqual", reportedResources.length, 1,
	"Only one of two resources is reported" );

utils.assert( "strictEqual", reportedResources[ 0 ].id.path, "/a/xyzzy",
	"The reported resource has the correct path" );

process.exit( 0 );

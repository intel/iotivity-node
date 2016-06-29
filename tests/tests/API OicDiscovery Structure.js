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

var utils = require( "../assert-to-console" ),
	device = require( "../../index" )( "client" );

console.log( JSON.stringify( { assertionCount: 11 } ) );

utils.assertProperties( "OicDiscovery", device, [
	{ name: "findResources", type: "function" },
	{ name: "getDeviceInfo", type: "function" },
	{ name: "getPlatformInfo", type: "function" },
	{ name: "findDevices", type: "function" },
	{ name: "onresourcefound" },
	{ name: "ondevicefound" },
	{ name: "ondiscoveryerror" }
] );

process.exit( 0 );

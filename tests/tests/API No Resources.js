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

var device = require( "../../index" )(),
	utils = require( "../assert-to-console" ),
	iotivity = require( "../../lowlevel" );

console.log( JSON.stringify( { assertionCount: 1 } ) );

device.configure( {
	role: "client"
} ).then(
	function() {
		utils.assert( "strictEqual",
			device._processResources( {
				result: iotivity.OCStackResult.OC_STACK_NO_RESOURCE
			} ),
			undefined,
			"_processResources() handles OC_STACK_NO_RESOURCE" );
		process.exit( 0 );
	},
	function() {
		utils.die( "device.configure() failed" );
	} );

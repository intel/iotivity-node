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
	dlopennow = require( "bindings" )( "dlopennow" ).dlopennow,
	exec = require( "child_process" ).exec;

// Report assertion count
console.log( JSON.stringify( { assertionCount: 1 } ) );

exec( "node -e 'console.log( JSON.stringify( {" +
"path: require( \"bindings\" )( \"iotivity\" ).path " +
"} ) );'",
	function( error, stdout ) {
	if ( error !== null ) {
		utils.die( "Failed to load iotivity in child process: " + error );
	}
	var filename = JSON.parse( stdout.toString() ).path,
		result = dlopennow( filename );

	utils.assert( "strictEqual", result, undefined, "dlopennow() of iotivity was successful" );
	} );

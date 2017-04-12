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

var utils = require( "../assert-to-console" );

console.log( JSON.stringify( { assertionCount: 4 } ) );

var stderr = "";
var stdout = "";

var childArguments = [ "-e",
		"try { require( '" + process.argv[ 3 ].replace( /[\\]/g, "\\\\" ) + "' ); } " +
		"catch( anError ) { " +
			"console.error( anError.stack ); process.exit( 1 ); " +
		"}" +
		"process.exit( 0 );"
	];

var theChild = require( "child_process" )
	.spawn( "node", [ "--napi-modules" ].concat( childArguments ) )
	.on( "exit", function( code, signal ) {
		utils.assert( "strictEqual", code, 0, "require() from interactive shell succeeded" );
		utils.assert( "strictEqual", signal, null,
			"require() from interactive shell did not receive a signal" );
		utils.assert( "strictEqual", stdout, "", "Process stdout is empty" );
		utils.assert( "strictEqual", stderr, "", "Process stderr is empty" );
	} );

theChild.stdout.on( "data", function( data ) {
	stdout += data.toString();
} );
theChild.stderr.on( "data", function( data ) {
	stderr += data.toString();
} );

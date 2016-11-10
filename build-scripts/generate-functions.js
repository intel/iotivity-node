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
var fs = require( "fs" );
var path = require( "path" );
var repoPaths = require( "./helpers/repo-paths" );

function getMethods( topPath ) {
	return _( fs.readdirSync( topPath ) )
		.map( function( item ) {
			item = path.join( topPath, item );
			return fs.statSync( item ).isDirectory() ?
				getMethods( item ) :
				_( fs.readFileSync( item, { encoding: "utf8" } )
					.replace( /\r/g, "" )
					.split( "\n" ) )
					.map( function( line ) {
						var match = line.match( /^NAN_METHOD\s*[(]\s*bind_([^)]*)*/ );
						if ( match && match.length > 1 ) {
							return match[ 1 ].replace( /\s/g, "" );
						}
					} )
					.without( undefined )
					.value();
		} )
		.flattenDeep()
		.uniq()
		.value();
}

var methods;
var functionsCC = path.join( repoPaths.generated, "functions.cc" );
var protosH = path.join( repoPaths.generated, "function-prototypes.h" );

methods = getMethods( repoPaths.src );

fs.writeFileSync( protosH, [
	"#ifndef __IOTIVITY_NODE_FUNCTION_PROTOTYPES_H__",
	"#define __IOTIVITY_NODE_FUNCTION_PROTOTYPES_H__",
	"",
	"#include <nan.h>",
	""
]
.concat( _.map( methods, function( item ) {
	return "NAN_METHOD(bind_" + item + ");";
} ) )
.concat( [
	"",
	"#endif /* ndef __IOTIVITY_NODE_FUNCTION_PROTOTYPES_H__ */"
] )
.join( "\n" ) );

fs.writeFileSync( functionsCC, [
	"#include \"../src/functions.h\"",
	"#include \"function-prototypes.h\"",
	"",
	"NAN_MODULE_INIT(InitFunctions) {"
]
.concat( _.map( methods, function( item ) {
	return "  SET_FUNCTION(target, " + item + ");";
} ) )
.concat( [
	"}\n"
] )
.join( "\n" ) );

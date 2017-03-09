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

var _ = {
	map: require( "lodash.map" ),
	flattenDeep: require( "lodash.flattendeep" ),
	uniq: require( "lodash.uniq" ),
	without: require( "lodash.without" )
};
var fs = require( "fs" );
var path = require( "path" );
var repoPaths = require( "./helpers/repo-paths" );

function getMethods( topPath ) {
	return _.uniq(
		_.flattenDeep(
			_.map(
				fs.readdirSync( topPath ),
				function( item ) {
					item = path.join( topPath, item );
					return fs.statSync( item ).isDirectory() ?
						getMethods( item ) :
						_.without(
							_.map(
								fs.readFileSync( item, { encoding: "utf8" } )
									.replace( /\r/g, "" )
									.split( "\n" ),
								function( line ) {
									var match = line.match( /^void\s+bind_([^(]+)/ );
									if ( match && match.length > 1 ) {
										return match[ 1 ].replace( /\s/g, "" );
									}
								} ),
							undefined );
				} ) ) );
}

var methods;
var functionsCC = path.join( repoPaths.generated, "functions.cc" );
var protosH = path.join( repoPaths.generated, "function-prototypes.h" );

methods = getMethods( repoPaths.src );

fs.writeFileSync( protosH, [
	"#ifndef __IOTIVITY_NODE_FUNCTION_PROTOTYPES_H__",
	"#define __IOTIVITY_NODE_FUNCTION_PROTOTYPES_H__",
	""
]
.concat( _.map( methods, function( item ) {
	return "void bind_" + item + "(napi_env env, napi_callback_info info);";
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
	"std::string InitFunctions(napi_env env, napi_value exports) {"
]
.concat( _.map( methods, function( item ) {
	return "  SET_FUNCTION(env, exports, " + item + ");";
} ) )
.concat( [
	"  return std::string();",
	"}\n"
] )
.join( "\n" ) );

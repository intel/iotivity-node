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
	uniq: require( "lodash.uniq" ),
	without: require( "lodash.without" )
};
var path = require( "path" );
var fs = require( "fs" );
var includePaths = require( "./helpers/header-paths" );
var repoPaths = require( "./helpers/repo-paths" );

function parseFileForConstants( fileName ) {
	return _.map(
		_.without(
			_.uniq(
				_.map(
					fs.readFileSync( fileName, { encoding: "utf8" } )
						.replace( /\r/g, "" )
						.split( "\n" ),
					function( line ) {
						var fields;

						if ( !line.match( /^#define/ ) ) {
							return;
						}

						// Do what awk does - split into tokens by whitespace
						fields = line.match( /\S+/g );
						if ( fields.length > 2 && !fields[ 1 ].match( /[()]/ ) &&

								// Constants we don't want
								fields[ 1 ] !== "OC_RSRVD_DEVICE_TTL" ) {
							return "SET_CONSTANT_" +
								( fields[ 2 ][ 0 ] === "\"" ? "STRING" : "NUMBER" ) +
								" " + fields[ 1 ];
						}
					} )
				),
			undefined ),
		function( line ) {
			var fields = line.split( " " );
			return [
				"#ifdef " + fields[ 1 ],
				"  " + fields[ 0 ] + "(target, " + fields[ 1 ] + ");",
				"#endif /* def " + fields[ 1 ] + " */"
			].join( "\n" ) + "\n";
		} ).join( "\n" ) + "\n";
}

var constantsCC = path.join( repoPaths.generated, "constants.cc" );

fs.writeFileSync( constantsCC, "" );

fs.writeFileSync( constantsCC,
	fs.readFileSync( path.join( repoPaths.src, "constants.cc.in" ), { encoding: "utf8" } ),
	{ flag: "a" } );

fs.writeFileSync( constantsCC,
	"NAN_MODULE_INIT(InitConstants) {\n",
	{ flag: "a" } );

fs.writeFileSync( constantsCC,
	"  // ocstackconfig.h: Stack configuration\n",
	{ flag: "a" } );

fs.writeFileSync( constantsCC,
	parseFileForConstants( includePaths[ "ocstackconfig.h" ] ),
	{ flag: "a" } );

fs.writeFileSync( constantsCC, "\n", { flag: "a" } );

fs.writeFileSync( constantsCC,
	"  // octypes.h: Definitions\n",
	{ flag: "a" } );

fs.writeFileSync( constantsCC,
	parseFileForConstants( includePaths[ "octypes.h" ] ),
	{ flag: "a" } );

fs.writeFileSync( constantsCC, "}\n", { flag: "a" } );

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

var fs = require( "fs" );
var path = require( "path" );
var includePaths = require( "./helpers/header-paths" );
var repoPaths = require( "./helpers/repo-paths" );

// We don't generate bindings for these enums because they're not defined in our use case.
var exceptions = [ "OCConnectUserPref_t" ];

var startingBraceRegex = /^}\s*/;
var closingBraceRegex = /^}/;

function parseFileForEnums( destination, fileName ) {
	var print = false;
	var output = "";
	var enumList = [];
	fs.readFileSync( fileName, { encoding: "utf8" } )
		.replace( /\r/g, "" )
		.split( "\n" )
		.forEach( function( line ) {
			var fields, enumName;

			if ( line === "typedef enum" ) {
				print = true;
			}
			if ( print ) {
				fields = line.match( /\S+/g );
				if ( !fields ) {
					return;
				}

				if ( !fields[ 0 ].match( /^[{}]/ ) && fields[ 0 ] !== "typedef" ) {
					if ( fields[ 0 ].match( /^[A-Z]/ ) ) {
						fields[ 0 ] = fields[ 0 ].replace( /,/g, "" );
						output += "  SET_CONSTANT_NUMBER(returnValue, " + fields[ 0 ] + ");\n";
					} else if ( fields[ 0 ].match( /^#(if|endif)/ ) ) {
						output += line + "\n";
					}
				} else if ( fields[ 0 ].match( closingBraceRegex ) ) {
					enumName = line
						.replace( startingBraceRegex, "" )
						.replace( /\s*;.*$/, "" );
					if ( exceptions.indexOf( enumName ) < 0 ) {
						enumList.push( "  SET_ENUM(target, " + enumName + ");" );
						fs.writeFileSync( destination,
							[
								"static Local<Object> bind_" + enumName + "() {",
								"  Local<Object> returnValue = Nan::New<Object>();"
							].join( "\n" ) + "\n",
							{ flag: "a" } );
					}
				} else if ( fields[ 0 ] !== "typedef" && fields[ 0 ] !== "{" ) {
					if ( exceptions.indexOf( enumName ) < 0 ) {
						fs.writeFileSync( destination, line, { flag: "a" } );
					}
				}
				if ( line.match( /;$/ ) ) {
					print = false;
					if ( exceptions.indexOf( enumName ) < 0 ) {
						fs.writeFileSync( destination, output + "\n  return returnValue;\n}\n",
							{ flag: "a" } );
					}
					output = "";
				}
			}
		} );
	return enumList.join( "\n" );
}

var enumsCC = path.join( repoPaths.generated, "enums.cc" );

fs.writeFileSync( enumsCC, "" );

fs.writeFileSync( enumsCC,
	fs.readFileSync( path.join( repoPaths.src, "enums.cc.in" ), { encoding: "utf8" } ),
	{ flag: "a" } );

fs.writeFileSync( enumsCC, [
		"NAN_MODULE_INIT(InitEnums) {",
		parseFileForEnums( enumsCC, includePaths[ "octypes.h" ] ),
		parseFileForEnums( enumsCC, includePaths[ "ocpresence.h" ] ),
		"}"
	].join( "\n" ) + "\n",
	{ flag: "a" } );

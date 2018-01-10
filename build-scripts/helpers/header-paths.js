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

var index, fileIndex, completeFileName;

var filesToFind = [ "octypes.h", "ocpresence.h", "ocstackconfig.h", "rd_client.h" ];

var paths = {};

var isIncludePathRegex = /^-I\s*/;

// indices 0 and 1 are "node" resp. this file's name. 2 may be "-c" to indeicate that the rest of
// the arguments are CFLAGS, not include paths.

var cflags = ( process.argv[ 2 ] === "-c" );

for ( fileIndex in filesToFind ) {
	completeFileName = "";
	for ( index = ( cflags ? 3 : 2 ); index < process.argv.length; index++ ) {
		if ( cflags && !process.argv[ index ].match( isIncludePathRegex ) ) {
			continue;
		}
		completeFileName = path.join( ( cflags ?
			process.argv[ index ].replace( isIncludePathRegex, "" ) :
			process.argv[ index ] ), filesToFind[ fileIndex ] );
		if ( fs.existsSync( completeFileName ) ) {
			break;
		}
		completeFileName = "";
	}
	if ( completeFileName ) {
		paths[ filesToFind[ fileIndex ] ] = completeFileName;
	} else {
		throw new Error( "Failed to find file " + filesToFind[ fileIndex ] );
	}
}

module.exports = paths;

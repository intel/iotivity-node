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

// Configuration directory
//
// Creates a directory structure of the following form:
// ${HOME}/.iotivity-node
//    |
//    +-- <per-app-path-0>
//    |
//    +-- <per-app-path-1>
//    |
//   ...
//    |
//    +-- <per-app-path-n>
// <per-app-path-n> is the sha256sum of the absolute filename of the main Node.js module.

var path = require( "path" );
var sha = require( "sha.js" );
var osenv = require( "osenv" );
var shelljs = require( "shelljs" );

function configurationDirectory( mainPath ) {
	var myDirectory = path.join( osenv.home(), ".iotivity-node",

		// Absent a path that was passed in as mainPath, we hash the absolute path to the top-level
		// script to create the directory where we store the files for this instance. If no top
		// level script is found, we construct a path using the current working directory and the
		// process id.
		sha( "sha256" )
			.update( mainPath ||
				require.main && require.main.filename ||
				path.join( process.cwd(), ( "" + process.pid ) ), "utf8" )
			.digest( "hex" ) );

	shelljs.mkdir( "-p", myDirectory );

	return myDirectory;
}

module.exports = configurationDirectory;

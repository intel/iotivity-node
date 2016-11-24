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

var rootPath = path.resolve( path.join( __dirname, "..", ".." ) );
var installPrefix = path.join( rootPath, "iotivity-installed" );

var paths = {
	root: rootPath,
	generated: path.join( rootPath, "generated" ),
	src: path.join( rootPath, "src" ),
	iotivity: path.join( rootPath, "iotivity-native" ),
	installPrefix: installPrefix,
	patchesPath: path.join( rootPath, "patches" ),
	installLibraries: path.join( installPrefix, "lib" ),
	installHeaders: path.join( installPrefix, "include" )
};

try {
	fs.mkdirSync( paths.generated );
} catch ( anError ) {
	if ( anError.code !== "EEXIST" ) {
		throw anError;
	}
};

module.exports = paths;

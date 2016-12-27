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

// Persistent storage handler
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
// <per-app-path-n> is constructed from process.argv[1] by replacing '/' and ':' characters with
// underscores.
//
// File names passed to open have no path component, so they will be created under
// ${HOME}/.iotivity-node/<per-app-path-n>

var _ = require( "lodash" ),
	fs = require( "fs" ),
	path = require( "path" ),
	configurationDirectory = require( "./configurationDirectory" ),
	StorageHandler = function StorageHandler() {
		if ( !( this instanceof StorageHandler ) ) {
			return new StorageHandler();
		}
		this._directory = configurationDirectory();
	};

_.extend( StorageHandler.prototype, {
	open: function( filename, mode ) {
		var fd,
			absolutePath = path.join( this._directory, filename );

		mode = mode.replace( /b/g, "" );

		// If the requested file does not exist, create it
		try {
			fd = fs.openSync( absolutePath, "wx" );
			fs.closeSync( fd );
		} catch ( theError ) {
			if ( theError.message.substr( 0, 6 ) !== "EEXIST" ) {
				throw theError;
			}
		}

		// Open the file in the requested mode
		fd = fs.openSync( absolutePath, mode );
		fd = ( fd === undefined ? -1 : fd );
		return fd;
	},
	close: function( fp ) {
		fs.closeSync( fp );
		return 0;
	},
	read: function( buffer, totalSize, fp ) {
		return fs.readSync( fp, buffer, 0, totalSize, null );
	},
	write: function( buffer, totalSize, fp ) {
		return fs.writeSync( fp, buffer, 0, totalSize, null );
	},
	unlink: function( path ) {
		fs.unlinkSync( path );
		return 0;
	}
} );

module.exports = StorageHandler;

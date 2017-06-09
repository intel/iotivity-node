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

module.exports = function( grunt ) {

var path = require( "path" );
var fs = require( "fs" );
var _ = {
	extend: require( "lodash.assignin" ),
	zipObject: require( "lodash.zipobject" )
};

var allowedStructure = {
	"build-scripts": {
		"postinstall.js": "required",
		"helpers": {
			"repo-paths.js": "required"
		}
	},
	"iotivity-installed": {
		"lib": "required"
	},
	"CONTRIBUTING.md": "required",
	"AUTHORS.txt": "required",
	"build": "required",
	"lowlevel.js": "required",
	"package.json": "required",
	"node_modules": {
		"inherits": "optional",
		"os-homedir": "optional",
		"os-tmpdir": "optional",
		"bindings": "required",
		"lodash.assignin": "required",
		"lodash.bind": "required",
		"lodash.bindkey": "required",
		"lodash.find": "required",
		"lodash.findindex": "required",
		"lodash.findkey": "required",
		"lodash.flattendeep": "required",
		"lodash.foreach": "required",
		"lodash.isequal": "required",
		"lodash.isnil": "required",
		"lodash.map": "required",
		"lodash.mergewith": "required",
		"lodash.omitby": "required",
		"lodash.pick": "required",
		"lodash.remove": "required",
		"lodash.uniq": "required",
		"lodash.without": "required",
		"osenv": "required",
		"sha.js": "required",
		"shelljs": "required"
	},
	"js": "required",
	"lib": "required",
	"README.md": "required",
	"index.js": "required"
};

// Return true on success, otherwise an object containing the parameters of the failure.
function checkStructure( structure, topPath )  {
	var index, result, local, localKeys;
	var actual = fs.readdirSync( topPath );
	var actualHash = _.zipObject( actual );
	var results = [];

	// If structure is an array, then each member represents a possible layout. Only one is
	// required to match. In other words, or-semantics.
	if ( Array.isArray( structure ) ) {
		for ( index in structure ) {
			result = checkStructure( structure[ index ], topPath );
			if ( result === true ) {
				return true;
			}
			results.push( result );
		}
		return {
			structure: structure,
			topPath: topPath,
			actual: Object.keys( actual ).sort(),
			results: results
		};
	}

	local = _.extend( {}, structure );

	// Go over the files found and make sure they conform. If they do, remove them from
	// consideration.
	for ( index in actual ) {
		if ( actual[ index ] in local ) {
			if ( typeof local[ actual[ index ] ] === "object" ) {
				result = checkStructure( local[ actual[ index ] ],
					path.join( topPath, actual[ index ] ) );
				if ( result !== true ) {
					return result;
				}
			}
			delete local[ actual[ index ] ];
			delete actualHash[ actual[ index ] ];
		}
	}

	// Remove any files that were not found but were marked as optional.
	localKeys = Object.keys( local );
	for ( index in localKeys ) {
		if ( local[ localKeys[ index ] ] === "optional" ) {
			delete local[ localKeys[ index ] ];
		}
	}

	// If anything is left, bail with an error.
	localKeys = Object.keys( local );
	actual = Object.keys( actualHash );
	return ( localKeys.length === 0 && actual.length === 0 ? true : {
		structure: structure,
		topPath: topPath,
		leftOver: {
			desired: localKeys,
			actual: actual
		}
	} );
}

grunt.registerTask( "testdist", "Check global iotivity-node installed structure", function() {
	var result = checkStructure( allowedStructure,
		path.dirname( require.resolve( "iotivity-node" ) ) );

	if ( typeof result === "object" ) {
		grunt.verbose.or
			.error()
			.writeln( "Installed package has unexpected directory structure" )
			.error( JSON.stringify( result, null, 4 ) );

		return false;
	}
} );

};

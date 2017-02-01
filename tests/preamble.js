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

// Create the configuration database that will allow the server to connect to the client.

module.exports = function( testFile, resourceUuid, location, noClobber ) {

// Create two ACEs from the given resources - one for plain text and one for encrypted
function generateACEList( resources ) {
	return [
		{ subject: { "conntype": "anon-clear" }, resources: resources, "permission": 31 },
		{ subject: { "conntype": "auth-crypt" }, resources: resources, "permission": 31 }
	];
}

var _ = {
	mergeWith: require( "lodash.mergewith" ),
	extend: require( "lodash.assignin" )
};
var fs = require( "fs" );
var path = require( "path" );
var uuid = require( "uuid" );
var spawnSync = require( "child_process" ).spawnSync;
var configurationDirectory = require( "../lib/configurationDirectory" );

var configuration, toolResult, toolPath, configPath;

var installPrefix = path.join( location, "iotivity-installed" );
var deviceUuid = uuid.v4();

if ( testFile ) {
	testFile = path.normalize( testFile );
}

configPath = configurationDirectory.apply( configurationDirectory, testFile ? [ testFile ] : [] );

if ( fs.existsSync( path.join( configPath, "oic_svr_db.dat" ) ) && noClobber ) {
	return;
}

// Load the per-test configuration (if any) and add a permissive acl for the path ("/a/" + uuid)
configuration = _.mergeWith( {},

	// The boilerplate
	require( "./security.boilerplate.json" ),

	// Special href containing the resource uuid
	{
		acl: {
			aclist2: generateACEList( typeof resourceUuid === "string" ? [

				// Resource names used during testing
				// 1. iotivity-node
				{
					href: "/a/" + resourceUuid + "-xyzzy"
				},

				// 2. iot-js-api
				{
					href: "/a/" + resourceUuid
				},
				{
					href: "/direct"
				},
				{
					href: "/target-resource"
				},
				{
					href: "/disable-presence"
				},
				{
					href: "/some/new/resource"
				}
			] : resourceUuid )
		}
	},

	// Per-test configuration (if any)
	( function() {
		var configuration = {};

		if ( testFile ) {
			try {
				configuration = require( path.join( path.dirname( testFile ), "security.json" ) );
			} catch ( anError ) {}
		}
		return configuration;
	} )(),

	// The device ID
	{
		pstat: { deviceuuid: deviceUuid, rowneruuid: deviceUuid },
		acl: { rowneruuid: deviceUuid },
		doxm: { deviceuuid: deviceUuid, deviceuuid: deviceUuid }
	},

	// Function to merge aces
	function( objectValue, sourceValue ) {
		if ( Array.isArray( objectValue ) && Array.isArray( sourceValue ) ) {
			return objectValue.concat( sourceValue );
		}
	} );

configuration.acl.aclist2 = configuration.acl.aclist2.map( function( value, index ) {
	return _.extend( value, { aceid: index + 1 } );
} );

toolPath = path.join( installPrefix, "bin" );

fs.writeFileSync( path.join( configPath, "oic_svr_db.json" ),
	JSON.stringify( configuration, null, "\t" ) );

toolResult = spawnSync( "json2cbor", [
	path.join( configPath, "oic_svr_db.json" ),
	path.join( configPath, "oic_svr_db.dat" ) ], {
		env: _.extend( {}, process.env, {
			PATH: ( process.env.PATH || "" )
				.split( path.delimiter )
				.concat( [ toolPath ] )
				.join( path.delimiter ),
			LD_LIBRARY_PATH: path.join( installPrefix, "lib" ),
			stdio: [ process.stdin, process.stdout, process.stderr ]
		} )
	} );

if ( toolResult.status !== 0 ) {
	console.log( JSON.stringify( { info: true, message:
		"Warning: Failed to run json2cbor: " + toolResult.stderr
	} ) );
}

};

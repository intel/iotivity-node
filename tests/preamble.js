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

module.exports = function( resourceUuid ) {

var _ = require( "lodash" );
var osenv = require( "osenv" );
var fs = require( "fs" );
var shelljs = require( "shelljs" );
var path = require( "path" );
var sha = require( "sha.js" );
var uuid = require( "uuid" );
var spawnSync = require( "child_process" ).spawnSync;
var binariesPath = path.join( require( "bindings" ).getRoot( __filename ),
	"iotivity-native", "binaries" );
var json2cborPath = path.join( binariesPath, "json2cbor" );
var configPath = path.join( osenv.home(), ".iotivity-node",
	sha( "sha256" )
		.update( require.main && require.main.filename ||
			path.join( process.cwd(), ( "" + process.pid ) ), "utf8" )
		.digest( "hex" ) );
var deviceUuid = uuid.v4();

// Load the per-test configuration (if any) and add a permissive acl for the path ("/a/" + uuid)
var configuration = _.mergeWith( {},

	// The boilerplate
	require( "./security.boilerplate.json" ),

	// Special href containing the resource uuid
	{
		acl: { aclist: { aces: [
			{
				subjectuuid: "*",
				permission: 31,
				resources: [

					// Resource names used during testing
					// 1. iotivity-node
					{
						href: "/a/" + resourceUuid + "-xyzzy",
						rel: "",
						rt: [ "core.light" ],
						"if": [ "oic.if.baseline" ]
					},

					// 2. iot-js-api
					{
						href: "/a/" + resourceUuid,
						rel: "",
						rt: [ "core.light" ],
						"if": [ "oic.if.baseline" ]
					},
					{
						href: "/direct",
						rel: "",
						rt: [ "core.light" ],
						"if": [ "oic.if.baseline" ]
					},
					{
						href: "/target-resource",
						rel: "",
						rt: [ "core.light" ],
						"if": [ "oic.if.baseline" ]
					},
					{
						href: "/disable-presence",
						rel: "",
						rt: [ "core.light" ],
						"if": [ "oic.if.baseline" ]
					},
					{
						href: "/some-new-resource",
						rel: "",
						rt: [ "core.light" ],
						"if": [ "oic.if.baseline" ]
					}
				]
			}
		] } }
	},

	// Per-test configuration (if any)
	( function() {
		var perTestConfiguration;
		var thePath = path.join( path.dirname( module.parent.filename ), "security.json" );

		try {
			perTestConfiguration =
				require( thePath );
		} catch ( anError ) {
			perTestConfiguration = {};
		}
		return perTestConfiguration;
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

shelljs.mkdir( "-p", configPath );

fs.writeFileSync( path.join( configPath, "oic_svr_db.json" ),
	JSON.stringify( configuration, null, "\t" ) );

spawnSync( json2cborPath, [
	path.join( configPath, "oic_svr_db.json" ),
	path.join( configPath, "oic_svr_db.dat" ) ], {
		env: _.extend( {}, process.env, {
			LD_LIBRARY_PATH: binariesPath,
			stdio: [ process.stdin, process.stdout, process.stderr ]
		} )
	} );

};

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
var uuid = require( "uuid" );
var configurationDirectory = require( "../../lib/configurationDirectory" );
var produceClientContent = require( "./produceClientContent" );
var produceServerContent = require( "./produceServerContent" );
var spawnSync = require( "child_process" ).spawnSync;
var assignin = require( "lodash.assignin" );

function produceDataItem( fileName ) {
	return {
		fileName: fileName,
		deviceUuid: uuid.v4(),
		configPath: configurationDirectory( fileName )
	};
}

function clientCred( item, index ) {
	return {
		credid: index + 1,
		subjectuuid: item.deviceUuid,
		credtype: 1,
		privatedata: {
			data: "AAAAAAAAAAAAAAAA",
			encoding: "oic.sec.encoding.raw"
		}
	};
}

function serverCred( item, index ) {
	return assignin( clientCred( item, index ), {
		period: "20150630T060000/20990920T220000"
	} );
}

// Create provisioning information for a set of clients and a set of servers such
// that any of the clients may connect to any of the servers.
// options:
//   location: string - absolute path to iotivity-node package root
//   clientPaths: array of strings - absolute paths of clients
//   serverPaths: array of strings - absolute paths of servers
//   uuid: - string - UUID that will be used for the recognition resource
//         - array of objects - entries to add to the ACL of all servers
module.exports = function provision( options ) {
	var index;

	// Establish paths for json2cbor and its libraries.
	var toolPath = path.join( options.location, "iotivity-installed", "bin" );
	var libPath = path.join( options.location, "iotivity-installed", "lib" );

	// Generate device UUIDs and configuration paths.
	var clientData = options.clientPaths.map( produceDataItem );
	var serverData = options.serverPaths.map( produceDataItem );

	// Cross-link server and client credentials. This means any client can access any resource on
	// any server.
	for ( index in clientData ) {
		clientData[ index ].fileContents =
			produceClientContent( clientData[ index ], serverData.map( clientCred ) );
	}
	for ( index in serverData ) {
		serverData[ index ].fileContents =
			produceServerContent( serverData[ index ], clientData.map( serverCred ), options.uuid );
	}

	// Write out the .json and .dat files
	clientData.concat( serverData ).map( function( item ) {

		// The .json file is not used but it's good to have it side-by-side with the .dat file.
		fs.writeFileSync( path.join( item.configPath, "oic_svr_db.json" ),
			JSON.stringify( item.fileContents, null, "\t" ) );

		// json2cbor creates the .dat file. We let node search the PATH, after having added the
		// iotivity-node path where we know json2cbor should be. So, if it's installed globally
		// then node will find it there (like, /usr/bin or something like that), otherwise it will
		// find it in the iotivity-node/iotivity-installed/bin directory.
		var toolResult = spawnSync( "json2cbor", [
			path.join( item.configPath, "oic_svr_db.json" ),
			path.join( item.configPath, "oic_svr_db.dat" ) ], {
				env: assignin( {}, process.env, {
					PATH: ( process.env.PATH || "" )
						.split( path.delimiter )
						.concat( [ toolPath ] )
						.join( path.delimiter ),
					LD_LIBRARY_PATH: libPath,
					stdio: [ process.stdin, process.stdout, process.stderr ]
				} )
			} );

		if ( toolResult.status !== 0 ) {
			console.log( JSON.stringify( { info: true, message:
				"Warning: Failed to run json2cbor: " + toolResult.stderr
			} ) );
		}

	} );
};

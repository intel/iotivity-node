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

var _ = require( "lodash" );
var path = require( "path" );
var spawnSync = require( "child_process" ).spawnSync;
var fs = require( "fs" );
var repoPaths = require( "./helpers/repo-paths" );
var shelljs = require( "shelljs" );

function run( command, arguments, options ) {
	var status;
	options = options || {};
	status = spawnSync( command, arguments, _.extend( {}, options, {
		stdio: [ process.stdin, process.stdout, process.stderr ],
		shell: true
	} ) ).status;

	if ( status !== 0 ) {
		process.exit( status );
	}
}

function findBins( iotivityPath ) {
	var index, entries;
	var thePath = path.join( iotivityPath, "out" );

	// Descend three levels: os, arch, and buildtype
	for ( index = 0; index < 3; index++ ) {
		entries = fs.readdirSync( thePath );
		if ( entries.length <= 0 ) {
			throw new Error( thePath + " is empty" );
		}
		thePath = path.join( thePath, entries[ 0 ] );
	}

	return thePath;
}

var binariesSource, installWinHeaders, architecture, tinycborPath;

// Map npm arch to iotivity arch - different mapping in each OS, it seems :/
// This can get complicated ...
var archMap = {
	"win32": { "x64": "amd64" },
	"linux": {
			"x64": "x86_64",
			"arm": "arm"
		 }
};

// Determine the CSDK revision
var csdkRevision = process.env.CSDK_REVISION ||
	require( path.join( repoPaths.root, "package.json" ) ).version
		.replace( /-[0-9]*$/, "" )
		.replace(  /^([^-]*-)pre-(.*$)/, "$2" );

// We assume that, if the path is there, its contents are also ready to go
if ( fs.existsSync( repoPaths.installPrefix ) ) {
	return;
}

// Establish paths needed by the iotivity source tree
tinycborPath = path.join( repoPaths.iotivity, "extlibs", "tinycbor", "tinycbor" );

// Attempt to determine the architecture
architecture = ( process.env.npm_config_user_agent || "" ).match( /\S+/g ) || [];
architecture =
	archMap[ architecture[ 2 ] ] ? archMap[ architecture[ 2 ] ][ architecture[ 3 ] ] : null;

// If the iotivity source tree is in place, we assume it's build and ready to go. This reduces this
// script to an install.
if ( !fs.existsSync( repoPaths.iotivity ) ) {

	// Do a shallow checkout of iotivity
	run( "git", [ "clone", "--depth", "1", "--single-branch", "--branch", csdkRevision,
		"https://gerrit.iotivity.org/gerrit/iotivity", repoPaths.iotivity ]  );

	// Apply patches
	if ( fs.existsSync( repoPaths.patchesPath ) &&
			fs.statSync( repoPaths.patchesPath ).isDirectory() ) {
		fs.readdirSync( repoPaths.patchesPath ).forEach( function( item ) {
			run( "git", [ "apply", path.join( repoPaths.patchesPath, item ) ],
				{ cwd: repoPaths.iotivity } );
		} );
	}

	// Clone tinycbor inside iotivity
	run( "git", [ "clone", "https://github.com/01org/tinycbor.git", tinycborPath ] );

	// Check out known-good commitid of tinycbor
	run( "git", [ "checkout", "31c7f81d45d115d2007b1c881cbbd3a19618465c" ],
		{ cwd: tinycborPath } );

	// Build
	run( "scons", []
		.concat( ( process.env.V === "1" || process.env.npm_config_verbose === "true" ) ?
			[ "VERBOSE=True" ] : [] )
		.concat( architecture ?
			[ "TARGET_ARCH=" + architecture ] : [] )
		.concat( process.env.npm_config_debug === "true" ?
			[ "RELEASE=False" ] : [] )
		.concat(
			[ "logger", "octbstack", "connectivity_abstraction", "coap", "c_common", "ocsrm",
				"routingmanager"
			] ), { cwd: repoPaths.iotivity } );
}

// Where are the binaries?
binariesSource = findBins( repoPaths.iotivity );

// Install the libraries
shelljs.mkdir( "-p", repoPaths.installLibraries );
shelljs.cp( "-r", path.join( binariesSource, "*" ), repoPaths.installLibraries );
shelljs.rm( "-rf", path.join( repoPaths.installLibraries, "extlibs" ) );
shelljs.rm( "-rf", path.join( repoPaths.installLibraries, "resource" ) );

// Install the headers
shelljs.mkdir( "-p", repoPaths.installHeaders );
shelljs.cp(
	path.join( repoPaths.iotivity, "resource", "csdk", "stack", "include", "ocpayload.h" ),
	path.join( repoPaths.iotivity, "resource", "csdk", "stack", "include", "ocpresence.h" ),
	path.join( repoPaths.iotivity, "resource", "csdk", "stack", "include", "ocstackconfig.h" ),
	path.join( repoPaths.iotivity, "resource", "csdk", "stack", "include", "ocstack.h" ),
	path.join( repoPaths.iotivity, "resource", "csdk", "stack", "include", "octypes.h" ),
	path.join( repoPaths.iotivity, "resource", "c_common", "iotivity_config.h" ),
	path.join( repoPaths.iotivity, "resource", "c_common", "platform_features.h" ),
	path.join( repoPaths.iotivity, "extlibs", "tinycbor", "tinycbor", "src", "cbor.h" ),
	repoPaths.installHeaders );

if ( process.platform.match( /^win/ ) ) {
	installWinHeaders = path.join( repoPaths.installHeaders, "windows" );
	shelljs.mkdir( "-p", installWinHeaders );
	shelljs.cp( "-r",
		path.join( repoPaths.iotivity, "resource", "c_common", "windows", "include" ),
		installWinHeaders );
}

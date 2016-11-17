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
	options = options || {};
	spawnSync( command, arguments, _.extend( {}, options, {
		stdio: [ process.stdin, process.stdout, process.stderr ],
		shell: true
	} ) );
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

var binariesSource, patchesPath, installBinaries, installLibraries, installHeaders,
	windowsInstallHeaders, architecture, iotivityPath, mbedtlsPath, tinycborPath;

// Map npm arch to iotivity arch - different mapping in each OS, it seems :/
// This can get complicated ...
var archMap = {
	"win32": { "x64": "amd64" },
	"linux": { "x64": "x86_64" }
};

// Determine the CSDK revision
var csdkRevision = process.env.CSDK_REVISION ||
	require( path.join( repoPaths.root, "package.json" ) ).version
		.replace( /-[0-9]*$/, "" )
		.replace(  /^([^-]*-)pre-(.*$)/, "$2" );

// Install
var installPrefix = path.join( repoPaths.root, "iotivity-installed" );

// We assume that, if the path is there, its contents are also ready to go
if ( fs.existsSync( installPrefix ) ) {
	return;
}

// Establish the path to the downstream iotivity patches
patchesPath = path.join( repoPaths.root, "patches" );

// Establish paths needed by the iotivity source tree
iotivityPath = path.join( repoPaths.root, "iotivity-native" );
mbedtlsPath = path.join( iotivityPath, "extlibs", "mbedtls", "mbedtls" );
tinycborPath = path.join( iotivityPath, "extlibs", "tinycbor", "tinycbor" );

// Establish paths needed by the installed version of iotivity
installBinaries = path.join( installPrefix, "bin" );
installLibraries = path.join( installPrefix, "lib" );
installHeaders = path.join( installPrefix, "include" );

// Attempt to determine the architecture
architecture = ( process.env.npm_config_user_agent || "" ).match( /\S+/g ) || [];
architecture =
	archMap[ architecture[ 2 ] ] ? archMap[ architecture[ 2 ] ][ architecture[ 3 ] ] : null;

// Do a shallow checkout of iotivity
run( "git", [ "clone", "--depth", "1", "--single-branch", "--branch", csdkRevision,
	"https://gerrit.iotivity.org/gerrit/iotivity", iotivityPath ]  );

// Apply patches
if ( fs.existsSync( patchesPath ) && fs.statSync( patchesPath ).isDirectory() ) {
	fs.readdirSync( patchesPath ).forEach( function( item ) {
		run( "git", [ "apply", path.join( patchesPath, item ) ], { cwd: iotivityPath } );
	} );
}

// Clone mbedtls inside iotivity
run( "git", [ "clone", "https://github.com/ARMmbed/mbedtls.git", mbedtlsPath ] );

// Check out known-good commitid of mbedtls
run( "git", [ "checkout", "ad249f509fd62a3bbea7ccd1fef605dbd482a7bd" ], { cwd: mbedtlsPath } );


// Clone tinycbor inside iotivity
run( "git", [ "clone", "https://github.com/01org/tinycbor.git", tinycborPath ] );

// Check out known-good commitid of tinycbor
run( "git", [ "checkout", "31c7f81d45d115d2007b1c881cbbd3a19618465c" ], { cwd: tinycborPath } );

// Build
run( "scons", [ "SECURED=1" ]
	.concat( ( process.env.V === "1" || process.env.npm_config_verbose === "true" ) ?
		[ "VERBOSE=True" ] : [] )
	.concat( architecture ?
		[ "TARGET_ARCH=" + architecture ] : [] )
	.concat( process.env.npm_config_debug === "true" ?
		[ "RELEASE=False" ] : [] )
	.concat(
		[ "logger", "octbstack", "connectivity_abstraction", "coap", "c_common", "ocsrm",
			"routingmanager", "json2cbor"
		] ), { cwd: iotivityPath } );

// Binaries
binariesSource = findBins( iotivityPath );

// json2cbor
shelljs.mkdir( "-p", installBinaries );
shelljs.mv(
	path.join( binariesSource, "resource", "csdk", "security", "tool",
		"json2cbor" + ( process.platform.match( /^win/ ) ? ".exe" : "" ) ),
	installBinaries );

// Libraries
shelljs.mv( binariesSource, installLibraries );
shelljs.rm( "-rf", path.join( installLibraries, "extlibs" ) );
shelljs.rm( "-rf", path.join( installLibraries, "resource" ) );

// Includes
shelljs.mkdir( "-p", installHeaders );
shelljs.mv(
	path.join( iotivityPath, "resource", "csdk", "stack", "include", "ocpayload.h" ),
	path.join( iotivityPath, "resource", "csdk", "stack", "include", "ocpresence.h" ),
	path.join( iotivityPath, "resource", "csdk", "stack", "include", "ocstackconfig.h" ),
	path.join( iotivityPath, "resource", "csdk", "stack", "include", "ocstack.h" ),
	path.join( iotivityPath, "resource", "csdk", "stack", "include", "octypes.h" ),
	path.join( iotivityPath, "resource", "c_common", "iotivity_config.h" ),
	path.join( iotivityPath, "resource", "c_common", "platform_features.h" ),
	path.join( iotivityPath, "extlibs", "tinycbor", "tinycbor", "src", "cbor.h" ),
	installHeaders );

if ( process.platform.match( /^win/ ) ) {
	windowsInstallHeaders = path.join( installHeaders, "windows" );
	shelljs.mkdir( "-p", windowsInstallHeaders );
	shelljs.mv(
		path.join( iotivityPath, "resource", "c_common", "windows", "include" ),
		windowsInstallHeaders );
}

// Remove of the source tree
shelljs.rm( "-rf", iotivityPath );

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

var assignIn = require( "lodash.assignin" );
var path = require( "path" );
var spawnSync = require( "child_process" ).spawnSync;
var fs = require( "fs" );
var repoPaths = require( "./helpers/repo-paths" );
var shelljs = require( "shelljs" );
var os = require( "os" );

function run( command, args, options ) {
	var result;
	options = options || {};
	result = spawnSync( command, args, assignIn( {}, options, {
		stdio: [ process.stdin, process.stdout, process.stderr ],
		shell: true
	} ) );

	if ( result.status !== 0 ) {
		process.exit( result.status || 1 );
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
		if ( entries[ 0 ] === "windows" ) {
			index--;
		}
	}

	return thePath;
}

// If the iotivity source tree is in place, we check for the specified
// revision and remove the source tree if it fails to find. This makes
// the script to install the correct version of iotivity always.
function checkRevision() {
	if ( !fs.existsSync( repoPaths.iotivity ) ) {
		return false;
	}

	var status = spawnSync( "git", [ "show", csdkRevision ],
		{ cwd: repoPaths.iotivity, shell: true } ).status;

	if ( status !== 0 ) {
		shelljs.rm( "-rf", path.join( repoPaths.generated ) );
		shelljs.rm( "-rf", path.join( repoPaths.installPrefix ) );
		shelljs.rm( "-rf", path.join( repoPaths.iotivity ) );
		return false;
	}

	return true;
}

var binariesSource, installWinHeaders, tinycborPath, userAgentParts, platform, localArch,
	targetArch, sysVersion, installBinaries, mbedtlsPath;

// Map npm arch to iotivity arch - different mapping in each OS, it seems :/
// This can get complicated ...
var archMap = {
	"win32": { "x64": "amd64" },
	"linux": {
			"x64": "x86_64",
			"arm": "arm"
	},
	"darwin": { "x64": "x86_64" }
};

// map of major version in os.release() to SYS_VERSION
var darwinMap = {
	"12": "10.8",
	"13": "10.9",
	"14": "10.10",
	"15": "10.11",
	"16": "10.12"
};

// Determine the CSDK revision
var csdkRevision = process.env.CSDK_REVISION ||
	require( path.join( repoPaths.root, "package.json" ) ).version
		.replace( /-[0-9]*$/, "" )
		.replace(  /^([^-]*-)pre-(.*$)/, "$2" );

// We assume that, if the path is there, its contents are also ready to go
if ( checkRevision() && fs.existsSync( repoPaths.installPrefix ) ) {
	return;
}

// Establish paths needed by the iotivity source tree
mbedtlsPath = path.join( repoPaths.iotivity, "extlibs", "mbedtls", "mbedtls" );
tinycborPath = path.join( repoPaths.iotivity, "extlibs", "tinycbor", "tinycbor" );

// Establish paths needed by the installed version of iotivity
installBinaries = path.join( repoPaths.installPrefix, "bin" );

// Attempt to determine the architecture
userAgentParts = ( process.env.npm_config_user_agent || "" ).match( /\S+/g ) || [];
platform = userAgentParts[ 2 ];
localArch = userAgentParts[ 3 ];

targetArch =
	archMap[ platform ] ? archMap[ platform ][ localArch ] : null;

if ( platform === "darwin" ) {
	var kernelVersionParts = os.release().split( "." );
	var majorVersion = kernelVersionParts[ 0 ];
	sysVersion = darwinMap[ majorVersion ] ? darwinMap[ majorVersion ] : null;
} else {
	sysVersion = null;
}

// If the iotivity source tree is in place, we assume it's build and ready to go. This reduces this
// script to an install.
if ( !fs.existsSync( repoPaths.iotivity ) ) {

	if ( csdkRevision.match( /[0-9a-f]{40}/ ) ) {

		// Do a full checkout when doing a commitid
		run( "git", [ "clone",
			"https://gerrit.iotivity.org/gerrit/iotivity", repoPaths.iotivity ] );
		run( "git", [ "checkout", csdkRevision ], { cwd: repoPaths.iotivity } );
	} else {

		// Do a shallow checkout of iotivity
		run( "git", [ "clone", "--depth", "1", "--single-branch", "--branch", csdkRevision,
			"https://gerrit.iotivity.org/gerrit/iotivity", repoPaths.iotivity ] );
	}

	// Apply patches
	if ( fs.existsSync( repoPaths.patchesPath ) &&
			fs.statSync( repoPaths.patchesPath ).isDirectory() ) {
		fs.readdirSync( repoPaths.patchesPath ).forEach( function( item ) {
			run( "git", [ "apply", path.join( repoPaths.patchesPath, item ) ],
				{ cwd: repoPaths.iotivity } );
		} );
	}

	// Clone mbedtls inside iotivity
	run( "git", [ "clone", "https://github.com/ARMmbed/mbedtls.git", mbedtlsPath ] );

	// Check out known-good commitid of mbedtls
	run( "git", [ "checkout", "ad249f509fd62a3bbea7ccd1fef605dbd482a7bd" ], { cwd: mbedtlsPath } );


	// Clone tinycbor inside iotivity
	run( "git", [ "clone", "https://github.com/01org/tinycbor.git", tinycborPath ] );

	// Check out known-good commitid of tinycbor
	run( "git", [ "checkout", "31c7f81d45d115d2007b1c881cbbd3a19618465c" ],
		{ cwd: tinycborPath } );

		// Build
	var sconsCommand = "scons";
	var sconsArguments = [
		"SECURED=1",
		"RD_MODE=all",
		"EXC_PROV_SUPPORT=1"
	]
	.concat( ( process.env.V === "1" || process.env.npm_config_verbose === "true" ) ?
		[ "VERBOSE=True" ] : [] )
	.concat( targetArch ?
		[ "TARGET_ARCH=" + targetArch ] : [] )
	.concat( sysVersion ?
		[ "SYS_VERSION=" + sysVersion ] : [] )
	.concat( process.env.npm_config_debug === "true" ?
		[ "RELEASE=False", "LOGGING=False" ] : [] )
	.concat( [ "logger", "octbstack", "connectivity_abstraction", "coap", "c_common", "ocsrm",
		"routingmanager", "resource/csdk/security/tool"
	] );

	// Node 5 on Windows seems unable to run batch files
	if ( +process.version.split( "." )[ 0 ].substr( 1, 1 ) === 5 &&
			process.platform === "win32" ) {
		sconsCommand = "cmd";
		sconsArguments = [ "/c", "scons" ].concat( sconsArguments );
	}

	run( sconsCommand, sconsArguments, { cwd: repoPaths.iotivity } );
}

// Where are the binaries?
binariesSource = findBins( repoPaths.iotivity );

// Install json2cbor
shelljs.mkdir( "-p", installBinaries );
shelljs.cp(
	path.join( binariesSource, "resource", "csdk", "security", "tool",
		"json2cbor" + ( process.platform.match( /^win/ ) ? ".exe" : "" ) ),
	installBinaries );

// Install the libraries
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
	path.join( repoPaths.iotivity, "resource", "csdk", "include", "octypes.h" ),
	path.join( repoPaths.iotivity, "resource", "csdk", "security", "include", "experimental",
		"securevirtualresourcetypes.h" ),
	path.join( repoPaths.iotivity, "resource", "c_common", "platform_features.h" ),
	path.join( repoPaths.iotivity, "extlibs", "tinycbor", "tinycbor", "src", "cbor.h" ),
	repoPaths.installHeaders );

shelljs.mkdir( "-p", path.join( repoPaths.installHeaders, "experimental" ) );
shelljs.cp(
	path.join( binariesSource, "resource", "c_common", "iotivity_config.h" ),
	path.join( repoPaths.iotivity, "resource", "c_common", "experimental", "byte_array.h" ),
	path.join( repoPaths.iotivity, "resource", "csdk", "security", "include", "experimental",
		"securevirtualresourcetypes.h" ),
	path.join( repoPaths.installHeaders, "experimental" ) );

if ( process.platform.match( /^win/ ) ) {
	installWinHeaders = path.join( repoPaths.installHeaders, "windows" );
	shelljs.mkdir( "-p", installWinHeaders );
	shelljs.cp( "-r",
		path.join( repoPaths.iotivity, "resource", "c_common", "windows", "include" ),
		installWinHeaders );
}

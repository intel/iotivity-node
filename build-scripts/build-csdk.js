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

var path = require( "path" );
var run = require( "child_process" ).spawn;
var fs = require( "fs" );
var repoPaths = require( "./helpers/repo-paths" );

// Map npm arch to iotivity arch - different mapping in each OS, it seems :/
// This can get complicated ...
var archMap = {
	"win32": { "x64": "amd64" },
	"linux": { "x64": "x86_64" }
};

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

var architecture;

var patchesDirectory = path.join( repoPaths.root, "patches" );

// Determine the CSDK revision
var csdkRevision = process.env.CSDK_REVISION ||
	require( path.join( repoPaths.root, "package.json" ) ).version
		.replace( /-[0-9]*$/, "" )
		.replace(  /^([^-]*-)pre-(.*$)/, "$2" );

var iotivityPath = path.join( repoPaths.root, "iotivity-native" );

var commands = [];

// We assume that, if the path is there, its contents are also ready to go
if ( fs.existsSync( iotivityPath ) ) {
	return;
}

// Attempt to determine the architecture
architecture = process.env.npm_config_user_agent.match( /\S+/g ) || [];
architecture =
	archMap[ architecture[ 2 ] ] ? archMap[ architecture[ 2 ] ][ architecture[ 3 ] ] : null;

// Do a shallow checkout of iotivity
commands.push( [ "git", [ "clone", "--depth", "1", "--single-branch", "--branch", csdkRevision,
	"https://gerrit.iotivity.org/gerrit/iotivity", iotivityPath ],
	{ shell: true, stdio: "inherit" } ] );

// Apply patches
if ( fs.existsSync( patchesDirectory ) && fs.statSync( patchesDirectory ).isDirectory() ) {
	fs.readdirSync( patchesDirectory ).forEach( function( item ) {
		item = path.join( patchesDirectory, item );
		commands.push( [ "git", [ "apply", item ],
			{ shell: true, cwd: iotivityPath, stdio: "inherit" } ] );
	} );
}

// Clone mbedtls inside iotivity
commands.push( [ "git",
	[ "clone", "https://github.com/ARMmbed/mbedtls.git", "extlibs/mbedtls/mbedtls" ],
	{ shell: true, cwd: iotivityPath, stdio: "inherit" } ] );

// Check out known-good commitid of mbedtls
commands.push( [ "git", [ "checkout", "ad249f509fd62a3bbea7ccd1fef605dbd482a7bd" ],
	{ shell: true, cwd: path.join( iotivityPath, "extlibs", "mbedtls", "mbedtls" ) } ] );


// Clone tinycbor inside iotivity
commands.push( [ "git",
	[ "clone", "https://github.com/01org/tinycbor.git", "extlibs/tinycbor/tinycbor" ],
	{ shell: true, cwd: iotivityPath, stdio: "inherit" } ] );

// Check out known-good commitid of tinycbor
commands.push( [ "git", [ "checkout", "31c7f81d45d115d2007b1c881cbbd3a19618465c" ],
	{ shell: true, cwd: path.join( iotivityPath, "extlibs", "tinycbor", "tinycbor" ) } ] );

// Build
commands.push( [ "scons", [ "SECURED=1" ]
	.concat( architecture ? [ "TARGET_ARCH=" + architecture ] : [] )
	.concat( process.env.npm_config_debug === "true" ?
		[ "RELEASE=False" ] : [] )
	.concat( [ "logger", "octbstack", "connectivity_abstraction", "coap", "c_common", "ocsrm",
		"routingmanager", "json2cbor"
	] ),
	{ shell: true, cwd: iotivityPath, stdio: "inherit" } ] );

// Actually run the commands
( function runCommand( index ) {
	run.apply( null, commands[ index ] ).on( "exit", function( code ) {
		var binariesSourcePath, binariesDestinationPath, toolName;

		if ( code !== 0 ) {
			process.exit( 1 );
		} else if ( ++index < commands.length ) {
			runCommand( index );
		} else {
			binariesSourcePath = findBins( iotivityPath );
			binariesDestinationPath = path.join( iotivityPath, "binaries" );
			toolName = "json2cbor" + ( process.platform.match( /^win/ ) ? ".exe" : "" );

			// The last step is to move the binaries from
			// iotivity-native/out/<os>/<arch>/<buildtype> to a hardcoded path. This is a necessary
			// step because the binding.gyp file is being evaluated before the path
			// iotivity-native/out/<os>/<arch>/<buildtype> is created, so it cannot be found at
			// that time.
			fs.renameSync( binariesSourcePath, binariesDestinationPath );

			// Move json2cbor next to the libraries
			fs.renameSync(
				path.join( binariesDestinationPath, "resource", "csdk", "security", "tool",
					toolName ),
				path.join( binariesDestinationPath, toolName ) );
		}
	} );
} )( 0 );

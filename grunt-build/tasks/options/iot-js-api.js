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

var assignIn = require( "lodash.assignin" );
var path = require( "path" );
var ocfRunner = require( "iot-js-api" );
var results = require( "../../../tests/getresult" );

var packageRoot = path.join( require( "bindings" ).getRoot( __filename ) );
var preamblePath = path.join( packageRoot, "tests", "preamble" );
var generateSpawn = function( spawnFinal ) {
	return function( interpreter, commandLine, name ) {
		commandLine[ 2 ] = grunt.option( "ci" ) ?
			path.dirname( require.resolve( "iotivity-node" ) ) :
			packageRoot;
		require( preamblePath ).apply( this, commandLine );
		return spawnFinal( interpreter, commandLine, name );
	};
};
var plain = {
	lineFilter: function( line, childPath ) {
		grunt.verbose.write(
			( childPath.match( /tests[/](.*)$/ ) || [] )[ 1 ] + ": " + line + "\n" );
		return line.match( /^{/ ) ? line : "";
	},
	spawn: generateSpawn( ocfRunner.defaultSpawn )
};
var coverage = assignIn( {}, plain, {
	spawn: generateSpawn( function( interpreter, commandLine ) {
		return require( "child_process" ).spawn(
			path.resolve( packageRoot, "node_modules", ".bin", "istanbul" ), [
				"cover",
				"--print", "none",
				"--report", "none",
				"--config", path.join( packageRoot, "tests", "istanbul.json" ),
				"--dir", path.join( packageRoot, "coverage",
					commandLine[ 0 ].split( path.sep ).slice( -2 ).join( "!" ) ),
				"--root", commandLine[ 2 ],
				commandLine[ 0 ], "--"
			].concat( commandLine.slice( 1 ) ), {
				stdio: [ process.stdin, "pipe", process.stderr ]
			} );
	} )
} );

// Shim the default log() and done() to also record results in JSON
ocfRunner.defaultCallbacks.done = ( function( originalDone ) {
	return function() {
		results.saveResults( "ocf" );
		return originalDone.apply( this, arguments );
	};
} )( ocfRunner.defaultCallbacks.done );

ocfRunner.defaultCallbacks.log = ( function( originalLog ) {
	return function( status ) {
		results.getTestResult( status );
		return originalLog.apply( this, arguments );
	};
} )( ocfRunner.defaultCallbacks.log );

return {
	plain: {
		api: "ocf",
		apiVersion: "oic1.1.0-0",
		client: plain,
		server: plain,
		single: plain
	},
	coverage: {
		api: "ocf",
		apiVersion: "oic1.1.0-0",
		client: coverage,
		server: coverage,
		single: coverage
	}
};

};

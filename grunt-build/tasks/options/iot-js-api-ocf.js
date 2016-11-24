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

var _ = require( "lodash" );
var path = require( "path" );
var ocfRunner = require( "iot-js-api" );
var results = require( "../../../tests/getresult" );

var packageRoot = path.join( require( "bindings" ).getRoot( __filename ) );
var location = grunt.option( "ci" ) ?
		path.join( packageRoot, "node_modules", "iotivity-node" ) : packageRoot;
var plain = {
	location: location,
	preamble: function( uuid ) {
		return "require( \"" +
			path.join( packageRoot, "tests", "preamble" )
				.replace( /[\\]/g, "\\\\" ) +
			"\" )( \"" + uuid + "\", \"" +
			location
				.replace( /[\\]/g, "\\\\" ) + "\" );";
	}
};
var coverage = _.extend( {}, plain, {
	spawn: function( interpreter, commandline ) {
		var testScript = commandline.shift();
		return require( "child_process" ).spawn(
			path.resolve( packageRoot, "node_modules", ".bin", "istanbul" ), [
				"cover",
				"--print", "none",
				"--report", "none",
				"--config", path.join( packageRoot, "tests", "istanbul.json" ),
				"--dir", path.join( packageRoot, "coverage",
					testScript.split( path.sep ).slice( -2 ).join( "!" ) ),
				"--root", location,
				testScript, "--"
			].concat( commandline ), { stdio: [ process.stdin, "pipe", process.stderr ] } );
	}
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
	plain: { client: plain, server: plain, single: plain },
	coverage: { client: coverage, server: coverage, single: coverage }
};

};

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
var ocfRunner = require( "iot-js-api" );
var results = require( "../../../tests/getresult" );

var plain = {
	location: path.resolve( path.join( __dirname, "..", "..", ".." ) ),
	preamble: function( uuid ) {
		return "require( \"" +
			path.join( require( "bindings" ).getRoot( __filename ), "tests", "preamble" )
				.replace( /[\\]/g, "\\\\" ) +
			"\" )( \"" + uuid + "\" );";
	}
};
var coverage = _.extend( {}, plain, {
		interpreter:
			path.resolve( path.join( __dirname, "..", "..", "..", "tests", "run-istanbul.sh" ) )
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

module.exports = {
	plain: { client: plain, server: plain, single: plain },
	coverage: { client: coverage, server: coverage, single: coverage }
};

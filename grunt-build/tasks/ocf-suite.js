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

var _ = require( "lodash" ),
	path = require( "path" );

grunt.task.registerTask( "ocf-suite", "Run the OCF test suite", function() {
	var testSuite;
	var done = this.async();

	if ( typeof Promise === "undefined" ) {
		return done();
	}

	testSuite = require( "ocf-test-suite" );

	_.extend( testSuite, {
		defaultCallbacks: _.extend( testSuite.defaultCallbacks, {
			done: ( function( originalDone ) {
				return function() {
					if ( originalDone ) {
						return originalDone.apply( this, arguments );
					}
					done();
				};
			} )( testSuite.defaultCallbacks.done )
		} )
	} )( _.extend( {
		location: path.resolve( path.join( __dirname, "..", ".." ) )
	}, grunt.option( "ocf-suites" ) ? {
		tests: grunt.option( "ocf-suites" ).split( "," )
	} : {} ) );
} );

};

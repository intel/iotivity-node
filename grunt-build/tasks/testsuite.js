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
	path = require( "path" ),
	spawn = require( "child_process" ).spawn;

grunt.task.registerTask( "testsuite", "Run the test suite", function() {
	var done = this.async();
	spawn( "node",
		[ path.join( process.cwd(), "tests", "suite.js" ) ]
			.concat( grunt.option( "suites" ) ? [ grunt.option( "suites" ) ] : [] ),
		{
			env: _.extend( {}, process.env, { "MALLOC_CHECK_": 2 } ),
			stdio: "inherit"
		} )
		.on( "close", function( code ) {
			done( code === 0 || code === null );
		} );
} );

};

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

var spawn = require( "child_process" ).spawn,
	packageName = grunt.file.readJSON( "package.json" ).name;

grunt.task.registerTask( "testdist", "Test the distributed version", function() {
	var done = this.async();

	spawn( ( process.env.SHELL || "sh" ),
		[
			"-c",
			"./dist.sh -i " + ( grunt.option( "ci" ) ? "-n" : "" ) +
			"&& cd dist/" + packageName +
			"&& echo '*** Loading high-level API ***'" +
			"&& node ./index.js" +
			"&& echo '*** Loading low-level API ***'" +
			"&& node ./lowlevel.js"
		],
		{ stdio: "inherit" } )
		.on( "exit", function( code ) {
			done.apply( this, ( code === 0 ? [] : [ false ] ) );
		} );
} );

};

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

var spawn = require( "child_process" ).spawn;

grunt.task.registerTask( "clangformat", "Format the C++ files", function() {
	var done = this.async();

	spawn( ( process.env.SHELL || "sh" ),
		[
			"-c",
			"find src -type f | while read; do " +
			"clang-format -style=Google \"$REPLY\" > \"$REPLY\".new && " +
			"mv \"$REPLY\".new \"$REPLY\" && echo \"File $REPLY formatted.\";" +
			"done"
		],
		{ stdio: "inherit" } )
		.on( "exit", function( code ) {
			done.apply( this, ( code === 0 ? [] : [ false ] ) );
		} );
} );

};

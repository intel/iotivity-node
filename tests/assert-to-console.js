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

module.exports = {

	// Create an assertion and pass it to the parent process via stdout
	assert: function( assertion ) {
		var copyOfArguments;

		// Copy the arguments and remove the assertion
		copyOfArguments = Array.prototype.slice.call( arguments, 0 );
		copyOfArguments.shift();

		console.log( JSON.stringify( {
			assertion: assertion,
			arguments: copyOfArguments
		} ) );
	},

	info: function( message ) {
		console.log( JSON.stringify( {
			info: true,
			message: message
		} ) );
	},

	die: function( message ) {
		console.log( JSON.stringify( { teardown: true, message: message, isError: true } ) );
		process.exit( 1 );
	}
};

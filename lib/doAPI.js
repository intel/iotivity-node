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
var csdk = require( "./csdk" );

// api: The API to call.
// error: The message of the Error with which to reject
// The rest of the arguments are passed to the API requested.
function doAPI( api, error ) {
	var apiArguments = Array.prototype.slice.call( arguments, 2 );
	return new Promise( function( fulfill, reject ) {
		var result = csdk[ api ].apply( this, apiArguments );
		if ( result === csdk.OCStackResult.OC_STACK_OK ) {
			fulfill();
		} else {
			reject( _.extend( new Error( error ), {
				result: result
			} ) );
		}
	} );
}

module.exports = doAPI;

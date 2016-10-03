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

var iotivity = require( "bindings" )( "iotivity" );
var utils = require( "./assert-to-console" );
var counter = 0;

iotivity.OCDoResource = ( function( original ) {
	return function() {
		var localCounter = counter++;
		utils.info( "OCDoResource(" + localCounter + "): " +
			JSON.stringify( arguments, null, 4 ) );
		arguments[ 7 ] = ( function( originalCallback ) {
			return function() {
				utils.info( "OCDoResource(" + localCounter + ") response: " +
					JSON.stringify( arguments, null, 4 ) );
				return originalCallback.apply( this, arguments );
			};
		} )( arguments[ 7 ] );
		return original.apply( this, arguments );
	};
} )( iotivity.OCDoResource );

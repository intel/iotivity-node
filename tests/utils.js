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

var _ = require( "lodash" ),
	TestUtils = function( iotivity ) {
		if ( !( this instanceof TestUtils ) ) {
			return new TestUtils( iotivity );
		}

		this._iotivity = iotivity;
		process.on( "message", function() {
			setTimeout( process.exit, 1000, 0 );
		} );
	};

_.extend( TestUtils.prototype, require( "./assert-to-console" ), {
	lookupEnumValueName: function( enumName, value ) {
		var index,
			enumeration = this._iotivity[ enumName ];

		for ( index in enumeration ) {
			if ( enumeration[ index ] === value ) {
				return index;
			}
		}
	},
	lookupBitfieldValueNames: function( enumName, value ) {
		var index,
			enumeration = this._iotivity[ enumName ],
			bits = {};

		for ( index in enumeration ) {
			if ( value & enumeration[ index ] ) {
				bits[ index ] = true;
			}
		}
		return bits;
	},

	stackOKOrDie: function( module, nameOfStep, result ) {

		// Two-argument configuration means module was skipped
		if ( arguments.length === 2 ) {
			result = nameOfStep;
			nameOfStep = module;
			module = "";
		}

		console.log( JSON.stringify( {
			assertion: "strictEqual",
			arguments: [
				this.lookupEnumValueName( "OCStackResult", result ),
				"OC_STACK_OK",
				( module ? module + ": " : "" ) + nameOfStep + " successful"
			]
		} ) );
		if ( result === this._iotivity.OCStackResult.OC_STACK_OK ) {
			return true;
		} else {
			this.die( module + ": " + nameOfStep + " failed" );
			return false;
		}
	},
	findResource: function( response, uuid ) {
		var index, resources,
			returnValue = false;

		if ( response &&
				response.payload &&
				response.payload.resources &&
				response.payload.resources.length ) {

			resources = response.payload.resources;

			for ( index in resources ) {
				if ( resources[ index ].uri === "/a/" + uuid ) {
					return response.addr;
				}
			}
		}

		return returnValue;
	}
} );

module.exports = TestUtils;

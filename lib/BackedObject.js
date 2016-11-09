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

function accessors( instance, property, value, backer ) {
	return {
		get: function() {
			return value;
		},
		set: function( newValue ) {
			var result;
			var toCommit = _.extend( {}, instance );

			toCommit[ property ] = newValue;

			result = backer( toCommit );
			if ( result instanceof Error ) {
				throw result;
			}

			value = newValue;
		}
	};
}

function BackedObject( object, backer ) {
	var index;

	if ( !( this instanceof BackedObject ) ) {
		return new BackedObject( object, backer );
	}

	for ( index in object ) {
		Object.defineProperty( this, index, _.extend( {
			enumerable: true
		}, accessors( this, index, object[ index ], backer ) ) );
	}
}

module.exports = _.extend( BackedObject, {
	attach: function( object, property, value, backer, wasSet ) {
		var index;
		var keys = [];

		for ( index in value ) {
			keys.push( index );
		}
		value = BackedObject( value, backer );

		Object.defineProperty( object, property, {
			enumerable: true,
			get: function() {
				return value;
			},
			set: function( newValue ) {
				var result, index;
				var actualValue = {};

				for ( index in keys ) {
					actualValue[ keys[ index ] ] = newValue[ keys[ index ] ] || null;
				}

				newValue = BackedObject( actualValue, backer );
				if ( wasSet ) {
					wasSet( newValue );
				}

				result = backer( newValue );
				if ( result instanceof Error ) {
					throw result;
				}

				value = newValue;
			}
		} );
		if ( wasSet ) {
			wasSet( value );
		}

		return value;
	}
} );

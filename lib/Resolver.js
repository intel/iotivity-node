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
var Resolver = function() {
	if ( !( this instanceof Resolver ) ) {
		return new Resolver();
	}

	// deviceId -> address
	this._addresses = {};
};
_.extend( Resolver.prototype, {
	add: function( deviceId, address ) {
		this._addresses[ deviceId ] = address;
	},
	remove: function( item ) {
		delete this._addresses[ typeof item === "string" ? item :
			_.findKey( this._addresses, function( value ) {
				return _.isEqual( value, item );
			} ) ];
	},
	get: function( item ) {
		return ( typeof item === "string" ? this._addresses[ item ] :
			_.findKey( this._addresses, function( value ) {
				return _.isEqual( value, item );
			} ) ) || _.extend( new Error( "Device not found" ), {
				given: item
			} );
	}
} );

module.exports = new Resolver();

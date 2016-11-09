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
var doAPI = require( "./doAPI" );
var payload = require( "./payload" );
var querystring = require( "querystring" );

function defaultTranslate() {
	return _.extend( {}, this.properties );
}

function stringArrayGetter( resource, api ) {
	return function() {
		var oneString;
		var index = 0;
		var returnValue = [];

		if ( resource._private.handle.stale ) {
			return;
		}

		for ( oneString = csdk[ api ]( resource._private.handle, index++ );
				oneString;
				oneString = csdk[ api ]( resource._private.handle, index++ ) ) {
			returnValue.push( oneString );
		}

		return returnValue;
	};
}

var ServerResource = function ServerResource() {
	if ( !( this instanceof ServerResource ) ) {
		return new ServerResource();
	}

	this.properties = {};

	Object.defineProperty( this, "_private", {
		value: { translate: _.bind( defaultTranslate, this ) }
	} );

	Object.defineProperty( this, "deviceId", {
		enumerable: true,
		get: function() {
			return csdk.OCGetServerInstanceIDString();
		}
	} );

	Object.defineProperty( this, "resourcePath", {
		enumerable: true,
		get: function() {
			return this._private.handle.stale ?
				this._private.oldResourcePath :
				csdk.OCGetResourceUri( this._private.handle );
		}
	} );
	Object.defineProperty( this, "resourceTypes", {
		enumerable: true,
		get: stringArrayGetter( this, "OCGetResourceTypeName" )
	} );
	Object.defineProperty( this, "interfaces", {
		enumerable: true,
		get: stringArrayGetter( this, "OCGetResourceInterfaceName" )
	} );
	Object.defineProperty( this, "slow", {
		enumerable: true,
		get: function() {
			return this._private.handle.stale ? undefined :
				!!( csdk.OCGetResourceProperties( this._private.handle ) &
					csdk.OCResourceProperty.OC_SLOW );
		}
	} );
	Object.defineProperty( this, "active", {
		enumerable: true,
		get: function() {
			return this._private.handle.stale ? undefined :
				!!( csdk.OCGetResourceProperties( this._private.handle ) &
					csdk.OCResourceProperty.OC_ACTIVE );
		}
	} );
	Object.defineProperty( this, "secure", {
		enumerable: true,
		get: function() {
			return this._private.handle.stale ? undefined :
				!!( csdk.OCGetResourceProperties( this._private.handle ) &
					csdk.OCResourceProperty.OC_SECURE );
		}
	} );
	Object.defineProperty( this, "observable", {
		enumerable: true,
		get: function() {
			return this._private.handle.stale ? undefined :
				!!( csdk.OCGetResourceProperties( this._private.handle ) &
					csdk.OCResourceProperty.OC_OBSERVABLE );
		}
	} );
	Object.defineProperty( this, "discoverable", {
		enumerable: true,
		get: function() {
			return this._private.handle.stale ? undefined :
				!!( csdk.OCGetResourceProperties( this._private.handle ) &
					csdk.OCResourceProperty.OC_DISCOVERABLE );
		}
	} );
};

function addHandler( proto, name ) {
	proto[ "on" + name ] = function( handler ) {
		this._private[ name ] = _.bind( handler, this );
		return this;
	};
}

[ "translate", "retrieve", "update", "delete", "create" ].forEach( function( item ) {
	addHandler( ServerResource.prototype, item );
} );

_.extend( ServerResource.prototype, {
	unregister: function() {

		// We'll need the resource path for a response to delete, if the application chooses to
		// send such a response.
		this._private.oldResourcePath = this.resourcePath;
		return doAPI( "OCDeleteResource", "Failed to delete resource", this._private.handle );
	},
	notify: function() {
		return new Promise( _.bind( function( fulfill, reject ) {
			var query, result, thePayload;
			var errors = [];

			for ( query in this._private.observers ) {
				thePayload = payload.objectToRepPayload(
					this._private.translate( querystring.parse( query ) ) );
				if ( thePayload instanceof Error ) {
					errors.push( thePayload );
				} else {
					result = csdk.OCNotifyListOfObservers( this._private.handle,
						this._private.observers[ query ], thePayload,
						csdk.OCQualityOfService.OC_HIGH_QOS );
					if ( result !== csdk.OCStackResult.OC_STACK_OK ) {
						errors.push( {
							query: query,
							result: result,
							observers: this._private.observers[ query ].slice( 0 )
						} );
					}
				}
			}

			if ( errors.length > 0 ) {
				reject( errors );
			} else {
				fulfill();
			}
		}, this ) );
	}
} );

module.exports = ServerResource;

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
var doAPI = require( "./doAPI" );
var csdk = require( "./csdk" );
var payload = require( "./payload" );
var Resource = require( "./Resource" );

function respond( request, error, data ) {
	var responsePayload = null;
	var resourceHandle = request.target._private.handle;

	// Special case: The server has decided to throw out an observer because of a network failure
	// encountered while attempting to deliver a notification. In that case, we do nothing.
	if ( !request.id && request.observe === false ) {
		return Promise.accept();
	}

	resourceHandle = ( ( resourceHandle && resourceHandle.stale ) ? null : resourceHandle );

	if ( !error ) {
		if ( !data && request._type === "retrieve" ) {
			data = request.target.properties;
		}

		responsePayload = ( request._type === "create" ) ?
			( data ?
				payload.initToCreateRequestPayload( data ) :
				new Error( "create: missing payload" ) ) :
			( data ?
				payload.objectToRepPayload(
					data instanceof Resource ?
						data._private.transform( data.properties, request.options ) :
						request.target._private.transform( data, request.options ) ) :
				null );

		if ( responsePayload instanceof Error ) {
			error = responsePayload;
			responsePayload = null;
		}
	}

	return doAPI( "OCDoResponse", "Failed to send response", {
		requestHandle: request.id,
		resourceHandle: resourceHandle,
		ehResult: csdk.OCEntityHandlerResult[
			error ? "OC_EH_ERROR" :
			request._type === "create" ? "OC_EH_RESOURCE_CREATED" :
			"OC_EH_OK"
		],
		payload: responsePayload,
		sendVendorSpecificHeaderOptions: [],
		resourceUri: request.target.resourcePath
	} );
}

function Request( init ) {
	if ( !( this instanceof Request ) ) {
		return new Request( init );
	}
	Object.defineProperty( this, "_type", { writable: true } );
	_.extend( this, init );
}

_.extend( Request.prototype, {
	respond: function( data ) {
		return respond( this, null, data );
	},
	error: function( error ) {
		return respond( this, error );
	}
} );

module.exports = Request;

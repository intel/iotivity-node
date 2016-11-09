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
var ServerResource = require( "./ServerResource" );

function respond( request, error, data ) {
	var responsePayload = null;
	var resourceHandle = request.target ? request.target._private.handle : null;

	// Special case: The server has decided to throw out an observer because of a network failure
	// encountered while attempting to deliver a notification. In that case, we do nothing.
	if ( !request.id && request.observe === false ) {
		return Promise.accept();
	}

	resourceHandle = ( ( resourceHandle && resourceHandle.stale ) ? null : resourceHandle );

	if ( !error ) {
		if ( !data && request.type === "retrieve" ) {
			data = request.target.properties;
		}

		responsePayload = ( request.type === "create" ) ?
			( data ?
				payload.initToCreateRequestPayload( data ) :
				new Error( "create: missing payload" ) ) :
			( data ?
				payload.objectToRepPayload(
					data instanceof ServerResource ?
						data._private.translate( request.options ) :
						request.target._private.translate( request.options ) ) :
				null );

		if ( responsePayload instanceof Error ) {
			error = responsePayload;
			responsePayload = null;
		}
	}

	var response = {
		requestHandle: request.id,
		resourceHandle: resourceHandle,
		ehResult: csdk.OCEntityHandlerResult[
			error ? "OC_EH_ERROR" :
			request.type === "create" ? "OC_EH_RESOURCE_CREATED" :
			request.type === "delete" ? "OC_EH_RESOURCE_DELETED" :
			"OC_EH_OK"
		],
		payload: responsePayload,
		sendVendorSpecificHeaderOptions: [],
		resourceUri: request.target ? request.target.resourcePath : data.resourcePath
	};

	return doAPI( "OCDoResponse", "Failed to send response", response );
}

function Request( init ) {
	if ( !( this instanceof Request ) ) {
		return new Request( init );
	}
	_.extend( this, init );
}

_.extend( Request.prototype, {
	respond: function( data ) {
		return respond( this, null, data );
	},
	respondWithError: function( error ) {
		return respond( this, error );
	}
} );

module.exports = Request;

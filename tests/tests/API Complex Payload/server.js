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

var requestPromise, resource,
	_ = require( "lodash" ),
	async = require( "async" ),
	uuid = process.argv[ 2 ],
	device = require( "../../../index" )(),
	testUtils = require( "../../assert-to-console" );

console.log( JSON.stringify( { assertionCount: 3 } ) );

function handlerWithPromise( handler ) {
	var succeeded, failed;

	return _.extend( function( event ) {
		return handler( event, succeeded, failed );
	}, {
		promise: new Promise( function( fulfill, reject ) {
			succeeded = fulfill, failed = reject;
		} )
	} );
}

async.series( [

	// Init
	function( callback ) {
		device.configure( {
			role: "server",
			info: {
				uuid: uuid,
				name: "api-complex-payload-" + uuid,
				manufacturerName: "Intel"
			}
		} ).then( callback, callback );
	},

	// Create resource and attach retrieve handler
	function( callback ) {
		device.registerResource( {
			id: { path: "/a/" + uuid },
			discoverable: true,
			observable: true,
			resourceTypes: [ "core.light" ],
			interfaces: [ "oic.if.baseline" ]
		} ).then(
			function( theResource ) {

				// Attach retrieve handler
				var handler = handlerWithPromise(
					function retrieveHandler( request, fulfill, reject ) {
						var cleanup = function( error ) {
							device.removeEventListener( "request", handler );
							if ( error ) {
								reject( error );
							} else {
								fulfill();
							}
						};
						testUtils.assert( "strictEqual", request.type, "retrieve",
							"Client: First event is 'retrieve'" );
						request.sendResponse( _.extend( theResource, {
							properties: {
								primitiveValue: 42,
								nullValue: null,
								objectValue: {
									childValue: 91
								},
								arrayValue: [ 19, 23, 7 ]
							}
						} ) ).then( cleanup, cleanup );
					} );

				resource = theResource;
				device.addEventListener( "request", handler );
				requestPromise = handler.promise;

				callback();
			}, callback );
	},

	// Wait for retrieve handler and attach update handler
	function( callback ) {

		// Signal that we're ready to go
		console.log( JSON.stringify( { ready: true } ) );

		requestPromise.then( function() {
			var handler = handlerWithPromise(
				function updateHandler( request, fulfill, reject ) {
					var cleanup = function( error ) {
						device.removeEventListener( "request", handler );
						if ( error ) {
							reject( error );
						} else {
							fulfill();
						}
					};

					testUtils.assert( "strictEqual", request.type, "update",
						"Client: Second event is 'update'" );

					testUtils.assert( "deepEqual", request.res, {
						putValue: "A string",
						anotherPutValue: 23.7,
						childValues: {
							putChildNull: null,
							putChildValue: false,
							putChildArray: [ [ 2, 3, 5 ], [ 9, 11, 17 ] ]
						}
					}, "Server: update event payload is correct" );

					request.sendResponse( null ).then( cleanup, cleanup );
				} );
			device.addEventListener( "request", handler );
			requestPromise = handler.promise;
			callback();
		}, callback );
	},

	// Wait for update handler
	function( callback ) {
		requestPromise.then( function() {
			callback();
		}, callback );
	}
], function( error ) {
	if ( error ) {
		testUtils.die( "Server: " + error.message + ", result: " + error.result );
	}
} );

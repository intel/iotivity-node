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

var path = require( "path" );

require( "../tests/preamble" )( __filename, [
	{
		href: "/a/high-level-resource-creation-example",
		rel: "",
		rt: [ "core.light" ],
		"if": [ "oic.if.baseline" ]
	},

	// The resource the remote end will ask us to create
	{
		href: "/a/new-resource",
		rel: "",
		rt: [ "core.light" ],
		"if": [ "oic.if.baseline" ]
	}
], path.resolve( path.join( __dirname, ".." ) ) );

var resourceCreatedByRemote,
	device = require( "iotivity-node" ),
	_ = {
		extend: require( "lodash.assignin" ),
		bind: require( "lodash.bind" )
	};

device.device = _.extend( device.device, {
	coreSpecVersion: "res.1.1.0",
	dataModels: [ "something.1.0.0" ],
	name: "api-server-example"
} );
device.platform = _.extend( device.platform, {
	manufacturerName: "Intel",
	manufactureDate: new Date( "Wed Sep 23 10:04:17 EEST 2015" ),
	platformVersion: "1.1.1",
	firmwareVersion: "0.0.1",
	supportUrl: "http://example.com/"
} );

function throwError( error ) {
	console.error( error.stack ? error.stack : ( error.message ? error.message : error ) );
	process.exit( 1 );
}

device.server
	.oncreate( function( request ) {
		console.log( "create request" );
		device.server.register( _.extend( request.data, {
			discoverable: true
		} ) ).then( function( resource ) {
			console.log( "resource successfully registered" );
			resourceCreatedByRemote = resource;
			request.respond( resource );
			resource.ondelete( function( request ) {
				console.log( "delete request" );
				resourceCreatedByRemote.unregister().then(
					function() {
						console.log( "resource successfully deleted" );
						request.respond()
							.then( function() {
								console.log( "delete request successfully delivered" );
							},
							function( anError ) {
								console.log( ( "" + anError.stack ) +
									JSON.stringify( anError, null, 4 ) );
							} );
					},
					_.bind( request.respondWithError, request ) );
			} );
		}, _.bind( request.respondWithError, request ) );
	} )
	.register( {
		resourcePath: "/a/high-level-resource-creation-example",
		resourceTypes: [ "core.light" ],
		interfaces: [ "oic.if.baseline" ],
		discoverable: true,
		observable: true,
		properties: { someValue: 0, someOtherValue: "Helsinki" }
	} ).then( function() {
		console.log( "initial resource successfully registered" );
	}, throwError );

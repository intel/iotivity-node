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

var resource,
	_ = require( "lodash" ),
	async = require( "async" ),
	uuid = process.argv[ 2 ],
	device = require( "../../../index" )(),
	testUtils = require( "../../assert-to-console" );

console.log( JSON.stringify( { assertionCount: 1 } ) );

async.series( [

	function initStack( callback ) {
		device.configure( {
			role: "client"
		} ).then( callback, callback );
	},

	function discoverResource( callback ) {
		Promise.all( [
			new Promise( function( fulfill ) {
				var resourceFound = function( event ) {
					if ( event.resource.id.path === "/a/" + uuid ) {
						resource = event.resource;
						fulfill();
						device.removeEventListener( "resourcefound", resourceFound );
					}
				};
				device.addEventListener( "resourcefound", resourceFound );
			} ),
			device.findResources()
		] ).then(
			function() {
				callback();
			}, callback );
	},

	function performRetrieve( callback ) {
		device.retrieveResource( resource.id ).then(
			function( resource ) {
				testUtils.assert( "deepEqual", resource.properties, {
					primitiveValue: 42,
					nullValue: null,
					objectValue: {
						childValue: 91
					},
					arrayValue: [ 19, 23, 7 ]
				}, "Client: Retrieved properties are correct" );
				callback();
			},
			callback );
	},

	function performUpdate( callback ) {
		device.updateResource( _.extend( resource, {
			properties: {
				putValue: "A string",
				anotherPutValue: 23.7,
				childValues: {
					putChildNull: null,
					putChildValue: false,
					putChildArray: [ [ 2, 3, 5 ], [ 9, 11, 17 ] ]
				}
			}
		} ) ).then( function() {
			console.log( JSON.stringify( { killPeer: true } ) );
			process.exit( 0 );
		}, callback );
	}
], function( error ) {
	if ( error ) {
		testUtils.die( "Client: " + error.message + ", result: " + error.result );
	}
} );

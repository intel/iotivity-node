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

var id,
	_ = require( "lodash" ),
	async = require( "async" ),
	util = require( "util" ),
	myUtils = require( "../../assert-to-console" ),
	device = require( "../../../index" )(),
	uuid = process.argv[ 2 ];

console.log( JSON.stringify( { assertionCount: 6 } ) );

async.series( [
	function configureTheDevice( callback ) {
		device.configure( { role: "client" } ).then( callback, callback );
	},

	function discoverTheResource( callback ) {
		var teardown,
			handler = function( event ) {
				if ( event.resource.id.path === "/a/" + uuid ) {
					id = event.resource.id;
					teardown();
				}
			};
		teardown = function( error ) {
			device.removeEventListener( "resourcefound", handler );
			callback( error );
		};
		device.addEventListener( "resourcefound", handler );
		device.findResources().catch( teardown );
	},

	function performFirstRetrieve( callback ) {
		device.retrieveResource( id ).then(
			function() {
				myUtils.die( "Client: First retrieve succeeded" );
			},
			function( error ) {
				myUtils.assert( "ok", util.isError( error ),
					"Client: First retrieve failed with Error" );
				myUtils.assert( "strictEqual", error.message,
					"retrieveResource: OCDoResource response failed",
					"Client: First error message is correct" );
				myUtils.assert( "deepEqual", _.extend( {}, error ), {
					result: 21,
					properties: {}
				}, "Client: First Error has the expected structure" );
				callback();
			} );
	},

	function performSecondRetrieve( callback ) {
		device.retrieveResource( id ).then(
			function() {
				myUtils.die( "Client: Second retrieve succeeded" );
			},
			function( error ) {
				myUtils.assert( "ok", util.isError( error ),
					"Client: Second retrieve failed with Error" );
				myUtils.assert( "strictEqual", error.message,
					"Something went horribly wrong!",
					"Client: Second error message is correct" );
				myUtils.assert( "deepEqual", _.extend( {}, error ), {
					result: 21,
					properties: {
						excuses: [ "Tired", "Hungry", "Sleepy", "Sick" ]
					}
				}, "Client: Second Error has the expected structure" );
				callback();
			} );
	}
], function( error ) {
	if ( error ) {
		myUtils.die( error );
	} else {
		console.log( JSON.stringify( { killPeer: true } ) );
		process.exit( 0 );
	}
} );

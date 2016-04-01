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

var theResource,
	async = require( "async" ),
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )(),
	uuid = process.argv[ 2 ];

console.log( JSON.stringify( { assertionCount: 6 } ) );

async.series( [
	function configureTheDevice( callback ) {
		device.configure( { role: "client" } ).then( function() {
			utils.assert( "ok", true, "Client: device.configure() successful" );
			callback();
		}, callback );
	},
	function attemptFakeRetrieve( callback ) {
		device.retrieveResource( { deviceId: "SomethingCrazy", path: "/going/nuts/here" } ).then(
			function( resource ) {
				utils.assert( "ok", false, "Client: Retrieving fake resource succeeded: " +
					JSON.stringify( resource ) );
				callback();
			},
			function( error ) {
				utils.assert( "strictEqual", "" + error,
					"Error: retrieveResource: resource undefined",
					"Client: Retrieving fake resource failed locally" );
				callback();
			} );
	},
	function discoverTheResource( callback ) {
		var teardown;
		var handleResourcefound = function( event ) {
			if ( event.resource.id.path === "/a/" + uuid ) {
				utils.assert( "ok", true, "Client: Resource found" );
				theResource = event.resource;
				teardown();
			}
		};
		teardown = function( error ) {
			device.removeEventListener( "resourcefound", handleResourcefound );
			callback( error );
		};
		device.addEventListener( "resourcefound", handleResourcefound );
		device.findResources().then( function() {
			utils.assert( "ok", true, "Client: findResources() successful" );
		}, teardown );
	},
	function retrieveTheResource( callback ) {
		device.retrieveResource( theResource.id ).then(
			function( resource ) {
				utils.assert( "deepEqual", resource.properties, {
					"How many angels can dance on the head of a pin?": "As many as wanting."
				}, "Client: Retrieved properties are as expected." );
				callback();
			},
			callback );
	},
	function retrieveTheResourceAgain( callback ) {
		device.retrieveResource( theResource.id ).then(
			function() {
				utils.assert( "ok", false,
					"Client: Retrieving the resource a second time succeeded" );
				callback();
			},
			function( error ) {
				utils.assert( "strictEqual", "" + error,
					"Error: resource not found",
					"Client: Retrieving the resource a second time failed remotely" );
				callback();
			} );
	}
], function( error ) {
	if ( error ) {
		utils.die( error );
	} else {
		console.log( JSON.stringify( { killPeer: true } ) );
		process.exit( 0 );
	}
} );

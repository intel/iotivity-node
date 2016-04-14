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

var utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )( "client" ),
	uuid = process.argv[ 2 ];

console.log( JSON.stringify( { assertionCount: 2 } ) );

new Promise( function discoverTheResource( fulfill, reject ) {
	var teardown,
		resourcefound = function( event ) {
			if ( event.resource.id.path === "/a/" + uuid ) {
				teardown( null, event.resource );
			}
		};
	teardown = function( error, resource ) {
		device.removeEventListener( "resourcefound", resourcefound );
		if ( error ) {
			reject( error );
		} else {
			fulfill( resource );
		}
	};
	device.addEventListener( "resourcefound", resourcefound );
	device.findResources().catch( teardown );
} ).then( function retrieveTheResourceInGerman( resource ) {
	return device.retrieve( resource.id, { language: "de" } );
} ).then( function checkGermanAndRetrieveInHungarian( resource ) {
	utils.assert( "deepEqual", resource.properties,
		{ "How many angels can dance on the head of a pin?": "As many as wanting." },
		"Client: Answer for language de is as expected" );
	return device.retrieve( resource.id, { language: "hu" } );
} ).then( function checkHungarianAndQuit( resource ) {
	utils.assert( "deepEqual", resource.properties,
		{ "How many angels can dance on the head of a pin?": "Ahány akar." },
		"Client: Answer for language hu is as expected" );
	console.log( JSON.stringify( { killPeer: true } ) );
	process.exit( 0 );
} ).catch( function dieWithError( error ) {
	utils.die( "Client: Failed: " + error + ", and result " + error.result );
} );

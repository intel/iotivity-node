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

var _ = require( "lodash" ),
	utils = require( "../../assert-to-console" ),
	uuid = process.argv[ 2 ],
	device = require( "../../../index" )( "server" );

var response = {
	default: "As many as wanting.",
	hu: "Ahány akar."
};

console.log( JSON.stringify( { assertionCount: 2 } ) );

function die( message ) {
	return function dieWithError( error ) {
		utils.die( message + ": " + error );
	};
}

function resourceOnRequest( request ) {
	utils.assert( "strictEqual", request.type, "retrieve", "Server: Request is of type retrieve" );
	if ( request.type === "retrieve" ) {
		request.sendResponse( _.extend( {}, request.target, {
			properties: {
				"How many angels can dance on the head of a pin?":
					response[ request.queryOptions.language ] || response.default
			}
		} ) ).catch( die( "Failed to answer retrieve request" ) );
	}
}

device.registerResource( {
	id: { path: "/a/" + uuid },
	resourceTypes: [ "core.light" ],
	interfaces: [ "oic.if.baseline" ],
	discoverable: true,
	properties: {
		"How many angels can dance on the head of a pin?": response.default
	}
} ).then(
	function( resource ) {
		device.addEventListener( "retrieverequest", resourceOnRequest );
		process.on( "SIGINT", function() {
			device.unregisterResource( resource ).catch( die( "Failed to unregister resource" ) );
			process.exit( 0 );
		} );
		console.log( JSON.stringify( { ready: true } ) );
	},
	die( "Failed to register resource" ) );

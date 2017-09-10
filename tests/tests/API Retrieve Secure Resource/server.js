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

var server = require( process.argv[ 3 ] ).server;

console.log( JSON.stringify( { assertionCount: 5 } ) );

// Multiply a value by a scale given in the options
function transformSensorData( options ) {
	var scale = ( options && "scale" in options ) ? ( +options.scale ) : 1;

	if ( isNaN( scale ) ) {
		scale = 1;
	}

	return { value: this.properties.value * scale };
}

server
	.register( {
		resourcePath: "/a/" + process.argv[ 2 ],
		resourceTypes: [ "core.light" ],
		interfaces: [ "oic.if.baseline" ],
		discoverable: true,
		secure: true
	} )
	.then(
		function( resource ) {
			var requestCount = 0;

			resource.properties = {
				value: 42
			};
			resource.ontranslate( transformSensorData ).onretrieve( function( request ) {
				if ( requestCount++ > 1 ) {
					console.log( JSON.stringify( { assertion: "ok", arguments: [
						false, "Server: Unexpected extra request"
					] } ) );
				}
				console.log( JSON.stringify( { assertion: "ok", arguments: [
					request.target === resource,
					"Server: Resource passed to retrieve request is the one that was registered"
				] } ) );
				request.respond()
					.then(
						function() {
							console.log( JSON.stringify( { assertion: "ok", arguments: [
								true, "Server: Successful response to retrieve request"
							] } ) );
						},
						function( error ) {
							console.log( JSON.stringify( { assertion: "ok", arguments: [
								false, "Server: Failed to respond to retrieve request: " +
									( "" + error.message ) + "\n" +
										JSON.stringify( error, null, 4 )
							] } ) );
						} );
			} );
			console.log( JSON.stringify( { assertion: "ok", arguments: [
				true, "Server: Registering resource succeeded"
			] } ) );
			console.log( JSON.stringify( { ready: true } ) );
		},
		function( error ) {
			console.log( JSON.stringify( { assertion: "ok", arguments: [
				false, "Server: Registering resource failed unexpectedly: " +
					( "" + error ) + "\n" + JSON.stringify( error, null, 4 )
			] } ) );
		} );
process.on( "message", function() {
	process.exit( 0 );
} );

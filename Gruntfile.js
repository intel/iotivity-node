//
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
//
module.exports = function( grunt ) {
"use strict";

var path = require( "path" );
var suites;

require( "load-grunt-config" )( grunt, {
	configPath: [
		path.join( __dirname, "grunt-build", "tasks", "options" ),
		path.join( __dirname, "grunt-build", "tasks" )
	],
	init: true
} );

suites = process.version.match( /v0.10/ ) ? [] :
	grunt.option( "ocf-suites" ) ?
		grunt.option( "ocf-suites" ).split( "," ) : undefined;

if ( suites ) {
	grunt.config.set( "iot-js-api-ocf.plain.tests", suites );
	grunt.config.set( "iot-js-api-ocf.coverage.tests", suites );
}

require( "load-grunt-tasks" )( grunt );
grunt.task.loadNpmTasks( "iot-js-api" );

};

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

var fs = require( "fs" );

var jsonFilename = require( "path" ).resolve( __dirname, "results.json" );

var date = new Date(),
	data = "",
	caseList = [],
	setList = [],
	allList = [],
	test = {},
	testInfo = {};

exports.getTestResult = function( status, success, failure ) {

if ( status.result ) {
	success = "PASS";
} else if ( !status.result ) {
	failure = "FAIL";
}

testInfo = {
	"message": ( status.runtime + ": " + status.message ),
	"result": ( status.result ? success : failure ),
	"runtime": ( date.toLocaleTimeString() )
};

if ( setList.indexOf( status.name ) > -1 && ( "results" in test ) ) {
	test.results.push( testInfo );
	caseList.push( test );
} else {
	setList.push( status.name );
	test = { "test": status.name, "results": [ testInfo ] };
	caseList.push( test );
	allList.push( test );
}

data = JSON.stringify( { "output": allList }, null, 4 );
fs.writeFileSync( jsonFilename, data );
};

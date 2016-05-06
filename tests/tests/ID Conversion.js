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
	iotivity = require( "../../lowlevel" ),
	testUtils = require( "../utils" )( iotivity );

console.log( JSON.stringify( { assertionCount: 4 } ) );

function testBuffer( prefix, buffer, expectedResult, expectedError ) {
	var result;
	var caughtError = null;
	var destination = {};

	try {
		result = iotivity.OCConvertUuidToString( buffer, destination );
	} catch ( theError ) {
		caughtError = theError;
	}

	testUtils.assert( "deepEqual", _.extend( {}, { message: "" + caughtError }, caughtError ),
		expectedError, prefix + "error was as expected" );
	if ( !caughtError ) {
		testUtils.assert( "strictEqual", destination.sid, expectedResult,
			prefix + "Resulting string is as expected" );
		testUtils.assert( "strictEqual", testUtils.lookupEnumValueName( "OCStackResult", result ),
			"OC_STACK_OK", prefix + "Conversion completed successfully" );
	}
}

testBuffer( "Normal buffer: ", [
	-111, -91, 37, 12, 26, -44, 19, 23, 0, 14, 62, -3, 15, 22, 1, -2
], "91a5250c-1ad4-1317-000e-3efd0f1601fe", { message: "null" } );

testBuffer( "Out-of-range buffer: ", [
	-111, -91, 37, 12, 26, -44, 19, 23, 0, 14, 62, -3, 15, 22, 1, -129
], "", { message: "RangeError: SID byte value outside its range" } );

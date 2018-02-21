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

var result,
	iotivity = require( process.argv[ 3 ] + "/lowlevel" ),
	testUtils = require( "../utils" )( iotivity );

console.log( JSON.stringify( { assertionCount: 7 } ) );

// Initialize
result = iotivity.OCInit( null, 0, iotivity.OCMode.OC_CLIENT_SERVER );
testUtils.stackOKOrDie( "Client", "OCInit", result );

function testThrows( errorSource, message ) {
	var theError;

	try {
		errorSource();
	} catch ( anError ) {
		theError = anError;
	}

	testUtils.assert( "ok", !!theError,
		"Client: " + message + ": " +
			( theError ?
				( "\n" + theError.stack + ": " + JSON.stringify( theError, null, 4 ) ) : "" ) );
}

testThrows( function testUnknownPayloadType() {
	return iotivity.OCDoResource(
		{},
		iotivity.OCMethod.OC_REST_GET,
		"/some/fictional/resource",
		null,
		{ type: 999999999 },
		iotivity.OCConnectivityType.CT_DEFAULT,
		iotivity.OCQualityOfService.OC_HIGH_QOS,
		function() {
			return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
		},
		null );
}, "Unknown payload type throws an error" );

testThrows( function testUnknownRepPayloadArrayMemberType() {
	return iotivity.OCDoResource(
		{},
		iotivity.OCMethod.OC_REST_GET,
		"/some/fictional/resource",
		null,
		{
			type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
			values: {
				arrayValue: [ null ]
			}
		},
		iotivity.OCConnectivityType.CT_DEFAULT,
		iotivity.OCQualityOfService.OC_HIGH_QOS,
		function() {
			return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
		},
		null );
}, "Unknown type in array-valued representation payload throws an error" );

testThrows( function testOCDevAddrAddrOverLong() {
	return iotivity.OCDoResource(
		{},
		iotivity.OCMethod.OC_REST_GET,
		"/some/fictional/resource",
		{
			adapter: 0,
			flags: 0,
			ifindex: 0,
			port: 0,
			addr:
				"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris eget " +
				"nulla magna. Suspendisse potenti. Sed fermentum bibendum sapien et " +
				"elementum. Nam interdum sapien ut lorem gravida lobortis. Maecenas vel " +
				"pharetra erat. Curabitur viverra tempor commodo. Praesent vestibulum " +
				"felis et rhoncus ullamcorper. Integer varius enim sed auctor tempus. " +
				"Duis dignissim consectetur eleifend. Proin tristique fermentum lectus, " +
				"et sollicitudin urna cursus id. Sed eget purus sagittis est elementum " +
				"varius. Aenean euismod at velit sit amet pulvinar. Nulla nec venenatis " +
				"massa. Quisque cursus molestie risus, et commodo ante pretium mollis. " +
				"Aliquam et elementum elit, ut iaculis sapien. Morbi semper, nisl et " +
				"feugiat faucibus, libero libero sodales quam, at maximus dolor lectus " +
				"ut felis."
		},
		null,
		iotivity.OCConnectivityType.CT_DEFAULT,
		iotivity.OCQualityOfService.OC_HIGH_QOS,
		function() {},
		null );
}, "An overlong address field throws an error" );

testThrows( function testOCRDPublishHostTypeCheck() {
	return iotivity.OCRDPublish(
		{},

		// Intentionally not a string
		{ hostname: "hostname" },
		iotivity.OCConnectivityType.CT_DEFAULT,
		[],
		86400,
		function OCRDPublishResponse() {},
		iotivity.OCQualityOfService.OC_HIGH_QOS );
}, "OCRDPublish() host must be a string" );

testThrows( function testOCRDPublishResourceCountOverflow() {
	return iotivity.OCRDPublish(
		{},
		"localhost:10123",
		iotivity.OCConnectivityType.CT_DEFAULT,
		[
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},

			// More than 256
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}
		],
		86400,
		function OCRDPublishResponse() {},
		iotivity.OCQualityOfService.OC_HIGH_QOS );
}, "Too many resources given to OCRDPublish results in an error" );

testThrows( function getResourceUriHandleValidation() {
  return iotivity.OCGetResourceUri( {} );
}, "OCGetResourceUri() validates the handle" );

console.log( JSON.stringify( { killPeer: true } ) );

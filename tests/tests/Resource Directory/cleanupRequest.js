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

module.exports = function cleanupRequest( prefix, ocf, testUtils, destination, uri ) {
	testUtils.stackOKOrDie( prefix, "OCDoRequest(cleanup)",
		ocf.OCDoResource(
			{},
			ocf.OCMethod.OC_REST_POST,
			uri,
			destination,
			{
				type: ocf.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
				values: {
					killPeer: true
				}
			},
			ocf.OCConnectivityType.CT_DEFAULT,
			ocf.OCQualityOfService.OC_HIGH_QOS,
			function cleanupResponse( handle, response ) {
				console.log( JSON.stringify( { info: true, message:
					"cleanup response: " + JSON.stringify( response, null, 4 )
				} ) );
				return ocf.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
			},
			null ) );
};

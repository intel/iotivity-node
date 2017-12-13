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

function ObservationWrapper( resource ) {
  if ( !( this instanceof ObservationWrapper ) ) {
    return new ObservationWrapper( resource );
  }
  this.resource = resource;
  this.observerCount = 0;
}

ObservationWrapper.prototype.addObserver = function() {
  if ( this.observerCount === 0 ) {
    console.log( "Firing up interval" );
    var resource = this.resource;
    this.intervalId = setInterval( function() {
      resource.properties.outputValue = Math.round( Math.random() * 42 ) + 1;
      resource.notify();
    }, 1000 );
  }
  this.observerCount++;
};

ObservationWrapper.prototype.removeObserver = function() {
  this.observerCount--;
  if ( this.observerCount <= 0 ) {
    console.log( "Turning off interval" );
    clearInterval( this.intervalId );
    this.observerCount = 0;
    this.intervalId = 0;
  }
};

var observationWrapper;

function errorHandler( error ) {
  console.log( error.stack + ": " + JSON.stringify( error, null, 4 ) );
  process.exit( 1 );
}

console.log( "Acquiring OCF device" );

var ocf = require( "iotivity-node" );

ocf.device.coreSpecVersion = "ocf.1.1.0";

ocf.server
  .register( {
    resourcePath: "/a/light",
    resourceTypes: [ "core.light" ],
    interfaces: [ "oic.if.baseline" ],
    discoverable: true,
    observable: true,
    secure: true
  } )
  .then( function( resource ) {
    console.log( "Resource registered: " + JSON.stringify( resource, null, 4 ) );
    observationWrapper = ObservationWrapper( resource );
    resource
      .onretrieve( function( request ) {
        if ( "observe" in request ) {
          var observationRequest = request.observe ? "add" : "remove";
          console.log( "Observation request: " + observationRequest );
          observationWrapper[ observationRequest + "Observer" ]();
        }
        request.target.properties.outputValue =
          Math.round( Math.random() * 42 ) + 1;
        console.log( "Retrieve request received. Responding with " +
          JSON.stringify( request.target, null, 4 ) );
        request
          .respond()
          .catch( errorHandler );
      } )
      .onupdate( function( request ) {
        console.log( "Update request: " + JSON.stringify( request, null, 4 ) );
        resource.properties = request.data;
        request.respond();
      } );
  } )
  .catch( errorHandler );

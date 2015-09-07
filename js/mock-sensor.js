// This mock sensor implementation triggers an event with some data every once in a while

var emitter = require( "events" ).EventEmitter,
	possibleStringValues = [
		"Helsinki",
		"Espoo",
		"Tampere",
		"Oulu",
		"Mikkeli",
		"Ii"
	];

function randomDelay() {
	return Math.round( ( 1 + Math.random() ) * 1000 );
}

module.exports = function mockSensor() {
var returnValue = new emitter(),
	trigger = function() {
		returnValue.emit( "change", {
			someValue: Math.round( Math.random() * 42 ),
			someOtherValue:
			possibleStringValues[
				Math.round( Math.random() * possibleStringValues.length - 1 )
			]
		} );
		setTimeout( trigger, randomDelay() );
	};

setTimeout( trigger, randomDelay() );

return returnValue;
};


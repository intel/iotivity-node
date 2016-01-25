// This mock sensor implementation triggers an event with some data every once in a while

var Emitter = require( "events" ).EventEmitter,
	possibleStrings = [
		"Helsinki",
		"Espoo",
		"Tampere",
		"Oulu",
		"Mikkeli",
		"Ii"
	];

// Return a random integer between 0 and @upperLimit
function randomInteger( upperLimit ) {
	return Math.round( Math.random() * upperLimit );
}

module.exports = function mockSensor() {
var returnValue = new Emitter(),
	trigger = function() {
		var someValue = Math.round( Math.random() * 42 ),
			someOtherValue = possibleStrings[ randomInteger( possibleStrings.length - 1 ) ];

		returnValue.emit( "change", { someValue: someValue, someOtherValue: someOtherValue } );
		setTimeout( trigger, randomInteger( 1000 ) + 1000 );
	};

setTimeout( trigger, randomInteger( 1000 ) + 1000 );

return returnValue;
};


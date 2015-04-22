module.exports = function() {
	return {
		uuid: 0,
		callbacks: {},
		reverseCallbacks: {},

		add: function( callback ) {
			var uuid = this.uuid++;

			callbacks[ uuid ] = callback;
			reverseCallbacks[ callback ] = uuid;

			return uuid;
		},

		remove: function( which ) {
			var uuid, callback;

			if ( typeof which === "number" ) {
				uuid = which;
				callback = callbacks[ uuid ];
			} else {
				callback = which;
				uuid = reverseCallbacks[ callback ];
			}

			if ( !( uuid === undefined || callback === undefined ) ) {
				delete callbacks[ uuid ];
				delete reverseCallbacks[ callback ];
			}
		}
	};
};

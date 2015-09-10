module.exports = function( grunt ) {

var exec = require( "child_process" ).exec;

grunt.task.registerTask( "clangformat", "Format the C++ files", function() {
	var done = this.async();

	exec( "find src -type f | while read; do " +
		"clang-format -style=Google \"$REPLY\" > \"$REPLY\".new && " +
		"mv \"$REPLY\".new \"$REPLY\";" +
		"done", function( error ) {
			done.apply( this, error ? [ false ] : [] );
		} );
} );

};

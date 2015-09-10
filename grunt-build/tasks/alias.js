module.exports = function( grunt ) {

grunt.registerTask( "default", [ "jshint", "jscs", "test" ] );

grunt.registerTask( "format", [ "esformatter", "clangformat" ] );

};

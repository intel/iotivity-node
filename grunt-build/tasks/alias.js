module.exports = function( grunt ) {

grunt.registerTask( "default", [ "test" ] );

grunt.registerTask( "test", [ "jshint", "jscs", "testsuite" ] );

grunt.registerTask( "format", [ "esformatter", "clangformat" ] );

};

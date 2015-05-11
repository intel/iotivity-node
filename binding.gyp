{
	"variables": {
		"IOTIVITY_PATH": '<!(echo "$IOTIVITY_PATH")',
		"LIBOCTBSTACK_PATH": '<!(dirname $( find "$IOTIVITY_PATH/out" -name liboctbstack.so -print -quit ))'
	},
	"target_defaults": {
		"include_dirs": [
			"<( IOTIVITY_PATH )/resource/csdk/stack/include",
			"<( IOTIVITY_PATH )/resource/csdk/ocsocket/include"
		],
		"libraries": [
			'<(LIBOCTBSTACK_PATH)/liboctbstack.so',
			'-Wl,-rpath,<(LIBOCTBSTACK_PATH)',
		],
	},
	"targets": [
		{
			"target_name": "iotivity",
			"sources": [
				"src/main.cc",
				"src/enums.cc",
				"src/structures.cc",
				"src/functions/oc-create-delete-resource.cc",
				"src/functions/oc-do-resource.cc",
				"src/functions/oc-do-response.cc",
				"src/functions/simple.cc",
				"src/functions.cc",
				"src/callback-info.c"
			],
			"libraries": [ "<(PRODUCT_DIR)/libffi.a" ],
			"conditions": [
				[ "'<!(echo $TESTING)'=='true'",
						{ "defines": [ "TESTING" ] } ]
			],
			"dependencies": [
				"node_modules/ffi/deps/libffi/libffi.gyp:ffi"
			]
		},
		{
			"target_name": "client",
			"type": "executable",
			"sources": [ "c/client.c" ],
			"libraries": [ '<!(pkg-config --libs glib-2.0)' ],
			"cflags": [ '<!(pkg-config --cflags glib-2.0)' ]
		},
		{
			"target_name": "server",
			"type": "executable",
			"sources": [ "c/server.c" ],
			"libraries": [ '<!(pkg-config --libs glib-2.0)' ],
			"cflags": [ '<!(pkg-config --cflags glib-2.0)' ]
		},
		{
			"target_name": "server.observable",
			"type": "executable",
			"sources": [ "c/server.observable.c" ],
			"libraries": [ '<!(pkg-config --libs glib-2.0)' ],
			"cflags": [ '<!(pkg-config --cflags glib-2.0)' ]
		}
	]
}

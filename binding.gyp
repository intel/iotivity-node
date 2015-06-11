{
	"target_defaults": {
		"include_dirs": [
			"<!(node -e \"require('nan')\")"
		],
		"libraries": [ '<!@(pkg-config --libs octbstack)' ],
		"cflags": [
			'<!(pkg-config --cflags octbstack)'
		],
		"cflags_cc": [ '-std=c++11' ],
		"xcode_settings": {
			"OTHER_CFLAGS": ['<!(pkg-config --cflags octbstack)']
		}
	},
	"targets": [
		{
			"target_name": "iotivity",
			"sources": [
				"src/main.cc",
				"src/common.cc",
				"src/enums.cc",
				"src/constants.cc",
				"src/structures.cc",
				"src/functions/oc-create-delete-resource.cc",
				"src/functions/oc-do-resource.cc",
				"src/functions/oc-do-response.cc",
				"src/functions/oc-notify.cc",
				"src/functions/simple.cc",
				"src/functions.cc",
				"src/callback-info.c"
			],
			"libraries": [ "<(PRODUCT_DIR)/libffi.a" ],
			"xcode_settings": {
				"OTHER_LDFLAGS": [
					'-loctbstack', '-locsrm', '-lconnectivity_abstraction', '-lc_common', '-lcoap'
				]
			},
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
			"cflags": [ '<!(pkg-config --cflags glib-2.0)' ],
			"xcode_settings": {
				"OTHER_CFLAGS": ['<!(pkg-config --cflags glib-2.0)'],
				"OTHER_LDFLAGS": [
					'-loctbstack', '-locsrm', '-lconnectivity_abstraction', '-lc_common', '-lcoap'
				]
			}
		},
		{
			"target_name": "client.observe",
			"type": "executable",
			"sources": [ "c/client.observe.c" ],
			"libraries": [ '<!(pkg-config --libs glib-2.0)' ],
			"cflags": [ '<!(pkg-config --cflags glib-2.0)' ],
			"xcode_settings": {
				"OTHER_CFLAGS": ['<!(pkg-config --cflags glib-2.0)'],
				"OTHER_LDFLAGS": [
					'-loctbstack', '-locsrm', '-lconnectivity_abstraction', '-lc_common', '-lcoap'
				]
			}
		},
		{
			"target_name": "server",
			"type": "executable",
			"sources": [ "c/server.c" ],
			"libraries": [ '<!(pkg-config --libs glib-2.0)' ],
			"cflags": [ '<!(pkg-config --cflags glib-2.0)' ],
			"xcode_settings": {
				"OTHER_CFLAGS": ['<!(pkg-config --cflags glib-2.0)'],
				"OTHER_LDFLAGS": [
					'-loctbstack', '-locsrm', '-lconnectivity_abstraction', '-lc_common', '-lcoap'
				]
			}
		},
		{
			"target_name": "server.observable",
			"type": "executable",
			"sources": [ "c/server.observable.c" ],
			"libraries": [ '<!(pkg-config --libs glib-2.0)' ],
			"cflags": [ '<!(pkg-config --cflags glib-2.0)' ],
			"xcode_settings": {
				"OTHER_CFLAGS": ['<!(pkg-config --cflags glib-2.0)'],
				"OTHER_LDFLAGS": [
					'-loctbstack', '-locsrm', '-lconnectivity_abstraction', '-lc_common', '-lcoap'
				]
			}
		}
	]
}

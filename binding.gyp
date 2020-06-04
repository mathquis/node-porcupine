{
	'targets': [{
		'target_name': 'porcupine',
		"cflags!": [ "-fno-exceptions" ],
		"cflags_cc!": [ "-fno-exceptions" ],
		'include_dirs': [
			"<!@(node -p \"require('node-addon-api').include\")",
			"./src/porcupine/include"
		],
		'sources': [
			'./src/porcupine.cpp'
		],
		'dependencies': [
            'src/dlfcn/dlfcn.gyp:dlfcn'
        ],
        "cflags_cc": [
			"-std=c++11",
			"-Wall",
			"-Winit-self"
		],
		"cflags": [
		],
		"library_dirs": [
			'<(module_root_dir)/src/porcupine/lib/windows/amd64/'
		],
		'libraries': [
			# '-ldl'
		],
		'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
	},
	{
	  "target_name": "action_after_build",
	  "type": "none",
	  "dependencies": [ "<(module_name)" ],
	  "copies": [
		{
		  "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
		  "destination": "<(module_path)"
		}
	  ]
	}
	]
}
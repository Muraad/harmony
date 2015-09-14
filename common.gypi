{
    'target_defaults': {
        #'default_configuration': 'Release',
        'default_configuration': 'Debug',
        'cflags_cc': [ 
			'-std=c++1y',
			'-fvisibility-inlines-hidden',
			'-fexceptions',
			'-frtti',
		],
        'cflags': [
			'-std=c++1y',
			'-Wall',
			'-Wno-pedantic',
			'-Wno-long-long',
			'-fvisibility=hidden',
			'-fomit-frame-pointer',
			'-fdata-sections',
			'-ffunction-sections',
            '-DGOOGLE_PROTOBUF_NO_THREAD_SAFETY',
		],
        'configurations': {
            'Debug': {
                'defines': [ 'DEBUG' ],
                'cflags' : [ 
					'-g',
					'-O0',
				],
            },
            'Release': {
                'defines': [ 'NDEBUG' ],
                'cflags': [
                    '-Os',
                ],
				'ldflags': [
                    '-Os',
					'-Wl,--exclude-libs,ALL',
					'-Wl,--gc-sections',
				],
                'xcode_settings': {
                    'DEAD_CODE_STRIPPING': 'YES',
                },
            },
        },
        'xcode_settings': {
            'OTHER_CFLAGS' : [
				'-Wall',
				'-fvisibility=hidden',
			],
            'OTHER_CPLUSPLUSFLAGS' : [
				'-Wall',
				'-fvisibility=hidden',
				'-DUSE_SQLITE',
				'-Dgoogle=_t_',
				'-Dprotobuf=_p_',
				'-fdata-sections',
				'-ffunction-sections',
                    		'-Os',
			],
            'CLANG_CXX_LANGUAGE_STANDARD': 'c++11',
            'CLANG_CXX_LIBRARY': 'libc++',
            'CLANG_ENABLE_OBJC_ARC': 'YES',
        },
        'conditions': [
            ['OS=="ios"', {
                'xcode_settings' : {
                    'SDKROOT': 'iphoneos',
                    'SUPPORTED_PLATFORMS': 'iphonesimulator iphoneos',
                },
            }],
            ['OS=="mac"', {
                'xcode_settings' : {
                    'SDKROOT': 'macosx10.10',
                },
            }],
        ],
    },
}

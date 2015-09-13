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
            },
        },
        'conditions': [
        ],
    },
}

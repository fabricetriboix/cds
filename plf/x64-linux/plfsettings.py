def getPlfSettings(variantNames):
    settings = {}

    # Common settings for all variants
    for v in variantNames:
        settings[v] = {}
        settings[v]['path'] = ['/usr/local/bin', '/bin', '/usr/bin']
        settings[v]['cc'] = 'gcc'
        settings[v]['ar'] = 'ar'
        settings[v]['ranlib'] = 'ranlib'
        settings[v]['cpppath'] = []
        settings[v]['cppdefines'] = [{'_GNU_SOURCE': '1'}]
        settings[v]['ccflags'] = ['-Wall', '-Wextra', '-Werror',
                '-std=c99', '-pthread']
        settings[v]['linkflags'] = ['-pthread']
        settings[v]['libpath'] = []

    # Customisation per variant
    settings['debug']['ccflags'].extend(['-O0', '-g'])
    settings['release']['ccflags'].extend(['-O3'])
    return settings

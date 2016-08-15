def GetPlfSettings(variantNames):
    settings = {}

    # Common settings for all variants
    for v in variantNames:
        settings[v] = {}
        settings[v]['path'] = ['/usr/local/bin', '/bin', '/usr/bin']
        settings[v]['cc'] = 'gcc'
        settings[v]['cxx'] = 'g++'
        settings[v]['ar'] = 'ar'
        settings[v]['ranlib'] = 'ranlib'
        settings[v]['cpppath'] = []
        settings[v]['cppdefines'] = [{'_GNU_SOURCE': '1'}]
        settings[v]['ccflags'] = ['-Wall', '-Wextra', '-Werror', '-std=c99']
        settings[v]['cxxflags'] = ['-Wall', '-Wextra', '-Werror', '-std=c++11']
        settings[v]['linkflags'] = []
        settings[v]['libpath'] = []

        # Default installation directories
        settings[v]['prefix'] = "/usr/local"
        settings[v]['bindir'] = "bin"
        settings[v]['libdir'] = "lib"
        settings[v]['incdir'] = "include"
        settings[v]['docdir'] = "doc/cds"
        settings[v]['etcdir'] = "etc"
        settings[v]['shrdir'] = "share"
        settings[v]['vardir'] = "var"

        # Customisation per variant
        if v == "debug":
            settings[v]['cppdefines'].append({'CDS_ENABLE_MTRACE': '1'})
            settings[v]['ccflags'].extend(['-O0', '-g'])
            settings[v]['cxxflags'].extend(['-O0', '-g'])
        elif v == "release":
            settings[v]['ccflags'].extend(['-O3'])
            settings[v]['cxxflags'].extend(['-O3'])

    return settings

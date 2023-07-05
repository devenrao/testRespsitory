cxx = meson.get_compiler('cpp')

sdbusplus = dependency(
    'sdbusplus',
    fallback: [
        'sdbusplus',
        'sdbusplus_dep'
    ],  
)

systemd = dependency('systemd')
libguard = cpp.find_library('libguard')

phosphor_logging = dependency(
    'phosphor-logging',
    fallback: ['phosphor-logging', 'phosphor_logging_dep'],
    )

executable(
    'guard',
    'guard.cpp',
    dependencies: [ sdbusplus, systemd, phosphor_logging, libguard ],
)

cxx = meson.get_compiler('cpp')

sdbusplus = dependency(
    'sdbusplus',
    fallback: [
        'sdbusplus',
        'sdbusplus_dep'
    ],  
)

systemd = dependency('systemd')

phosphor_logging = dependency(
    'phosphor-logging',
    fallback: ['phosphor-logging', 'phosphor_logging_dep'],
    )

executable(
    'pwrerr',
    'pwrerr.cpp',
    dependencies: [ sdbusplus, systemd, phosphor_logging ],
)

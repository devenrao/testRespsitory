cxx = meson.get_compiler('cpp')

sdbusplus = dependency(
    'sdbusplus',
    fallback: [
        'sdbusplus',
        'sdbusplus_dep'
    ],  
)
sdeventplus = dependency(
    'sdeventplus',
    fallback: [
        'sdeventplus',
        'sdeventplus_dep'
    ],  
)

systemd = dependency('systemd')

executable(
    'watch',
    'watch.cpp',
    dependencies: [ sdbusplus, systemd, sdeventplus ],
)

cxx = meson.get_compiler('cpp')

sdbusplus = dependency(
    'sdbusplus',
    fallback: [
        'sdbusplus',
        'sdbusplus_dep'
    ],  
)

systemd = dependency('systemd')

executable(
    'coroutine_example',
    'coroutine_example.cpp',
    dependencies: [ sdbusplus, systemd ],
)

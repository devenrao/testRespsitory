sdbusplus_dep = dependency(
    'sdbusplus',
    fallback: ['sdbusplus', 'sdbusplus_dep'],
)

sdeventplus_dep = dependency(
    'sdeventplus',
    fallback: ['sdeventplus', 'sdeventplus_dep'],
)

phosphor_dbus_interfaces_dep = dependency(
    'phosphor-dbus-interfaces',
    fallback: [
        'phosphor-dbus-interfaces',
        'phosphor_dbus_interfaces_dep'
    ],
)

phosphor_logging_dep = dependency(
    'phosphor-logging',
    fallback: ['phosphor-logging', 'phosphor_logging_dep'],
)

systemd_dep = dependency('systemd')
pldm_dep = dependency('libpldm')

configure_file(
    input: 'config.h.in',
    output: 'config.h',
    configuration: configuration_data(),
)

backend_switch_deps = [
    phosphor_logging_dep,
    sdbusplus_dep,
    sdeventplus_dep,
    pldm_dep,
]

subdir('dist')

executable(
    'backend_switch',
    'sbe_fsi_backend_swtich.cpp',
    dependencies: backend_switch_deps,
    install: true,
)

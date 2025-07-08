if get_option('datafiles').allowed()
  install_data('bluetooth-mesh.conf',
    install_dir: dbusdir)
  install_data('mesh-main.conf',
    install_dir: configdir)
endif

if systemd_dep.found()
  configure_file(
    input: 'bluetooth-mesh.service.in',
    output: 'bluetooth-mesh.service',
    install: true,
    install_dir: systemdsystemunitdir,
    configuration: service_conf
  )
  install_data('org.bluez.mesh.service',
    install_dir: dbussystembusdir
  )
endif

executable('bluetooth-meshd',
  sources: [
    builtin_h,
    'main.c',
    'mesh.c',
    'net-keys.c',
    'mesh-io.c',
    'mesh-mgmt.c',
    'mesh-io-unit.c',
    'mesh-io-mgmt.c',
    'mesh-io-generic.c',
    'net.c',
    'crypto.c',
    'friend.c',
    'appkey.c',
    'node.c',
    'model.c',
    'cfgmod-server.c',
    'remprv-server.c',
    'mesh-config-json.c',
    'util.c',
    'dbus.c',
    'agent.c',
    'prov-acceptor.c',
    'prov-initiator.c',
    'manager.c',
    'pb-adv.c',
    'keyring.c',
    'rpl.c',
    'prvbeac-server.c'
  ],
  include_directories: '../lib',
  dependencies: [ libshared_ell_dep, jsonc_dep ],
  c_args: [
    '-DCONFIGDIR="@0@"'.format(configdir),
    '-DMESH_STORAGEDIR="@0@"'.format(meshstoragedir)
  ],
  install: true,
  install_dir: pkglibexecdir
)

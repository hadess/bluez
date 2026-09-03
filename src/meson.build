subdir('shared')

if get_option('daemon').allowed()
  if get_option('datafiles').allowed()
    install_data('bluetooth.conf',
      install_dir: dbusconfdir / 'dbus-1/system.d'
    )
    install_data('main.conf',
      install_dir: configdir
    )
  endif

  if systemd_dep.found()
    configure_file(
      input: 'bluetooth.service.in',
      output: 'bluetooth.service',
      install: true,
      install_dir: systemdsystemunitdir,
      configuration: service_conf
    )

    install_data('org.bluez.service',
      install_dir: dbussystembusdir
    )
  endif

  genbuiltin = find_program('genbuiltin')
  builtin_h = custom_target('src-builtin.h',
    output: 'builtin.h',
    capture: true,
    command: [ genbuiltin, builtin_modules ]
  )

  bluetoothd_internal_sources = files(
    'log.c',
    'backtrace.c',
    'rfkill.c',
    'sdpd-server.c',
    'sdpd-request.c',
    'sdpd-service.c',
    'sdpd-database.c',
    'gatt-database.c',
    'sdp-xml.c',
    'sdp-client.c',
    'textfile.c',
    'uuid-helper.c',
    'storage.c',
    'advertising.c',
    'agent.c',
    'error.c',
    'adapter.c',
    'profile.c',
    'bearer.c',
    'service.c',
    'gatt-client.c',
    'device.c',
    'dbus-common.c',
    'eir.c',
    'adv_monitor.c',
    'battery.c',
    'settings.c',
    'set.c',
    'ranging.c'
  )

  executable('bluetoothd',
    sources: [
      builtin_sources,
      builtin_h,
      attrib_sources,
      btio_sources,
      bluetoothd_internal_sources,
      'main.c',
      'plugin.c',
    ],
    dependencies: [
      libbluetooth_internal_dep,
      libgdbus_internal_dep,
      libshared_glib_dep,
      backtrace_dep,
      glib_dep,
      dbus_dep,
      udevlib_dep,
      builtin_deps
    ],
    override_options: [ 'b_lundef=false' ],
    c_args: [
      '-DBLUETOOTH_PLUGIN_BUILTIN',
      '-DPLUGINDIR="@0@"'.format(plugindir),
      '-DSTORAGEDIR="@0@"'.format(storagedir),
      '-DCONFIGDIR="@0@"'.format(configdir)
    ],
    link_args: [ '-Wl,--gc-sections', external_plugins_link_args ],
    install: true,
    install_dir: pkglibexecdir
  )
endif

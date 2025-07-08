if get_option('client').enabled()
  executable('bluetoothctl',
    sources: [
      'main.c',
      'print.c',
      'display.c',
      'agent.c',
      'advertising.c',
      'adv_monitor.c',
      'gatt.c',
      'admin.c',
      'player.c',
      'mgmt.c',
      'assistant.c',
      'hci.c',
      'telephony.c'
      ],
    dependencies: [
      libbluetooth_internal_dep,
      libgdbus_internal_dep,
      libshared_glib_dep,
      glib_dep,
      udevlib_dep,
      dbus_dep,
      readline_dep
      ],
    install: true,
  )
endif

if get_option('btpclient').allowed() and ell_dep.found()
  subdir('btpclient')
endif

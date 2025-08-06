if get_option('logger').allowed()
  executable('btmon-logger',
    [ 'btmon-logger.c' ],
    dependencies: libshared_mainloop_dep,
    include_directories: '../lib/',
    install_dir: pkglibexecdir
  )

  if systemd_dep.found()
    configure_file(
      input: 'bluetooth-logger.service.in',
      output: 'bluetooth-logger.service',
      configuration: service_in,
      install: true,
      install_dir: systemdsystemunitdir
    )
  endif
endif

executable('3dsp',
  sources: '3dsp.c',
  dependencies: libshared_mainloop_dep
)

tester_tools = [
  'mgmt',
  'mesh',
  'l2cap',
  'rfcomm',
  'bnep',
  'smp',
  'gap',
  'sco',
  'userchan',
  'iso',
  'ioctl',
  'hci',
  '6lowpan',
]

if get_option('tests').allowed()
  foreach tool: tester_tools
    executable(tool + '-tester',
      sources: [ tool + '-tester.c', emulator_sources ],
      dependencies: [ libbluetooth_internal_dep, libshared_glib_dep, libgdbus_internal_dep, glib_dep ]
    )
  endforeach
endif

if udevlib_hid2hci_dep.found()
  executable('hid2hci',
    sources: 'hid2hci.c',
    dependencies: udevlib_hid2hci_dep,
    install: true,
    install_dir: udevdir
  )
  install_data('hid2hci.rules',
    install_dir: udevdir / 'rules.d',
    rename: '97-hid2hci.rules'
  )
endif

tools_names = [
  'avinfo',
  'avtest',
  'scotest',
  'hwdb',
  'hcieventmask',
  'btinfo',
  'btconfig',
  'btsnoop',
  'btproxy',
  'cltest',
  'oobtest',
  'advtest',
  'seq2bseq',
  'nokfw',
  'rtlfw',
  'create-image',
  'eddystone',
  'ibeacon',
]

inst_tools_names = [
  'rctest',
  'l2test',
  'l2ping',
  'bluemoon',
  'hex2hcd',
  'mpris-proxy',
  'btattach',
  'isotest'
]

if get_option('tools').allowed()
  foreach tool: tools_names
    executable(tool,
      sources: tool + '.c',
      dependencies: [ libbluetooth_internal_dep, libshared_mainloop_dep ]
    )
  endforeach

  foreach tool: inst_tools_names
    executable(tool,
      sources: tool + '.c',
      dependencies: [ libbluetooth_internal_dep, libshared_mainloop_dep, libgdbus_internal_dep, dbus_dep, glib_dep ],
      install: true
    )
  endforeach

  if udevlib_dep.found()
    executable('bdaddr',
      sources: [ 'bdaddr.c', '../src/oui.c' ],
      dependencies: [ libbluetooth_internal_dep, udevlib_dep ]
    )
  endif
  executable('btiotest',
    sources: [ 'btiotest.c', btio_sources ],
    dependencies: [ libbluetooth_internal_dep, glib_dep ]
  )
  executable('bneptest',
    sources: [ 'bneptest.c', btio_sources, '../src/log.c', '../profiles/network/bnep.c' ],
    dependencies: [ libbluetooth_internal_dep, libshared_mainloop_dep, glib_dep ]
  )
  executable('btgatt-client',
    sources: [ 'btgatt-client.c', '../src/uuid-helper.c' ],
    dependencies: [ libbluetooth_internal_dep, libshared_mainloop_dep ]
  )
  executable('btgatt-server',
    sources: [ 'btgatt-server.c', '../src/uuid-helper.c' ],
    dependencies: [ libbluetooth_internal_dep, libshared_mainloop_dep ]
  )
  if udevlib_dep.found()
    executable('gatt-service',
      sources: 'gatt-service.c',
      dependencies: [ libbluetooth_internal_dep, libshared_mainloop_dep, glib_dep, libgdbus_internal_dep, udevlib_dep ]
    )
  endif
  executable('test-runner',
    sources: 'test-runner.c',
    dependencies: [ libbluetooth_internal_dep ]
  )

  if readline_dep.found()
    executable('btmgmt',
      sources: [ 'btmgmt.c', '../src/uuid-helper.c', '../client/display.c', '../client/mgmt.c' ],
      dependencies: [ libbluetooth_internal_dep, libshared_glib_dep, readline_dep ]
    )
    executable('obex-client-tool',
      sources: [ 'obex-client-tool.c', gobex_sources, btio_sources ],
      dependencies: [ libbluetooth_internal_dep, libshared_glib_dep, readline_dep, glib_dep ]
    )
    executable('obex-server-tool',
      sources: [ 'obex-server-tool.c', gobex_sources, btio_sources ],
      dependencies: [ libbluetooth_internal_dep, libshared_glib_dep, glib_dep ]
    )
    executable('bluetooth-player',
      sources: [ 'bluetooth-player.c', '../client/print.c', '../client/player.c' ],
      dependencies: [ libbluetooth_internal_dep, libshared_glib_dep, libgdbus_internal_dep, readline_dep ]
    )
    executable('obexctl',
      sources: [ 'obexctl.c' ],
      dependencies: [ libbluetooth_internal_dep, libshared_glib_dep, libgdbus_internal_dep, readline_dep ]
    )
    if get_option('mesh').enabled()
      executable('mesh-cfgtest',
        sources: 'mesh-cfgtest.c',
        dependencies: [ libbluetooth_internal_dep, dbus_dep, ell_dep ],
        include_directories: '..',
        install: true
      )
      if get_option('deprecated').allowed()
        executable('meshctl',
          sources: [
            'meshctl.c',
            'mesh/agent.c',
            'mesh-gatt/node.c',
            'mesh-gatt/gatt.c',
            'mesh-gatt/crypto.c',
            'mesh-gatt/net.c',
            'mesh-gatt/prov.c',
            'mesh-gatt/util.c',
            'mesh-gatt/prov-db.c',
            'mesh-gatt/config-client.c',
            'mesh-gatt/config-server.c',
            'mesh-gatt/onoff-model.c'
            ],
          dependencies: [
            libgdbus_internal_dep,
            libshared_glib_dep,
            libbluetooth_internal_dep,
            glib_dep,
            dbus_dep,
            jsonc_dep,
            readline_dep
            ],
          install: true
        )
      endif
    endif
  endif

  if get_option('deprecated').allowed()
    executable('gatttool',
      sources: gatttool_sources,
      dependencies: [ libbluetooth_internal_dep, libshared_glib_dep, readline_dep ],
      install: true
    )
    executable('hciattach',
      sources: [
        'hciattach.c',
        'hciattach_st.c',
        'hciattach_ti.c',
        'hciattach_tialt.c',
        'hciattach_ath3k.c',
        'hciattach_qualcomm.c',
        'hciattach_intel.c',
        'hciattach_bcm43xx.c'
      ],
      dependencies: libbluetooth_internal_dep,
      install: true
    )
    if udevlib_dep.found()
      executable('hcitool',
        sources: [ 'hcitool.c', '../src/oui.c' ],
        dependencies: [ libbluetooth_internal_dep, udevlib_dep ],
        install: true
      )
    endif
    executable('sdptool',
      sources: [ 'sdptool.c', '../src/sdp-xml.c' ],
      dependencies: [ libbluetooth_internal_dep, glib_dep ],
      install: true
    )
    deprecated_tools = [
      'ciptool',
      'hciconfig',
      'rfcomm'
    ]
    foreach tool: deprecated_tools
      executable(tool,
        sources: tool + '.c',
        dependencies: libbluetooth_internal_dep,
        install: true
      )
    endforeach
  endif

  if systemd_dep.found()
    configure_file(
      input: 'mpris-proxy.service.in',
      output: 'mpris-proxy.service',
      configuration: service_in,
      install: true,
      install_dir: systemduserunitdir
    )
  endif
endif

if get_option('mesh').enabled()
  subdir('mesh')
endif

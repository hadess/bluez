builtin_modules += [
  'gap',
  'scanparam',
  'deviceinfo',
  'battery',
  'rap',
]

builtin_sources += files(
  'audio/media.c',
  'audio/transport.c',
  'audio/player.c',
  'audio/avrcp-player.c',
  'gap/gas.c',
  'scanparam/scan.c',
  'deviceinfo/deviceinfo.c',
  'battery/battery.c',
  'ranging/rap.c',
  'ranging/rap_hci.c',
)

if get_option('a2dp').allowed()
  builtin_modules += 'a2dp'
  builtin_sources += files(
    'audio/source.c',
    'audio/sink.c',
    'audio/a2dp.c',
    'audio/avdtp.c'
  )
endif

if get_option('avrcp').allowed()
  builtin_modules += 'avrcp'
  builtin_sources += files(
    'audio/control.c',
    'audio/avctp.c',
    'audio/avctp.c',
    'audio/avrcp.c'
  )
endif

if get_option('network').allowed()
  builtin_modules += 'network'
  builtin_sources += files(
    'network/manager.c',
    'network/bnep.c',
    'network/server.c',
    'network/connection.c'
  )
  install_data('network/network.conf',
    install_dir: configdir
  )
endif

if get_option('hid').allowed()
  builtin_modules += 'input'
  builtin_sources += files(
    'input/manager.c',
    'input/server.c',
    'input/device.c'
  )

  install_data('input/input.conf',
    install_dir: configdir
  )
endif

if get_option('hog').allowed()
  builtin_modules += 'hog'
  builtin_sources += files(
    'input/hog.c',
    'input/hog-lib.c',
    'deviceinfo/dis.c',
    'battery/bas.c',
    'scanparam/scpp.c',
    'input/suspend-none.c'
  )
endif

if alsa_dep.found()
  builtin_modules += 'midi'
  builtin_sources += files(
    'midi/midi.c',
    'midi/libmidi.c'
  )
  builtin_deps += alsa_dep
endif

if get_option('bap').allowed()
  builtin_modules += 'bap'
  builtin_sources += files('audio/bap.c')
endif

if get_option('bass').allowed()
  builtin_modules += 'bass'
  builtin_sources += files('audio/bass.c')
endif

if get_option('mcp').allowed()
  builtin_modules += 'mcp'
  builtin_sources += files('audio/mcp.c')
endif

if get_option('vcp').allowed()
  builtin_modules += 'vcp'
  builtin_sources += files('audio/vcp.c')
endif

if get_option('micp').allowed()
  builtin_modules += 'micp'
  builtin_sources += files('audio/micp.c')
endif

if get_option('ccp').allowed()
  builtin_modules += 'ccp'
  builtin_sources += files('audio/ccp.c')
endif

if get_option('csip').allowed()
  builtin_modules += 'csip'
  builtin_sources += files('audio/csip.c')
endif

if get_option('asha').allowed()
  builtin_modules += 'asha'
  builtin_sources += files('audio/asha.c')
endif

if get_option('tmap').allowed()
  builtin_modules += 'tmap'
  builtin_sources += files('audio/tmap.c')
endif

if get_option('gmap').allowed()
  builtin_modules += 'gmap'
  builtin_sources += files('audio/gmap.c')
endif

if get_option('hfp').allowed()
  builtin_modules += 'hfp'
  builtin_sources += files('audio/telephony.c', 'audio/hfp-hf.c')
endif

if get_option('tools').allowed()
  executable('iapd',
    sources: 'iap/main.c',
    dependencies: [ libgdbus_internal_dep, glib_dep, dbus_dep ]
  )
endif

if cups_dep.found()
  executable('bluetooth',
    sources: [
      'cups/main.c',
      'cups/sdp.c',
      'cups/spp.c',
      'cups/hcrp.c'
    ],
    dependencies: [
      libbluetooth_internal_dep,
      libgdbus_internal_dep,
      glib_dep,
      dbus_dep
    ],
    install: true,
    install_dir: cups_serverbin / 'backend'
  )
endif

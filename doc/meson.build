man_pages = [
  'hci.7',
  'iso.7',
  'mgmt.7',
  'l2cap.7',
  'rfcomm.7',
  'sco.7',
  'btsnoop.7',
  'org.bluez.Adapter.5',
  'org.bluez.Bearer.LE.5',
  'org.bluez.Bearer.BREDR.5',
  'org.bluez.Device.5',
  'org.bluez.DeviceSet.5',
  'org.bluez.AgentManager.5',
  'org.bluez.Agent.5',
  'org.bluez.ProfileManager.5',
  'org.bluez.Profile.5',
  'org.bluez.NetworkServer.5',
  'org.bluez.Network.5',
  'org.bluez.Input.5',
  'org.bluez.BatteryProviderManager.5',
  'org.bluez.BatteryProvider.5',
  'org.bluez.Battery.5',
  'org.bluez.AdminPolicySet.5',
  'org.bluez.AdminPolicyStatus.5',
  'org.bluez.Media.5',
  'org.bluez.MediaControl.5',
  'org.bluez.MediaPlayer.5',
  'org.bluez.MediaFolder.5',
  'org.bluez.MediaItem.5',
  'org.bluez.MediaEndpoint.5',
  'org.bluez.MediaTransport.5',
  'org.bluez.MediaAssistant.5',
  'org.bluez.GattManager.5',
  'org.bluez.GattProfile.5',
  'org.bluez.GattService.5',
  'org.bluez.GattCharacteristic.5',
  'org.bluez.GattDescriptor.5',
  'org.bluez.LEAdvertisingManager.5',
  'org.bluez.LEAdvertisement.5',
  'org.bluez.AdvertisementMonitorManager.5',
  'org.bluez.AdvertisementMonitor.5',
  'org.bluez.Telephony.5',
  'org.bluez.Call.5',
  'org.bluez.ThermometerManager.5',
  'org.bluez.Thermometer.5',
  'org.bluez.ThermometerWatcher.5',
]
doc_man_pages = man_pages

obex_man_pages = [
  'org.bluez.obex.Client.5',
  'org.bluez.obex.Session.5',
  'org.bluez.obex.Transfer.5',
  'org.bluez.obex.ObjectPush.5',
  'org.bluez.obex.FileTransfer.5',
  'org.bluez.obex.Synchronization.5',
  'org.bluez.obex.PhonebookAccess.5',
  'org.bluez.obex.MessageAccess.5',
  'org.bluez.obex.Message.5',
  'org.bluez.obex.AgentManager.5',
  'org.bluez.obex.Agent.5',
  'org.bluez.obex.Image.5',
]
doc_man_pages += obex_man_pages

if get_option('obex').enabled()
  man_pages += obex_man_pages
endif

monitor_man_pages = [ 'btmon.1' ]
doc_man_pages += monitor_man_pages

if get_option('monitor').allowed()
  man_pages += monitor_man_pages
endif

tools_man_pages = [
  'rctest.1',
  'l2ping.1',
  'btattach.1',
  'isotest.1',
  'bdaddr.1',
]
doc_man_pages += tools_man_pages

noinst_tools_man_pages = [ 'btmgmt.1' ]
doc_man_pages += noinst_tools_man_pages

if get_option('tools').allowed()
  man_pages += tools_man_pages
  man_pages += noinst_tools_man_pages
endif

deprecated_tools_man_pages = [
  'hciattach.1',
  'hciconfig.1',
  'hcitool.1',
  'rfcomm.1',
  'sdptool.1',
  'ciptool.1',
]
doc_man_pages += deprecated_tools_man_pages

if get_option('deprecated').allowed()
  man_pages += deprecated_tools_man_pages
endif

hid2hci_man_pages = [ 'hid2hci.1' ]
doc_man_pages += hid2hci_man_pages

if get_option('hid2hci').enabled()
  man_pages += 'hid2hci.1'
endif

client_man_pages = [
  'bluetoothctl.1',
  'bluetoothctl-mgmt.1',
  'bluetoothctl-monitor.1',
  'bluetoothctl-admin.1',
  'bluetoothctl-advertise.1',
  'bluetoothctl-endpoint.1',
  'bluetoothctl-gatt.1',
  'bluetoothctl-player.1',
  'bluetoothctl-scan.1',
  'bluetoothctl-transport.1',
  'bluetoothctl-assistant.1',
  'bluetoothctl-hci.1',
  'bluetoothctl-telephony.1'
]
doc_man_pages += client_man_pages

if get_option('client').enabled()
  man_pages += client_man_pages
endif

if rst2man.found()
  foreach man_page: man_pages
    man_page_name = man_page.substring(0, -2)
    man_section = man_page.substring(-1)
    if man_section.to_int() == 7
      man_page_name = man_page_name + '-protocol'
    endif
    install = not noinst_tools_man_pages.contains(man_page)
    custom_target(man_page,
      input: man_page_name + '.rst',
      output: man_page,
      command: rst2man_command,
      install: install,
      install_dir: get_option('mandir') / 'man@0@'.format(man_section)
    )
  endforeach
endif

qualifications_docs = [
  'qualification/a2dp-pics.rst',
  'qualification/a2dp-pts.rst',
  'qualification/avctp-pics.rst',
  'qualification/avctp-pts.rst',
  'qualification/avdtp-pics.rst',
  'qualification/avdtp-pts.rst',
  'qualification/avrcp-pics.rst',
  'qualification/avrcp-pts.rst',
  'qualification/gap-pics.rst',
  'qualification/gap-pts.rst',
  'qualification/gatt-pics.rst',
  'qualification/gatt-pts.rst',
  'qualification/gavdp-pics.rst',
  'qualification/gavdp-pts.rst',
  'qualification/hid-pics.rst',
  'qualification/hid-pts.rst',
  'qualification/hogp-pics.rst',
  'qualification/hogp-pts.rst',
  'qualification/hsp-pics.rst',
  'qualification/hsp-pts.rst',
  'qualification/iopt-pics.rst',
  'qualification/iopt-pts.rst',
  'qualification/l2cap-pics.rst',
  'qualification/l2cap-pts.rst',
  'qualification/rfcomm-pics.rst',
  'qualification/rfcomm-pts.rst',
  'qualification/scpp-pics.rst',
  'qualification/scpp-pts.rst',
  'qualification/sdp-pics.rst',
  'qualification/sdp-pts.rst',
  'qualification/sm-pics.rst',
  'qualification/sm-pts.rst',
  'qualification/spp-pics.rst',
  'qualification/spp-pts.rst',
]

bluetoothd_rst = configure_file(
  input: 'bluetoothd.rst.in',
  output: 'bluetoothd.rst',
  configuration: man_conf
)

if get_option('daemon').enabled() and rst2man.found()
  custom_target(
    input: bluetoothd_rst,
    output: 'bluetoothd.8',
    command: rst2man_command,
    install: true,
    install_dir: get_option('mandir') / 'man8'
  )
endif

bluetooth_meshd_rst = configure_file(
  input: 'bluetooth-meshd.rst.in',
  output: 'bluetooth-meshd.rst',
  configuration: man_conf
)
if get_option('mesh').enabled() and rst2man.found()
  custom_target(
    input: bluetooth_meshd_rst,
    output: 'bluetooth-meshd.8',
    command: rst2man_command,
    install: true,
    install_dir: get_option('mandir') / 'man8'
  )
endif

if get_option('documentation').enabled()
  docs_sources = [
    'conf.py',
    'favicon.ico',

    'index.rst',
    'dev-maint.rst',
    'test-runner.rst',
    'assigned-numbers.rst',
    'security-bugs.rst',
    'coding-style.rst',
    'maintainer-guidelines.rst',
    'protocols.rst',
    'daemons.rst',
    'bluetoothctl-cmd.rst',
    'tools.rst',
    'deprecated-tools.rst',
    'dbus.rst',
    'dbus-gatt.rst',
    'dbus-obex.rst',
    'qualifications.rst',
  ]
  docs_sources += qualifications_docs

  foreach man_page: doc_man_pages
    man_page_name = man_page.substring(0, -2)
    man_section = man_page.substring(-1)
    if man_section.to_int() == 7
      man_page_name = man_page_name + '-protocol'
    endif
    docs_sources += man_page_name + '.rst'
  endforeach

  copied_docs_sources = []
  foreach d: docs_sources
    copied_docs_sources += configure_file(input: d, output: fs.name(d), copy: true)
  endforeach
  copied_docs_sources += [ bluetooth_meshd_rst, bluetoothd_rst ]

  docs_inputs = copied_docs_sources

  # Copy static files to the appropriate output subdir
  copy = find_program('copy-subdir.py')

  static_files = [
    'bullet.svg',
    'card.png',
    'figtree.woff2',
    'toc_bullet.svg',
    'bluez.css',
    'bluez.png',
  ]

  foreach f: static_files
    run_command(copy, '_static' / f, '_static' / f, check: true)
  endforeach

  sphinx_command = [ sphinx_build ]

  if get_option('werror')
    sphinx_command += [ '-W', '--keep-going' ]
  endif

  custom_target('docs',
    input: docs_inputs,
    output: 'html',
    command: sphinx_command + [ meson.current_build_dir(), '@OUTPUT@' ],
    build_by_default: true,
  )
endif

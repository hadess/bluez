man_pages = [
  'hci.7',
  'iso.7',
  'mgmt.7',
  'l2cap.7',
  'rfcomm.7',
  'sco.7',
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
]

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

if get_option('obex').enabled()
  man_pages += obex_man_pages
endif

monitor_man_pages = [ 'btmon.1' ]

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

noinst_tools_man_pages = [ 'btmgmt.1' ]

if get_option('tools').allowed()
  man_pages += tools_man_pages
  man_pages += noinst_tools_man_pages
endif

deprecated_tools_man_pages = [
  'hciattach.1',
  'hciconfig.1',
  'hcitool.1',
  'hcidump.1',
  'rfcomm.1',
  'sdptool.1',
  'ciptool.1',
]

if get_option('deprecated').allowed()
  man_pages += deprecated_tools_man_pages
endif

hid2hci_man_pages = [ 'hid2hci.1' ]

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

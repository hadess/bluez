
tests1 = [
  [ 'test-tester.c' ],
  [ 'test-uuid.c' ],
  [ 'test-eir.c', '../src/eir.c', '../src/uuid-helper.c' ],
  [ 'test-sdp.c', '../src/sdpd-database.c', '../src/log.c', '../src/sdpd-service.c', '../src/sdpd-request.c' ],
  [ 'test-avrcp.c', '../src/log.c', 'avctp.c', 'avrcp-lib.c' ],
  [ 'test-lib.c' ],
  [ 'test-gatt.c' ],
  [ 'test-micp.c' ],
  [ 'test-bass.c' ],
  [ 'test-vcp.c' ],
  [ 'test-battery.c' ],
  [ 'test-tmap.c' ],
  [ 'test-gmap.c' ],
  [ 'test-rap.c' ],
  [ 'test-mcp.c' ],
  [ 'test-hog.c',
    btio_sources,
    '../profiles/input/hog-lib.c',
    '../profiles/scanparam/scpp.c',
    '../profiles/battery/bas.c',
    '../profiles/deviceinfo/dis.c',
    '../src/log.c',
    '../attrib/att.c',
    '../attrib/gatt.c',
    '../attrib/gattrib.c'
  ],
]

foreach t: tests1
  test_name = t[0].substring(0, -2)
  exe = executable(test_name,
    sources: t,
    dependencies: [ libshared_glib_dep, libbluetooth_internal_dep ],
  )
  test(test_name, exe)
endforeach

tests2 = [
  [ 'test-bap.c' ],
]

unittest_list = find_program('unittest_list.sh')

foreach t: tests2
  test_name = t[0].substring(0, -2)
  exe = executable(test_name,
    sources: t,
    dependencies: [ libshared_glib_dep, libbluetooth_internal_dep ],
  )

  r = run_command(sh, unittest_list, t, check: true)
  unit_tests = r.stdout().strip().split(' ')
  foreach ut: unit_tests
    test(ut, exe, args: [ '-p', ut ])
  endforeach
endforeach

tests3 = [
  [ 'test-textfile.c', '../src/textfile.c' ],
  [ 'test-crc.c', '../monitor/crc.c' ],
  [ 'test-crypto.c' ],
  [ 'test-ecc.c' ],
  [ 'test-ringbuf.c' ],
  [ 'test-queue.c' ],
  [ 'test-mgmt.c' ],
  [ 'test-uhid.c' ],
  [ 'test-hfp.c' ],
  [ 'test-avdtp.c', '../src/log.c', 'avdtp.c' ],
  [ 'test-avctp.c', '../src/log.c', 'avctp.c' ],
]
tests4 = []

if ical_dep.found()
  tests3 += [
    [ 'test-gobex.c', 'util.c', gobex_sources ],
    [ 'test-gobex-packet.c', 'util.c', gobex_sources ],
    [ 'test-gobex-header.c', 'util.c', gobex_sources ],
    [ 'test-gobex-apparam.c', 'util.c', gobex_sources ],
  ]
  tests4 = [
    [ 'test-gobex-transfer.c', 'util.c', gobex_sources ],
  ]
endif

foreach t: tests3
  test_name = t[0].substring(0, -2)
  exe = executable(test_name,
    sources: t,
    c_args: [ '-DSTORAGEDIR="@0"'.format(storagedir) ],
    include_directories: '../lib',
    dependencies: libshared_glib_dep
  )
  test(test_name, exe)
endforeach

foreach t: tests4
  test_name = t[0].substring(0, -2)
  exe = executable(test_name,
    sources: t,
    c_args: [ '-DSTORAGEDIR="@0"'.format(storagedir) ],
    include_directories: '../lib',
    dependencies: libshared_glib_dep
  )

  r = run_command(sh, unittest_list, t, check: true)
  unit_tests = r.stdout().strip().split(' ')
  foreach ut: unit_tests
    test(ut, exe, args: [ '-p', ut ])
  endforeach
endforeach

exe = executable('test-gdbus-client',
  sources: 'test-gdbus-client.c',
  dependencies: [ libgdbus_internal_dep, libshared_glib_dep ]
)
dbus_run_session = find_program('dbus-run-session')
if dbus_run_session.found()
  test('test-gdbus-client', dbus_run_session, args: [ '--', exe ])
else
  test('test-gdbus-client', exe)
endif

exe = executable('test-gattrib',
  sources: [ 'test-gattrib.c', '../attrib/gattrib.c', btio_sources, '../src/log.c' ],
  dependencies: [ libshared_glib_dep, libbluetooth_internal_dep ]
)
test('test-gattrib', exe)

exe = executable('test-profile',
  sources: [ 'test-profile.c', 'btd.c', '../src/profile.c' ],
  dependencies: [ dbus_dep, libshared_glib_dep, libbluetooth_internal_dep ]
)
test('test-profile', exe)

if alsa_dep.found()
  exe = executable('test-midi',
    sources: [ 'test-midi.c', '../profiles/midi/libmidi.c' ],
    c_args: [ '-DMIDI_TEST' ],
    dependencies: [ libshared_glib_dep, alsa_dep ]
  )
  test('test-midi', exe)
endif

if get_option('mesh').enabled()
  exe = executable('test-mesh-crypto',
    sources: [ 'test-mesh-crypto.c' ],
    dependencies: [ ell_dep ]
  )
  test('test-mesh-crypto', exe)
endif

add_test_setup(
  'valgrind',
  exe_wrapper: [
    'valgrind', '--quiet', '--leak-check=full', '--trace-children=yes',
    '--suppressions=@0@'.format(meson.current_source_dir() / 'valgrind.supp'),
    '--num-callers=30', '--show-reachable=no', '--error-exitcode=1',
  ],
  # default timeout in meson is 30s
  timeout_multiplier: 4,
)

if not has_dbusmock
  test('integration-test-skipped',
    find_program('integration-test-skipped.sh'),
  )
else
  integration_test = find_program('integration-test.py')

  envs = environment()
  envs.set ('top_builddir', meson.global_build_root())
  envs.set ('top_srcdir', meson.global_source_root())

  test_deps = []

  python3 = find_program('python3')
  unittest_inspector = find_program('unittest_inspector.py')
  r = run_command(unittest_inspector, files('integration-test.py'), check: true)
  unit_tests = r.stdout().strip().split('\n')

  foreach ut: unit_tests
    ut_args = files('integration-test.py')
    ut_args += ut
    test(ut,
      python3,
      args: ut_args,
      env: envs,
      depends: test_deps
    )
  endforeach
endif

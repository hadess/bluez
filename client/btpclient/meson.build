executable('btpclient',
  sources: [
    'btpclient.c',
    'core.c',
    'gap.c',
    '../../src/shared/btp.c' ],
  include_directories: '..',
  dependencies: [ libbluetooth_internal_dep, libshared_ell_dep ],
)
executable('btpclientctl',
  sources: [ 'btpclientctl.c', '../../client/display.c' ],
  dependencies: [
    libshared_mainloop_dep,
    libshared_glib_dep,
    libbluetooth_internal_dep,
    readline_dep
  ]
)

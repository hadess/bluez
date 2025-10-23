builtin_modules = [
  'hostname',
  'autopair',
  'policy',
]

builtin_sources = []

if get_option('rust').enabled()
  # FIXME
  # keep .rs file first
  # See https://github.com/mesonbuild/meson/pull/15125
  builtin_modules += 'rust_test'
  builtin_sources += files(
    'rust-test.rs',
    'plugin.rs'
  )
endif

builtin_sources += files(
  'hostname.c',
  'autopair.c',
  'policy.c'
)

builtin_deps = []

if get_option('admin').enabled()
  builtin_modules += 'admin'
  builtin_sources += files('admin.c')
endif

if get_option('nfc').enabled()
  builtin_modules += 'neard'
  builtin_sources += files('neard.c')
endif

if get_option('sixaxis').enabled()
  builtin_modules += 'sixaxis'
  builtin_sources += files('sixaxis.c')
  builtin_deps += udev_dep
endif

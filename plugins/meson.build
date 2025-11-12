builtin_modules += [
  'autopair',
  'policy',
]

builtin_sources += files(
  'autopair.c',
  'policy.c'
)

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

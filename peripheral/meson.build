if get_option('tests').allowed()
  executable('btsensor',
    sources: [
      'main.c',
      'efivars.c',
      'attach.c',
      'log.c',
      'gap.c',
      'gatt.c',
    ],
    dependencies: [ libshared_mainloop_dep, libbluetooth_internal_dep ]
  )
endif

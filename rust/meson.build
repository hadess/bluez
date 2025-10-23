version_rs = configure_file(
  input: 'version.rs.in',
  output: 'version.rs',
  configuration: config_h
)

# FIXME
# keep .rs file first
# See https://github.com/mesonbuild/meson/pull/15125
builtin_modules += 'rust_test'
builtin_sources += structured_sources([
  'rust-test.rs',
  'plugin.rs',
  'adapter.rs',
  version_rs
])

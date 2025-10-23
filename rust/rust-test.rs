#![no_main]

pub mod adapter;
pub mod plugin;
pub mod version;

use std::ffi::CStr;

static PLUGIN_NAME: &CStr = c"rust_test";

#[allow(non_upper_case_globals)]
#[unsafe(export_name = "__bluetooth_builtin_rust_test")]
pub static __bluetooth_builtin_rust_test: plugin::bluetooth_plugin_desc = plugin::bluetooth_plugin_desc {
    name: PLUGIN_NAME.as_ptr(),
    version: version::VERSION.as_ptr(),
    priority: plugin::PRIORITY_DEFAULT,
    init: Some(rust_test_init),
    exit: Some(rust_test_exit),
    debug_start: std::ptr::null_mut(),
    debug_stop: std::ptr::null_mut()
};

unsafe impl Sync for plugin::bluetooth_plugin_desc {}
unsafe impl Send for plugin::bluetooth_plugin_desc {}

#[allow(non_upper_case_globals)]
static rust_test_driver: adapter::btd_adapter_driver = adapter::btd_adapter_driver {
    name: PLUGIN_NAME.as_ptr(),
    probe: Some(rust_test_probe),
    remove: Some(rust_test_remove),
    resume: None,
    device_added: None,
    device_removed: None,
    device_resolved: None,
    experimental: false,
};

unsafe impl Sync for adapter::btd_adapter_driver {}
unsafe impl Send for adapter::btd_adapter_driver {}

unsafe extern "C" fn rust_test_probe(_adapter: *mut adapter::btd_adapter) -> i32 {
    println!("rust_test_probe");
    0
}

unsafe extern "C" fn rust_test_remove(_adapter: *mut adapter::btd_adapter) {
    println!("rust_test_remove");
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn rust_test_init() -> i32 {
    println!("rust_test_init called");
    unsafe { adapter::btd_register_adapter_driver(&raw const rust_test_driver); }
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn rust_test_exit() {
    println!("rust_test_exit called");
    unsafe { adapter::btd_unregister_adapter_driver(&raw const rust_test_driver); }
}

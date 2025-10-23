#![no_main]

pub mod plugin;
pub mod version;

use std::ffi::CStr;

//static VERSION: &CStr = c"5.84";
static PLUGIN_NAME: &CStr = c"rust_test";

#[allow(non_upper_case_globals)]
#[unsafe(export_name = "__bluetooth_builtin_rust_test")]
pub static __bluetooth_builtin_rust_test: plugin::bluetooth_plugin_desc = plugin::bluetooth_plugin_desc {
    name: PLUGIN_NAME.as_ptr(),
    version: version::VERSION.as_ptr(),
    priority: plugin::BLUETOOTH_PLUGIN_PRIORITY_DEFAULT,
    init: Some(rust_test_init),
    exit: Some(rust_test_exit),
    debug_start: std::ptr::null_mut(),
    debug_stop: std::ptr::null_mut()
};

unsafe impl Sync for plugin::bluetooth_plugin_desc {}
unsafe impl Send for plugin::bluetooth_plugin_desc {}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn rust_test_init() -> i32 {
    println!("rust_test_init called");
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn rust_test_exit() {
    println!("rust_test_exit called");
}

#![no_main]

// Generated with bindgen on src/plugin.h

use std::ffi::CStr;

pub const BLUETOOTH_PLUGIN_PRIORITY_DEFAULT: i32 = 0;

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct bluetooth_plugin_desc {
    pub name: *const ::std::os::raw::c_char,
    pub version: *const ::std::os::raw::c_char,
    pub priority: ::std::os::raw::c_int,
    pub init: ::std::option::Option<unsafe extern "C" fn() -> ::std::os::raw::c_int>,
    pub exit: ::std::option::Option<unsafe extern "C" fn()>,
    pub debug_start: *mut ::std::os::raw::c_void,
    pub debug_stop: *mut ::std::os::raw::c_void,
}

static PLUGIN_NAME: &CStr = c"rust_test";
static VERSION: &CStr = c"5.84";

//#[unsafe(no_mangle)]
#[unsafe(export_name = "__bluetooth_builtin_rust_test")]
pub static __bluetooth_builtin_rust_test: bluetooth_plugin_desc = bluetooth_plugin_desc {
    name: PLUGIN_NAME.as_ptr(),
    version: VERSION.as_ptr(),
    priority: BLUETOOTH_PLUGIN_PRIORITY_DEFAULT,
    init: Some(rust_test_init),
    exit: Some(rust_test_exit),
    debug_start: std::ptr::null_mut(),
    debug_stop: std::ptr::null_mut()
};

#[unsafe(no_mangle)]
pub unsafe extern "C" fn rust_test_init() -> i32 {
    println!("rust_test_init called");
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn rust_test_exit() {
    println!("rust_test_exit called");
}

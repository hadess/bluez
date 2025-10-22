#![no_main]

// Generated with bindgen on src/plugin.h

use std::ffi::CStr;

pub const BLUETOOTH_PLUGIN_PRIORITY_DEFAULT: u32 = 0;

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

//#[unsafe]
const rust_test: bluetooth_plugin_desc = bluetooth_plugin_desc {
    name: &CStr = c"rust_test",
//    version: "5.84",
//    priority: BLUETOOTH_PLUGIN_PRIORITY_DEFAULT,
//    init: rust_test_init,
//    exit: rust_test_exit
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

// Generated with bindgen on src/plugin.h

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



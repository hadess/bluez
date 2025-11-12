#![no_main]

pub mod adapter;
pub mod plugin;
pub mod version;

use std::ffi::CStr;

static PLUGIN_NAME: &CStr = c"hostname";

#[allow(non_upper_case_globals)]
#[unsafe(export_name = "__bluetooth_builtin_hostname")]
pub static __bluetooth_builtin_hostname: plugin::bluetooth_plugin_desc = plugin::bluetooth_plugin_desc {
    name: PLUGIN_NAME.as_ptr(),
    version: version::VERSION.as_ptr(),
    priority: plugin::PRIORITY_DEFAULT,
    init: Some(hostname_init),
    exit: Some(hostname_exit),
    debug_start: std::ptr::null_mut(),
    debug_stop: std::ptr::null_mut()
};

unsafe impl Sync for plugin::bluetooth_plugin_desc {}
unsafe impl Send for plugin::bluetooth_plugin_desc {}

#[allow(non_upper_case_globals)]
static hostname_driver: adapter::btd_adapter_driver = adapter::btd_adapter_driver {
    name: PLUGIN_NAME.as_ptr(),
    probe: Some(hostname_probe),
    remove: Some(hostname_remove),
    resume: None,
    device_added: None,
    device_removed: None,
    device_resolved: None,
    experimental: false,
};

unsafe impl Sync for adapter::btd_adapter_driver {}
unsafe impl Send for adapter::btd_adapter_driver {}

unsafe extern "C" fn hostname_probe(_adapter: *mut adapter::btd_adapter) -> i32 {
    println!("hostname_probe");
    0
}

unsafe extern "C" fn hostname_remove(_adapter: *mut adapter::btd_adapter) {
    println!("hostname_remove");
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn hostname_init() -> i32 {
    println!("hostname_init called");
    //read DMI
    //read transient hostname
    //setup dbus client to access hostnamed
    unsafe { adapter::btd_register_adapter_driver(&raw const hostname_driver); }
    //monitor /proc/sys/kernel/hostname
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn hostname_exit() {
    println!("hostname_exit called");
    unsafe { adapter::btd_unregister_adapter_driver(&raw const hostname_driver); }

    //FIXME
    //free dbus proxy
    //free dbus client
    //free hostname change fd
    //free strings
}

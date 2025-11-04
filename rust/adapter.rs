// Excerpts from:
// bindgen src/adapter.h -- -I/usr/lib/gcc/*/*/include/ `pkg-config --cflags dbus-1 glib-2.0` -Ilib/ > rust/adapter.rs

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct btd_adapter {
    _unused: [u8; 0],
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct btd_device {
    _unused: [u8; 0],
}

unsafe extern "C" {
    pub fn btd_register_adapter_driver(driver: *const btd_adapter_driver) -> ::std::os::raw::c_int;
}
unsafe extern "C" {
    pub fn btd_unregister_adapter_driver(driver: *const btd_adapter_driver);
}
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct btd_adapter_driver {
    pub name: *const ::std::os::raw::c_char,
    pub probe: ::std::option::Option<
        unsafe extern "C" fn(adapter: *mut btd_adapter) -> ::std::os::raw::c_int,
    >,
    pub remove: ::std::option::Option<unsafe extern "C" fn(adapter: *mut btd_adapter)>,
    pub resume: ::std::option::Option<unsafe extern "C" fn(adapter: *mut btd_adapter)>,
    pub device_added: ::std::option::Option<
        unsafe extern "C" fn(adapter: *mut btd_adapter, device: *mut btd_device),
    >,
    pub device_removed: ::std::option::Option<
        unsafe extern "C" fn(adapter: *mut btd_adapter, device: *mut btd_device),
    >,
    pub device_resolved: ::std::option::Option<
        unsafe extern "C" fn(adapter: *mut btd_adapter, device: *mut btd_device),
    >,
    pub experimental: bool,
}
#[allow(clippy::unnecessary_operation, clippy::identity_op)]
const _: () = {
    ["Size of btd_adapter_driver"][::std::mem::size_of::<btd_adapter_driver>() - 64usize];
    ["Alignment of btd_adapter_driver"][::std::mem::align_of::<btd_adapter_driver>() - 8usize];
    ["Offset of field: btd_adapter_driver::name"]
        [::std::mem::offset_of!(btd_adapter_driver, name) - 0usize];
    ["Offset of field: btd_adapter_driver::probe"]
        [::std::mem::offset_of!(btd_adapter_driver, probe) - 8usize];
    ["Offset of field: btd_adapter_driver::remove"]
        [::std::mem::offset_of!(btd_adapter_driver, remove) - 16usize];
    ["Offset of field: btd_adapter_driver::resume"]
        [::std::mem::offset_of!(btd_adapter_driver, resume) - 24usize];
    ["Offset of field: btd_adapter_driver::device_added"]
        [::std::mem::offset_of!(btd_adapter_driver, device_added) - 32usize];
    ["Offset of field: btd_adapter_driver::device_removed"]
        [::std::mem::offset_of!(btd_adapter_driver, device_removed) - 40usize];
    ["Offset of field: btd_adapter_driver::device_resolved"]
        [::std::mem::offset_of!(btd_adapter_driver, device_resolved) - 48usize];
    ["Offset of field: btd_adapter_driver::experimental"]
        [::std::mem::offset_of!(btd_adapter_driver, experimental) - 56usize];
};

unsafe extern "C" {
    pub fn btd_adapter_is_default(adapter: *mut btd_adapter) -> bool;
}
unsafe extern "C" {
    pub fn btd_adapter_set_name(
        adapter: *mut btd_adapter,
        name: *const ::std::os::raw::c_char,
    ) -> ::std::os::raw::c_int;
}
pub const MGMT_INDEX_NONE: u16 = 0xFFFF;
unsafe extern "C" {
    pub fn btd_adapter_get_index(adapter: *mut btd_adapter) -> u16;
}
unsafe extern "C" {
    pub fn btd_adapter_set_class(adapter: *mut btd_adapter, major: u8, minor: u8);
}
#[allow(non_camel_case_types)]
pub type adapter_cb =
    ::std::option::Option<unsafe extern "C" fn(adapter: *mut btd_adapter, user_data: *mut ::std::os::raw::c_void)>;
unsafe extern "C" {
    pub fn btd_adapter_foreach(func: adapter_cb, user_data: *mut ::std::os::raw::c_void);
}

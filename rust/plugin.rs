// Generated with:
// bindgen ../src/plugin.h -- -DBLUETOOTH_PLUGIN_BUILTIN
// and hand-modified.

pub const PRIORITY_LOW: BluetoothPluginPriority = -100;
pub const PRIORITY_DEFAULT: BluetoothPluginPriority = 0;
pub const PRIORITY_HIGH: BluetoothPluginPriority = 100;
pub type BluetoothPluginPriority = ::std::os::raw::c_int;
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
#[allow(clippy::unnecessary_operation, clippy::identity_op)]
const _: () = {
    ["Size of bluetooth_plugin_desc"][::std::mem::size_of::<bluetooth_plugin_desc>() - 56usize];
    ["Alignment of bluetooth_plugin_desc"]
        [::std::mem::align_of::<bluetooth_plugin_desc>() - 8usize];
    ["Offset of field: bluetooth_plugin_desc::name"]
        [::std::mem::offset_of!(bluetooth_plugin_desc, name) - 0usize];
    ["Offset of field: bluetooth_plugin_desc::version"]
        [::std::mem::offset_of!(bluetooth_plugin_desc, version) - 8usize];
    ["Offset of field: bluetooth_plugin_desc::priority"]
        [::std::mem::offset_of!(bluetooth_plugin_desc, priority) - 16usize];
    ["Offset of field: bluetooth_plugin_desc::init"]
        [::std::mem::offset_of!(bluetooth_plugin_desc, init) - 24usize];
    ["Offset of field: bluetooth_plugin_desc::exit"]
        [::std::mem::offset_of!(bluetooth_plugin_desc, exit) - 32usize];
    ["Offset of field: bluetooth_plugin_desc::debug_start"]
        [::std::mem::offset_of!(bluetooth_plugin_desc, debug_start) - 40usize];
    ["Offset of field: bluetooth_plugin_desc::debug_stop"]
        [::std::mem::offset_of!(bluetooth_plugin_desc, debug_stop) - 48usize];
};

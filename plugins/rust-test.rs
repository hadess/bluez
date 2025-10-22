#![no_main]

#[no_mangle]
pub unsafe extern "C" fn rust_test_init() -> i32 {
    println!("rust_test_init called");
    0
}

#[no_mangle]
pub unsafe extern "C" fn rust_test_exit() {
    println!("rust_test_exit called");
}

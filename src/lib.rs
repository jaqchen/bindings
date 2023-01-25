#![allow(non_snake_case)]
#![allow(non_camel_case_types)]
#![allow(non_upper_case_globals)]

use std::os::raw::c_int;
use std::os::raw::c_longlong;
use std::os::raw::c_ulonglong;

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

pub fn strtol(x: &str, base: i32) -> Result<i64, std::io::Error> {
    let mut res: c_longlong = 0;
    let y: Vec<u8> = x.as_bytes().iter().cloned().collect();
    let error = unsafe {
        let z = std::ffi::CString::from_vec_unchecked(y);
        extm_strtol(z.as_ptr(), &mut res as *mut c_longlong, base as c_int)
    };
    match error {
        0 => Ok(res as i64),
        _ => Err(std::io::Error::from_raw_os_error(error as i32)),
    }
}

pub fn strtoul(x: &str, base: i32) -> Result<u64, std::io::Error> {
    let mut res: c_ulonglong = 0;
    let y: Vec<u8> = x.as_bytes().iter().cloned().collect();
    let error = unsafe {
        let z = std::ffi::CString::from_vec_unchecked(y);
        extm_strtoul(z.as_ptr(), &mut res as *mut c_ulonglong, base as c_int)
    };
    match error {
        0 => Ok(res as u64),
        _ => Err(std::io::Error::from_raw_os_error(error as i32)),
    }
}

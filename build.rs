use std::ffi::CString;
use libc::{c_char, execv};
use std::collections::VecDeque;

fn main() {
    // invoke build.rs.sh script instead
    let argv: Vec<String> = std::env::args().skip(1).collect();
    let mut argw: VecDeque<CString> = argv.iter()
        .map(|x| CString::new(x.as_bytes()).unwrap()).collect();
    argw.push_front(CString::new("./build.rs.sh").unwrap());
    let mut argx: Vec<*const c_char> = argw.iter().map(|y| y.as_ptr()).collect();
    argx.push(std::ptr::null());
    unsafe { execv(argx[0], argx.as_mut_ptr()) };
    eprintln!("Error, failed to invoke ./build.rs.sh: {:?}",
        std::io::Error::last_os_error());
    std::process::exit(1);
}

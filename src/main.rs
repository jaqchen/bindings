use std::process::Command;
use bindings::extm_uptime;
use bindings::extm_cloexec;
use bindings::extm_zipstdio;
use bindings::extm_close_range;
use bindings::{strtol, strtoul};
use std::os::unix::io::AsRawFd;
use std::os::unix::process::CommandExt;

fn close_range() -> Result<(), std::io::Error> {
    unsafe { extm_close_range(3, 1024, 0) };
    Ok(())
}

fn zip_stdio() -> Result<(), std::io::Error> {
    unsafe { extm_zipstdio(std::ptr::null()) };
    Ok(())
}

fn invoke_ls(doclose: bool, dozip: bool) {
    let mut cmds = Command::new("ls");
    let mut cmd: &mut Command = cmds.args(["-l", "/proc/self/fd"]);
    if doclose {
        cmd = unsafe { cmd.pre_exec(close_range) };
    }
    if dozip {
        cmd = unsafe { cmd.pre_exec(zip_stdio) };
    }
    let _ = cmd.spawn().unwrap().wait().unwrap();
}

fn main() {
    let arg0: i64 = std::env::args().nth(1)
        .and_then(|arg1| strtol(&arg1, 0).ok())
        .unwrap_or(2021);
    let arg1: u64 = std::env::args().nth(2)
        .and_then(|arg2| strtoul(&arg2, 0).ok())
        .unwrap_or(0x2022);
    println!("arg0: {}, arg1: {:#x}", arg0, arg1);

    let uptim = unsafe { extm_uptime(std::ptr::null_mut()) };
    println!("System uptime: {}", uptim);
    let tmph = std::fs::File::open("/dev/null").unwrap();
    let _ = unsafe { extm_cloexec(tmph.as_raw_fd() as i32, 0) };
    invoke_ls(false, false);
    invoke_ls(true, false);
    invoke_ls(false, true);
    drop(tmph);
}

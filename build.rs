extern crate bindgen;
use std::process::Command;

fn main() {
    // generate binding.rs for extmodule
    let bindings = bindgen::Builder::default()
        .header("extmodule/extmodule.h")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        .generate()
        .expect("Unable to generate bindings");
    let out_path = std::path::PathBuf::from(std::env::var("OUT_DIR").unwrap());
    bindings.write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");

    // invoke make to build external C module
    let cc = format!("CC={}", std::env::var("TARGET_CC")
        .unwrap_or_else(|_| "cc".to_string()));
    let cflags = format!("CFLAGS={}", std::env::var("TARGET_CFLAGS")
        .unwrap_or_else(|_| "-Wall -fPIC -D_GNU_SOURCE -Os -ggdb".to_string()));
    let okay = Command::new("make")
        .arg(AsRef::<std::ffi::OsStr>::as_ref(&cc))
        .arg(AsRef::<std::ffi::OsStr>::as_ref(&cflags))
        .args(&["-C", "./extmodule", "-j1", "clean", "all"])
        .spawn()
        .expect("Failed to invoke make utility")
        .wait()
        .expect("Failed to wait make utility")
        .success();
    if !okay {
        eprintln!("Error, make for external C module has failed!");
        std::process::exit(1);
    }

    println!("cargo:rustc-link-lib=extm");
    println!("cargo:rustc-link-search=./extmodule");
    println!("cargo:rerun-if-changed=./extmodule/extmodule.h");
    println!("cargo:rustc-link-arg-bins=-Wl,-rpath=$ORIGIN");
}

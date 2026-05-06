//! Build script for metkit-sys
//!
//! Supports two build modes:
//! - `vendored` (default): Clone and build metkit from source using ecbuild
//! - `system`: Use `CMake` `find_package` to find system-installed metkit

const METKIT_VERSION: &str = "1.18.1";

fn main() {
    println!("cargo:rerun-if-changed=build.rs");
    println!("cargo:rerun-if-env-changed=METKIT_DIR");
    println!("cargo:rerun-if-env-changed=CMAKE_PREFIX_PATH");
    println!("cargo:rerun-if-env-changed=DOCS_RS");

    if bindman_utils::is_docs_rs() {
        return;
    }

    bindman_utils::validate_build_mode(cfg!(feature = "system"), cfg!(feature = "vendored"));

    let include = if cfg!(feature = "system") {
        build_system()
    } else {
        build_vendored()
    };

    build_cxx_bridge(&include);

    let crate_dir =
        std::path::PathBuf::from(std::env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR"));
    bindman_build::check_cpp_api(&include, &crate_dir.join("src/lib.rs"));

    // Export cpp directory for downstream crates (metkit_bridge.h)
    println!("cargo:cpp_dir={}", crate_dir.join("cpp").display());
}

/// Compile the CXX bridge.
fn build_cxx_bridge(include: &std::path::Path) {
    let crate_dir = std::path::PathBuf::from(
        std::env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR not set"),
    );
    let eckit_include = std::env::var("DEP_ECKIT_SYS_INCLUDE")
        .expect("DEP_ECKIT_SYS_INCLUDE not set — eckit-sys must be a dependency");
    let eckit_out_dir = std::env::var("DEP_ECKIT_SYS_EXCEPTIONS_HEADER")
        .ok()
        .and_then(|p| {
            std::path::PathBuf::from(p)
                .parent()
                .map(std::path::Path::to_path_buf)
        });

    println!("cargo:rerun-if-changed=cpp/metkit_bridge.h");
    println!("cargo:rerun-if-changed=cpp/metkit_bridge.cpp");

    let mut build = cxx_build::bridge("src/lib.rs");
    build
        .file(crate_dir.join("cpp/metkit_bridge.cpp"))
        .include(include)
        .include(crate_dir.join("cpp"))
        .include(&eckit_include);

    // Include eckit's OUT_DIR for eckit_exceptions.h
    if let Some(ref eckit_out) = eckit_out_dir {
        build.include(eckit_out);
    }

    // Include eckit's cpp dir for eckit_bridge.h (needed for StreamWrapper)
    if let Ok(eckit_cpp_dir) = std::env::var("DEP_ECKIT_SYS_CPP_DIR") {
        build.include(&eckit_cpp_dir);
    }

    build
        .flag_if_supported("-std=c++17")
        .compile("metkit_sys_bridge");

    bindman_utils::link_cpp_stdlib();
}

#[cfg(feature = "system")]
fn build_system() -> std::path::PathBuf {
    let (root, include, lib_dir) = bindman_utils::cmake_find_package("metkit", METKIT_VERSION);

    println!("cargo:rustc-link-search=native={}", lib_dir.display());
    println!("cargo:rustc-link-lib=dylib=metkit");
    bindman_utils::link_cpp_stdlib();

    // Export for downstream crates
    println!("cargo:root={}", root.display());
    println!("cargo:include={}", include.display());

    include
}

#[cfg(not(feature = "system"))]
fn build_system() -> std::path::PathBuf {
    unreachable!("build_system called without system feature");
}

/// Build metkit from source using ecbuild
#[cfg(feature = "vendored")]
#[allow(clippy::too_many_lines)]
fn build_vendored() -> std::path::PathBuf {
    use std::env;
    use std::fs;
    use std::path::PathBuf;
    use std::process::Command;

    const ECBUILD_REPO: &str = "https://github.com/ecmwf/ecbuild.git";
    const ECBUILD_TAG: &str = "3.13.1";

    const METKIT_REPO: &str = "https://github.com/ecmwf/metkit.git";

    let out_dir = PathBuf::from(env::var("OUT_DIR").expect("OUT_DIR not set"));
    let src_dir = out_dir.join("src");
    let build_dir = out_dir.join("build");
    let install_dir = out_dir.join("install");

    fs::create_dir_all(&src_dir).expect("Failed to create src directory");
    fs::create_dir_all(&build_dir).expect("Failed to create build directory");

    // Get dependency paths
    let eckit_root = env::var("DEP_ECKIT_SYS_ROOT")
        .expect("DEP_ECKIT_SYS_ROOT not set - eckit-sys must be a dependency");
    let eccodes_root = env::var("DEP_ECCODES_SYS_ROOT")
        .expect("DEP_ECCODES_SYS_ROOT not set - eccodes-sys must be a dependency");

    // Clone sources
    let ecbuild_src = bindman_utils::git_clone(ECBUILD_REPO, ECBUILD_TAG, &src_dir.join("ecbuild"));
    let metkit_src = bindman_utils::git_clone(METKIT_REPO, METKIT_VERSION, &src_dir.join("metkit"));

    let ecbuild_bin = ecbuild_src.join("bin/ecbuild");
    let num_jobs = bindman_utils::build_parallelism();

    let cmake_prefix_path = format!("{eckit_root};{eccodes_root}");

    // Build metkit
    let mut cmd = Command::new(&ecbuild_bin);
    cmd.current_dir(&build_dir)
        .arg(format!("--prefix={}", install_dir.display()))
        .arg("--")
        .arg(&metkit_src)
        .arg(format!("-DCMAKE_PREFIX_PATH={cmake_prefix_path}"))
        .arg(format!(
            "-DCMAKE_BUILD_TYPE={}",
            bindman_utils::cmake_build_type()
        ))
        // Always disabled (no features)
        .arg("-DENABLE_TESTS=OFF")
        .arg("-DENABLE_DOCS=OFF")
        .arg("-DENABLE_BUILD_TOOLS=OFF")
        .arg("-DENABLE_MARS2GRIB_PYTHON=OFF");

    // Feature-gated options
    cmd.arg(format!(
        "-DENABLE_GRIB={}",
        bindman_utils::on_off(cfg!(feature = "grib"))
    ));
    cmd.arg(format!(
        "-DENABLE_BUFR={}",
        bindman_utils::on_off(cfg!(feature = "bufr"))
    ));
    cmd.arg(format!(
        "-DENABLE_NETCDF={}",
        bindman_utils::on_off(cfg!(feature = "netcdf"))
    ));
    cmd.arg(format!(
        "-DENABLE_ODB={}",
        bindman_utils::on_off(cfg!(feature = "odb"))
    ));
    cmd.arg(format!(
        "-DENABLE_MARS2GRIB={}",
        bindman_utils::on_off(cfg!(feature = "mars2grib"))
    ));
    cmd.arg(format!(
        "-DENABLE_METKIT_CONFIG={}",
        bindman_utils::on_off(cfg!(feature = "metkit-config"))
    ));
    cmd.arg(format!(
        "-DENABLE_EXPERIMENTAL={}",
        bindman_utils::on_off(cfg!(feature = "experimental"))
    ));
    cmd.arg(format!(
        "-DENABLE_FAIL_ON_CCSDS={}",
        bindman_utils::on_off(cfg!(feature = "fail-on-ccsds"))
    ));

    // Use @rpath install names — the leaf binary sets rpaths via bindman_utils::emit_rpaths()
    #[cfg(target_os = "macos")]
    cmd.arg("-DCMAKE_INSTALL_NAME_DIR=@rpath");

    bindman_utils::run_command(&mut cmd, "ecbuild configure metkit");

    bindman_utils::run_command(
        Command::new("cmake")
            .args(["--build", ".", "--parallel", &num_jobs])
            .current_dir(&build_dir),
        "cmake build metkit",
    );

    bindman_utils::run_command(
        Command::new("cmake")
            .args(["--install", "."])
            .current_dir(&build_dir),
        "cmake install metkit",
    );

    // Copy share directory (contains language.yaml needed at runtime)
    let share_src = build_dir.join("share");
    let share_dst = install_dir.join("share");
    if share_src.exists() {
        bindman_utils::copy_dir_all(&share_src, &share_dst)
            .expect("Failed to copy share directory");
    }

    // Link directives
    let lib_dir = bindman_utils::resolve_lib_dir(&install_dir);

    println!("cargo:rustc-link-search=native={}", lib_dir.display());
    println!("cargo:rustc-link-lib=dylib=metkit");
    bindman_utils::link_cpp_stdlib();

    // Export for downstream crates
    let include = install_dir.join("include");
    println!("cargo:root={}", install_dir.display());
    println!("cargo:include={}", include.display());

    include
}

#[cfg(not(feature = "vendored"))]
fn build_vendored() -> std::path::PathBuf {
    unreachable!("build_vendored called without vendored feature");
}

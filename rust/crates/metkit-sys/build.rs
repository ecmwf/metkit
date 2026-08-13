//! Build script for metkit-sys
//!
//! Supports two build modes:
//! - `vendored` (default): Clone and build metkit from source using ecbuild
//! - `system`: Use `CMake` `find_package` to find system-installed metkit

fn main() {
    println!("cargo:rerun-if-changed=build.rs");
    println!("cargo:rerun-if-env-changed=METKIT_DIR");
    println!("cargo:rerun-if-env-changed=CMAKE_PREFIX_PATH");
    println!("cargo:rerun-if-env-changed=DOCS_RS");

    if bindman_utils::is_docs_rs() {
        generate_exceptions(&docs_source_include());
        return;
    }

    bindman_utils::validate_build_mode(cfg!(feature = "system"), cfg!(feature = "vendored"));

    let include = if cfg!(feature = "system") {
        build_system()
    } else {
        build_vendored()
    };

    generate_exceptions(&include);
    build_cxx_bridge(&include);

    let crate_dir =
        std::path::PathBuf::from(std::env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR"));
    bindman_build::check_cpp_api(&include, &crate_dir.join("src/lib.rs"));

    // Export cpp directory for downstream crates (MetkitBridge.h)
    println!("cargo:cpp_dir={}", crate_dir.join("cpp").display());
}

/// Generate `metkit_exceptions.{h,rs}`. The bridge exposes `CodesHandleWrapper`
/// methods that can throw `metkit::codes::CodesException` / `CodesWrongLength`;
/// nothing in the bridge currently calls `mars2grib`, so its exception headers
/// aren't a source here. eckit's exceptions are inherited from eckit-sys.
fn generate_exceptions(include: &std::path::Path) {
    let out_dir = std::path::PathBuf::from(std::env::var("OUT_DIR").expect("OUT_DIR not set"));

    let own = vec![bindman_build::ExceptionSource {
        header: include.join("metkit/codes/api/CodesTypes.h"),
        include_path: "metkit/codes/api/CodesTypes.h".to_string(),
        cpp_namespace: "metkit::codes".to_string(),
        message_prefix: "metkit".to_string(),
        base_class: "eckit::Exception".to_string(),
        recursive: true,
    }];

    let inherited = bindman_build::collect_dep_exception_sources();

    bindman_build::generate_exception_bridge(&bindman_build::ExceptionBridgeConfig {
        primary_namespace: "metkit",
        out_dir: &out_dir,
        own: &own,
        inherited: &inherited,
    });

    bindman_build::publish_exception_sources(&own, &out_dir);
}

/// Header root for docs builds (`DOCS_RS=1`), where the native metkit build
/// — normally the provider of the include tree — is skipped. `docs-headers/`
/// mirrors the include-tree layout, holding a symlink into this repo's
/// `src/`; `cargo package` embeds the linked file's content, so the same
/// path serves in-repo checkouts and published crates alike. Docs builds
/// consume only the generated Rust side; the C++ catch-block header (where
/// the eckit-inherited sources would matter) is never compiled.
fn docs_source_include() -> std::path::PathBuf {
    std::path::PathBuf::from(std::env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR"))
        .join("docs-headers")
}

/// Compile the CXX bridge.
fn build_cxx_bridge(include: &std::path::Path) {
    let crate_dir = std::path::PathBuf::from(
        std::env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR not set"),
    );
    let out_dir = std::path::PathBuf::from(std::env::var("OUT_DIR").expect("OUT_DIR not set"));
    let eckit_include = std::env::var("DEP_ECKIT_SYS_INCLUDE")
        .expect("DEP_ECKIT_SYS_INCLUDE not set — eckit-sys must be a dependency");
    // metkit >= 1.19 public headers (CodesAPI.h) include eccodes.h directly.
    let eccodes_include = std::env::var("DEP_ECCODES_SYS_INCLUDE")
        .expect("DEP_ECCODES_SYS_INCLUDE not set — eccodes-sys must be a dependency");

    println!("cargo:rerun-if-changed=cpp/MetkitBridge.h");
    println!("cargo:rerun-if-changed=cpp/MarsRequest.h");
    println!("cargo:rerun-if-changed=cpp/MarsRequest.cc");
    println!("cargo:rerun-if-changed=cpp/CodesHandle.h");
    println!("cargo:rerun-if-changed=cpp/CodesHandle.cc");
    println!("cargo:rerun-if-changed=cpp/HyperCube.h");
    println!("cargo:rerun-if-changed=cpp/HyperCube.cc");
    println!("cargo:rerun-if-changed=cpp/ParsedRequests.h");
    println!("cargo:rerun-if-changed=cpp/ParsedRequests.cc");
    println!("cargo:rerun-if-changed=cpp/MarsLanguage.h");
    println!("cargo:rerun-if-changed=cpp/MarsLanguage.cc");
    println!("cargo:rerun-if-changed=cpp/RequestEnvironment.h");
    println!("cargo:rerun-if-changed=cpp/RequestEnvironment.cc");

    let mut build = cxx_build::bridge("src/lib.rs");
    build
        .file(crate_dir.join("cpp/MarsRequest.cc"))
        .file(crate_dir.join("cpp/CodesHandle.cc"))
        .file(crate_dir.join("cpp/HyperCube.cc"))
        .file(crate_dir.join("cpp/ParsedRequests.cc"))
        .file(crate_dir.join("cpp/MarsLanguage.cc"))
        .file(crate_dir.join("cpp/RequestEnvironment.cc"))
        .include(include)
        .include(crate_dir.join("cpp"))
        .include(&eckit_include)
        .include(&eccodes_include)
        .include(&out_dir); // for metkit_exceptions.h (generated)

    // Include eckit's cpp dir for EckitBridge.h (needed for StreamWrapper)
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
    // Minimum supported system version, independent of the crate version
    // (which tracks the vendored metkit release).
    let (root, include, lib_dir) = bindman_utils::cmake_find_package("metkit", "1.18.1");

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

/// Locate the metkit C++ sources. When the crate lives inside the metkit
/// repository (path dependency, or a git dependency — cargo checks out the
/// whole repo), the sources are three levels up from the crate and we build
/// them directly: branch changes take effect and no tag/network is required.
/// Cloning the release tag is the fallback for the packaged (crates.io) case,
/// where the crate ships without the C++ tree.
#[cfg(feature = "vendored")]
fn resolve_metkit_src(src_dir: &std::path::Path) -> std::path::PathBuf {
    const METKIT_REPO: &str = "https://github.com/ecmwf/metkit.git";
    const METKIT_TAG: &str = env!("CARGO_PKG_VERSION");

    let manifest_dir = std::path::PathBuf::from(
        std::env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR not set"),
    );
    if let Some(root) = manifest_dir.ancestors().nth(3)
        && root.join("CMakeLists.txt").exists()
        && root.join("VERSION").exists()
        && root.join("src/metkit").is_dir()
    {
        eprintln!("metkit-sys: building in-tree sources at {}", root.display());

        // Retrigger on C++ source edits.
        println!("cargo:rerun-if-changed={}", root.join("src").display());
        println!(
            "cargo:rerun-if-changed={}",
            root.join("CMakeLists.txt").display()
        );
        println!("cargo:rerun-if-changed={}", root.join("VERSION").display());

        // Diverging is legitimate mid-development (unreleased C++ changes are
        // the point of in-tree builds), but should never go unnoticed. The
        // crate-version test enforces equality at release time.
        let tree_version = std::fs::read_to_string(root.join("VERSION"))
            .map(|s| s.trim().to_string())
            .unwrap_or_default();
        if tree_version != METKIT_TAG {
            println!(
                "cargo:warning=metkit-sys {METKIT_TAG} is building in-tree metkit {tree_version} (versions differ)"
            );
        }

        return root.to_path_buf();
    }
    bindman_utils::git_clone(METKIT_REPO, METKIT_TAG, &src_dir.join("metkit"))
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

    // Clone ecbuild (always external); metkit comes from the in-tree checkout
    // when available, falling back to a clone of the release tag.
    let ecbuild_src = bindman_utils::git_clone(ECBUILD_REPO, ECBUILD_TAG, &src_dir.join("ecbuild"));
    let metkit_src = resolve_metkit_src(&src_dir);

    // CMakeCache.txt pins the source path the build dir was configured with;
    // cmake hard-errors if it changes (e.g. switching between cloned and
    // in-tree sources). Wipe the build dir when the cached path is stale.
    if let Ok(cache) = fs::read_to_string(build_dir.join("CMakeCache.txt")) {
        let cached_src = cache
            .lines()
            .find_map(|l| l.strip_prefix("CMAKE_HOME_DIRECTORY:INTERNAL="));
        if cached_src != metkit_src.to_str() {
            fs::remove_dir_all(&build_dir).expect("Failed to remove stale metkit build directory");
            fs::create_dir_all(&build_dir).expect("Failed to create build directory");
        }
    }

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

fn main() {
    bindman_utils::reexport_dep_root("METKIT_SYS");
    // Rpaths for this crate's own test binaries; consumers emit their own.
    bindman_utils::emit_rpaths();
}

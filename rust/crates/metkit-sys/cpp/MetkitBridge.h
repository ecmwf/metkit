// metkit C++ bridge for Rust FFI — umbrella header pulled in by the
// cxx-generated bridge (`include!("MetkitBridge.h")` in lib.rs) and by
// downstream `-sys` crates. Real declarations live in the per-topic headers
// below.
#pragma once

#include "CodesHandle.h"
#include "HyperCube.h"
#include "MarsLanguage.h"
#include "MarsRequest.h"
#include "ParsedRequests.h"
#include "RequestEnvironment.h"

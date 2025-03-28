#include "metkit_version.h"

#ifdef __cplusplus
extern "C" {
#endif

const char* metkit_version() {
    return metkit_VERSION;
}

unsigned int metkit_version_int() {
    return 10000 * metkit_VERSION_MAJOR + 100 * metkit_VERSION_MINOR + 1 * metkit_VERSION_PATCH;
}

const char* metkit_version_str() {
    return metkit_VERSION_STR;
}

const char* metkit_git_sha1() {
    return metkit_GIT_SHA1;
}

#ifdef __cplusplus
}
#endif

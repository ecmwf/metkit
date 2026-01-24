// mars2grib_dict_capi.cc
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"

#include "metkit/mars2grib/Mars2Grib.h"

#include "mars2grib_constants.h"

extern "C" {

// ==========================
// helpers
// ==========================
static char* dup_string(const std::string& s) {
    try {
        char* p = static_cast<char*>(std::malloc(s.size() + 1));
        if (!p)
            return nullptr;
        std::memcpy(p, s.c_str(), s.size() + 1);
        return p;
    }
    catch (...) {
        return nullptr;
    }
}

template <typename T>
static T* dup_array(const std::vector<T>& v) {
    if (v.empty())
        return nullptr;
    T* p = static_cast<T*>(std::malloc(sizeof(T) * v.size()));
    if (!p)
        return nullptr;
    std::memcpy(p, v.data(), sizeof(T) * v.size());
    return p;
}

static const char** dup_string_array(const std::vector<std::string>& v) {
    if (v.empty())
        return nullptr;

    const char** arr = static_cast<const char**>(std::malloc(sizeof(char*) * v.size()));
    if (!arr)
        return nullptr;

    for (size_t i = 0; i < v.size(); ++i) {
        arr[i] = dup_string(v[i]);
        if (!arr[i]) {
            for (size_t j = 0; j < i; ++j)
                std::free((void*)arr[j]);
            std::free((void*)arr);
            return nullptr;
        }
    }
    return arr;
}

// ==========================
// opaque structs
// ==========================
struct Dict {
    int type;
    eckit::LocalConfiguration cfg;
};

struct Iterator {
    Dict* dict{nullptr};
    std::vector<std::string> keys;
    size_t idx{0};
};

struct Mars2GribHandle {
    std::unique_ptr<metkit::mars2grib::Mars2Grib> enc;
};

// ==========================
// memory free
// ==========================
void mars2grib_free(void* p) {
    if (p)
        std::free(p);
}

// ==========================
// create / destroy
// ==========================
int mars2grib_dict_create(void** dict, const char* dict_type) {
    if (!dict)
        return M2G_ERR_ARGS;
    if (!dict_type || dict_type[0] == '\0')
        return M2G_ERR_ARGS;
    try {
        switch (dict_type[0]) {
            case 'm':
                // mars
                *dict                           = new Dict();
                static_cast<Dict*>(*dict)->type = M2G_DICT_MARS;
                break;
            case 'g':
                // geom
                *dict                           = new Dict();
                static_cast<Dict*>(*dict)->type = M2G_DICT_GEOM;
                break;
            case 'p':
                // misc
                *dict                           = new Dict();
                static_cast<Dict*>(*dict)->type = M2G_DICT_MISC;
                break;
            case 'o':
                // opt
                *dict                           = new Dict();
                static_cast<Dict*>(*dict)->type = M2G_DICT_OPT;
                break;
            default:
                return M2G_ERR_ARGS;
        }
        return M2G_OK;
    }
    catch (...) {
        return M2G_ERR_EXC;
    }
}

int mars2grib_dict_destroy(void** dict) {
    if (!dict || !*dict)
        return M2G_OK;
    delete static_cast<Dict*>(*dict);
    *dict = nullptr;
    return M2G_OK;
}

// ==========================
// has → type id
// ==========================
static int detect_type(const eckit::LocalConfiguration& c, const std::string& k) {
    if (!c.has(k))
        return M2G_UNDEFINED;
    if (c.isString(k))
        return M2G_STRING;
    if (c.isBoolean(k))
        return M2G_BOOL;
    if (c.isIntegral(k))
        return M2G_LONG;
    if (c.isFloatingPoint(k))
        return M2G_DOUBLE;
    if (c.isStringList(k))
        return M2G_STRING_ARRAY;
    if (c.isIntegralList(k))
        return M2G_LONG_ARRAY;
    if (c.isFloatingPointList(k))
        return M2G_DOUBLE_ARRAY;
    return M2G_UNDEFINED;
}

int mars2grib_dict_has(void* dict, const char* key, int* type_id) {
    if (!dict || !key || !type_id)
        return M2G_ERR_ARGS;
    auto& cfg = static_cast<Dict*>(dict)->cfg;
    *type_id  = detect_type(cfg, key);
    return M2G_OK;
}

int mars2grib_dict_type(void* dict, const char** dictType, int* dictTypeId) {
    if (!dict || !dictType || !dictTypeId)
        return M2G_ERR_ARGS;
    *dictType = nullptr;
    auto& d   = *static_cast<Dict*>(dict);
    switch (d.type) {
        case M2G_DICT_MARS:
            *dictType   = M2G_DICT_MARS_STR;
            *dictTypeId = M2G_DICT_MARS;
            break;
        case M2G_DICT_GEOM:
            *dictType   = M2G_DICT_GEOM_STR;
            *dictTypeId = M2G_DICT_GEOM;
            break;
        case M2G_DICT_MISC:
            *dictType   = M2G_DICT_MISC_STR;
            *dictTypeId = M2G_DICT_MISC;
            break;
        case M2G_DICT_OPT:
            *dictType   = M2G_DICT_OPT_STR;
            *dictTypeId = M2G_DICT_OPT;
            break;
        default:
            *dictType   = nullptr;
            *dictTypeId = M2G_DICT_UNKNOWN;
            return M2G_ERR_ARGS;
    }
    return M2G_OK;
}

// ==========================
// iterator
// ==========================
int mars2grib_dict_iterator_begin(void* dict, void** it) {
    if (!dict || !it)
        return M2G_ERR_ARGS;
    if (*it) {
        delete static_cast<Iterator*>(*it);
        *it = nullptr;
    }
    Iterator* i = new Iterator();
    i->dict     = static_cast<Dict*>(dict);
    i->keys     = i->dict->cfg.keys();
    i->idx      = 0;
    *it         = i;
    return M2G_OK;
}

int mars2grib_dict_iterate(void* dict, void** it, char** key, void** value, int* type_id, int* len) {
    if (!dict || !it || !key || !value || !type_id || !len)
        return M2G_ERR_ARGS;

    Iterator* itr = static_cast<Iterator*>(*it);
    if (!itr) {
        mars2grib_dict_iterator_begin(dict, it);
        itr = static_cast<Iterator*>(*it);
    }

    if (itr->idx >= itr->keys.size()) {
        delete itr;
        *it      = nullptr;
        *key     = nullptr;
        *value   = nullptr;
        *type_id = M2G_UNDEFINED;
        *len     = 0;
        return M2G_OK;
    }

    const std::string& k = itr->keys[itr->idx++];
    *key                 = dup_string(k);

    auto& cfg = itr->dict->cfg;
    *type_id  = detect_type(cfg, k);
    *value    = nullptr;
    *len      = 1;

    switch (*type_id) {
        case M2G_STRING: {
            std::string s;
            cfg.get(k, s);
            *value = dup_string(s);
            break;
        }
        case M2G_BOOL: {
            bool b;
            cfg.get(k, b);
            long* v = static_cast<long*>(std::malloc(sizeof(long)));
            *v      = b ? 1 : 0;
            *value  = v;
            break;
        }
        case M2G_LONG: {
            long v;
            cfg.get(k, v);
            long* p = static_cast<long*>(std::malloc(sizeof(long)));
            *p      = v;
            *value  = p;
            break;
        }
        case M2G_DOUBLE: {
            double v;
            cfg.get(k, v);
            double* p = static_cast<double*>(std::malloc(sizeof(double)));
            *p        = v;
            *value    = p;
            break;
        }
        case M2G_STRING_ARRAY: {
            std::vector<std::string> v;
            cfg.get(k, v);
            *len = static_cast<int>(v.size());
            break;
        }
        case M2G_LONG_ARRAY: {
            std::vector<long> v;
            cfg.get(k, v);
            *len = static_cast<int>(v.size());
            break;
        }
        case M2G_DOUBLE_ARRAY: {
            std::vector<double> v;
            cfg.get(k, v);
            *len = static_cast<int>(v.size());
            break;
        }
        default:
            break;
    }

    return M2G_OK;
}

int mars2grib_dict_iterator_destroy(void* dict, void** iterator) {
    (void)dict; /* unused */

    if (!iterator || !*iterator)
        return M2G_OK;

    delete static_cast<Iterator*>(*iterator);
    *iterator = NULL;
    return M2G_OK;
}

// ==========================
// getters
// ==========================
int mars2grib_dict_get_string(void* d, const char* k, char** v) {
    if (!d || !k || !v)
        return M2G_ERR_ARGS;
    std::string s;
    if (!static_cast<Dict*>(d)->cfg.get(k, s))
        return M2G_ERR_NFOUND;
    *v = dup_string(s);
    return M2G_OK;
}

int mars2grib_dict_get_bool(void* d, const char* k, long* v) {
    if (!d || !k || !v)
        return M2G_ERR_ARGS;
    bool b;
    if (!static_cast<Dict*>(d)->cfg.get(k, b))
        return M2G_ERR_NFOUND;
    *v = b ? 1 : 0;
    return M2G_OK;
}

int mars2grib_dict_get_long(void* d, const char* k, long* v) {
    if (!d || !k || !v)
        return M2G_ERR_ARGS;
    if (!static_cast<Dict*>(d)->cfg.get(k, *v))
        return M2G_ERR_NFOUND;
    return M2G_OK;
}

int mars2grib_dict_get_double(void* d, const char* k, double* v) {
    if (!d || !k || !v)
        return M2G_ERR_ARGS;
    if (!static_cast<Dict*>(d)->cfg.get(k, *v))
        return M2G_ERR_NFOUND;
    return M2G_OK;
}

int mars2grib_dict_get_float(void* d, const char* k, float* v) {
    if (!d || !k || !v)
        return M2G_ERR_ARGS;
    if (!static_cast<Dict*>(d)->cfg.get(k, *v))
        return M2G_ERR_NFOUND;
    return M2G_OK;
}

// ==========================
// array getters
// ==========================
int mars2grib_dict_get_string_array(void* d, const char* k, const char*** v, int* n) {
    if (!d || !k || !v || !n)
        return M2G_ERR_ARGS;
    std::vector<std::string> a;
    if (!static_cast<Dict*>(d)->cfg.get(k, a))
        return M2G_ERR_NFOUND;
    *n = static_cast<int>(a.size());
    *v = dup_string_array(a);
    return M2G_OK;
}

int mars2grib_dict_get_long_array(void* d, const char* k, const long** v, int* n) {
    if (!d || !k || !v || !n)
        return M2G_ERR_ARGS;
    std::vector<long> a;
    if (!static_cast<Dict*>(d)->cfg.get(k, a))
        return M2G_ERR_NFOUND;
    *n = static_cast<int>(a.size());
    *v = dup_array(a);
    return M2G_OK;
}

int mars2grib_dict_get_double_array(void* d, const char* k, const double** v, int* n) {
    if (!d || !k || !v || !n)
        return M2G_ERR_ARGS;
    std::vector<double> a;
    if (!static_cast<Dict*>(d)->cfg.get(k, a))
        return M2G_ERR_NFOUND;
    *n = static_cast<int>(a.size());
    *v = dup_array(a);
    return M2G_OK;
}

int mars2grib_dict_get_float_array(void* d, const char* k, const float** v, int* n) {
    if (!d || !k || !v || !n)
        return M2G_ERR_ARGS;
    std::vector<float> a;
    if (!static_cast<Dict*>(d)->cfg.get(k, a))
        return M2G_ERR_NFOUND;
    *n = static_cast<int>(a.size());
    *v = dup_array(a);
    return M2G_OK;
}

// ==========================
// setters (scalars + arrays)
// ==========================
int mars2grib_dict_set_string(void* d, const char* k, const char* v) {
    static_cast<Dict*>(d)->cfg.set(k, v);
    return M2G_OK;
}
int mars2grib_dict_set_bool(void* d, const char* k, long v) {
    static_cast<Dict*>(d)->cfg.set(k, v != 0);
    return M2G_OK;
}
int mars2grib_dict_set_long(void* d, const char* k, long v) {
    static_cast<Dict*>(d)->cfg.set(k, v);
    return M2G_OK;
}
int mars2grib_dict_set_double(void* d, const char* k, double v) {
    static_cast<Dict*>(d)->cfg.set(k, v);
    return M2G_OK;
}
int mars2grib_dict_set_float(void* d, const char* k, float v) {
    static_cast<Dict*>(d)->cfg.set(k, v);
    return M2G_OK;
}

int mars2grib_dict_set_string_array(void* d, const char* k, const char* const* v, int n) {
    std::vector<std::string> a;
    for (int i = 0; i < n; i++)
        a.emplace_back(v[i]);
    static_cast<Dict*>(d)->cfg.set(k, a);
    return M2G_OK;
}
int mars2grib_dict_set_long_array(void* d, const char* k, const long* v, int n) {
    std::vector<long> a(v, v + n);
    static_cast<Dict*>(d)->cfg.set(k, a);
    return M2G_OK;
}
int mars2grib_dict_set_double_array(void* d, const char* k, const double* v, int n) {
    std::vector<double> a(v, v + n);
    static_cast<Dict*>(d)->cfg.set(k, a);
    return M2G_OK;
}
int mars2grib_dict_set_float_array(void* d, const char* k, const float* v, int n) {
    std::vector<float> a(v, v + n);
    static_cast<Dict*>(d)->cfg.set(k, a);
    return M2G_OK;
}

// ==========================
// serialisation
// ==========================
int mars2grib_dict_to_json(void* d, char** v) {
    std::ostringstream oss;
    oss << static_cast<Dict*>(d)->cfg;
    *v = dup_string(oss.str());
    return M2G_OK;
}

int mars2grib_dict_to_yaml(void* d, const char* f) {
    std::ofstream ofs(f);
    ofs << static_cast<Dict*>(d)->cfg;
    return M2G_OK;
}

int mars2grib_encoder_open(void* opt_dict, void** mars2grib) {
    if (!mars2grib)
        return M2G_ERR_ARGS;

    try {
        auto* h = new Mars2GribHandle();

        if (opt_dict) {
            auto* d = static_cast<Dict*>(opt_dict);
            if (d != nullptr && d->type != M2G_DICT_OPT) {
                delete h;
                return M2G_ERR_ARGS;
            }
            h->enc = std::make_unique<metkit::mars2grib::Mars2Grib>(d->cfg);
        }
        else {
            h->enc = std::make_unique<metkit::mars2grib::Mars2Grib>();
        }

        *mars2grib = h;
        return M2G_OK;
    }
    catch (...) {
        return M2G_ERR_EXC;
    }
}

// ------------------------------------------------------------
// encode double precision
// ------------------------------------------------------------
int mars2grib_encoder_encode64(void* mars2grib, void* mars_dict, void* misc_dict, void* geom_dict, const double* data,
                               long data_len, void** out_handle) {
    if (!mars2grib || !mars_dict || !geom_dict || !misc_dict || !data || !out_handle)
        return M2G_ERR_ARGS;

    try {
        auto* h    = static_cast<Mars2GribHandle*>(mars2grib);
        auto* mars = static_cast<Dict*>(mars_dict);
        auto* geom = static_cast<Dict*>(geom_dict);
        auto* misc = static_cast<Dict*>(misc_dict);

        if (mars->type != M2G_DICT_MARS)
            return M2G_ERR_ARGS;
        if (misc->type != M2G_DICT_MISC)
            return M2G_ERR_ARGS;
        if (geom->type != M2G_DICT_GEOM)
            return M2G_ERR_ARGS;
        if (data_len < 0)
            return M2G_ERR_ARGS;

        auto handle = h->enc->encode(mars->cfg, misc->cfg, geom->cfg, data, static_cast<size_t>(data_len));

        // release ownership to C
        *out_handle = handle.release();
        return M2G_OK;
    }
    catch (...) {
        return M2G_ERR_EXC;
    }
}

// ------------------------------------------------------------
// encode single precision
// ------------------------------------------------------------
int mars2grib_encoder_encode32(void* mars2grib, void* mars_dict, void* misc_dict, void* geom_dict, const float* data,
                               long data_len, void** out_handle) {
    if (!mars2grib || !mars_dict || !geom_dict || !misc_dict || !data || !out_handle)
        return M2G_ERR_ARGS;

    try {
        auto* h    = static_cast<Mars2GribHandle*>(mars2grib);
        auto* mars = static_cast<Dict*>(mars_dict);
        auto* geom = static_cast<Dict*>(geom_dict);
        auto* misc = static_cast<Dict*>(misc_dict);

        if (mars->type != M2G_DICT_MARS)
            return M2G_ERR_ARGS;
        if (misc->type != M2G_DICT_MISC)
            return M2G_ERR_ARGS;
        if (geom->type != M2G_DICT_GEOM)
            return M2G_ERR_ARGS;
        if (data_len < 0)
            return M2G_ERR_ARGS;

        auto handle = h->enc->encode(mars->cfg, misc->cfg, geom->cfg, data, static_cast<size_t>(data_len));

        *out_handle = handle.release();
        return M2G_OK;
    }
    catch (...) {
        return M2G_ERR_EXC;
    }
}

// ------------------------------------------------------------
// close encoder
// ------------------------------------------------------------
int mars2grib_encoder_close(void** mars2grib) {
    if (!mars2grib || !*mars2grib)
        return M2G_OK;

    try {
        delete static_cast<Mars2GribHandle*>(*mars2grib);
        *mars2grib = nullptr;
        return M2G_OK;
    }
    catch (...) {
        return M2G_ERR_EXC;
    }
}

}  // extern "C"

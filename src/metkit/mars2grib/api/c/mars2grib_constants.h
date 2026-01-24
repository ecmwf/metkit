#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "metkit/mars2grib/api/c/mars2grib_constant_values.h"

/* Public aliases */
#define M2G_OK           M2G_VALUE_OK
#define M2G_ERR_ARGS     M2G_VALUE_ERR_ARGS
#define M2G_ERR_ALLOC    M2G_VALUE_ERR_ALLOC

#define M2G_UNDEFINED    M2G_VALUE_UNDEFINED
#define M2G_STRING       M2G_VALUE_STRING
#define M2G_BOOL         M2G_VALUE_BOOL
#define M2G_INT          M2G_VALUE_INT
#define M2G_LONG         M2G_VALUE_LONG
#define M2G_FLOAT        M2G_VALUE_FLOAT
#define M2G_DOUBLE       M2G_VALUE_DOUBLE

#define M2G_STRING_ARRAY M2G_VALUE_STRING_ARRAY
#define M2G_LONG_ARRAY   M2G_VALUE_LONG_ARRAY
#define M2G_DOUBLE_ARRAY M2G_VALUE_DOUBLE_ARRAY

#define M2G_DICT_UNKNOWN M2G_VALUE_DICT_UNKNOWN
#define M2G_DICT_MARS    M2G_VALUE_DICT_MARS
#define M2G_DICT_GEOM    M2G_VALUE_DICT_GEOM
#define M2G_DICT_MISC    M2G_VALUE_DICT_MISC
#define M2G_DICT_OPT     M2G_VALUE_DICT_OPT

struct m2g_const_map {
    int         value;
    const char* name;
};

static const struct m2g_const_map m2g_const_table[] = {
    { M2G_VALUE_OK,               M2G_VALUE_OK_STR },
    { M2G_VALUE_ERR_ARGS,         M2G_VALUE_ERR_ARGS_STR },
    { M2G_VALUE_ERR_ALLOC,        M2G_VALUE_ERR_ALLOC_STR },

    { M2G_VALUE_UNDEFINED,        M2G_VALUE_UNDEFINED_STR },
    { M2G_VALUE_STRING,           M2G_VALUE_STRING_STR },
    { M2G_VALUE_BOOL,             M2G_VALUE_BOOL_STR },
    { M2G_VALUE_INT,              M2G_VALUE_INT_STR },
    { M2G_VALUE_LONG,             M2G_VALUE_LONG_STR },
    { M2G_VALUE_FLOAT,            M2G_VALUE_FLOAT_STR },
    { M2G_VALUE_DOUBLE,           M2G_VALUE_DOUBLE_STR },

    { M2G_VALUE_STRING_ARRAY,     M2G_VALUE_STRING_ARRAY_STR },
    { M2G_VALUE_LONG_ARRAY,       M2G_VALUE_LONG_ARRAY_STR },
    { M2G_VALUE_DOUBLE_ARRAY,     M2G_VALUE_DOUBLE_ARRAY_STR },

    { M2G_VALUE_DICT_UNKNOWN,     M2G_VALUE_DICT_UNKNOWN_STR },
    { M2G_VALUE_DICT_MARS,        M2G_VALUE_DICT_MARS_STR },
    { M2G_VALUE_DICT_GEOM,        M2G_VALUE_DICT_GEOM_STR },
    { M2G_VALUE_DICT_MISC,        M2G_VALUE_DICT_MISC_STR },
    { M2G_VALUE_DICT_OPT,         M2G_VALUE_DICT_OPT_STR }
};

static inline const char*
mars2grib_const_to_string(int value)
{
    for (unsigned i = 0; i < sizeof(m2g_const_table)/sizeof(m2g_const_table[0]); ++i)
        if (m2g_const_table[i].value == value)
            return m2g_const_table[i].name;
    return 0;
}

static inline int
mars2grib_const_from_string(const char* name, int* value)
{
    if (!name || !value) return -1;
    for (unsigned i = 0; i < sizeof(m2g_const_table)/sizeof(m2g_const_table[0]); ++i)
        if (strcmp(m2g_const_table[i].name, name) == 0) {
            *value = m2g_const_table[i].value;
            return 0;
        }
    return -1;
}

#ifdef __cplusplus
}
#endif

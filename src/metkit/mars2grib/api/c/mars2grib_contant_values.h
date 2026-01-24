#if 0
/* -------------------------
 * mars2grib_constant_values.h
 *
 * Single source of truth for:
 *   - numeric values
 *   - string representations
 *
 * This file is included by:
 *   - C headers
 *   - Fortran modules (via INCLUDE)
 *
 * Rules:
 *   - No C comments
 *   - No includes
 *   - No code
 *
 * if 0 is needed because fortran does not understand c-like comments
 * ------------------------- */
#endif

#if 0
/* -------------------------
 * Error codes
 * ------------------------- */
#endif
#define M2G_VALUE_OK 0
#define M2G_VALUE_OK_STR "ok"

#define M2G_VALUE_ERR_ARGS 1
#define M2G_VALUE_ERR_ARGS_STR "invalid-arguments"

#define M2G_VALUE_ERR_ALLOC 2
#define M2G_VALUE_ERR_ALLOC_STR "allocation-failed"

#if 0
/* -------------------------
 * Value type identifiers
 * ------------------------- */
#endif
#define M2G_VALUE_UNDEFINED (-1)
#define M2G_VALUE_UNDEFINED_STR "undefined"

#define M2G_VALUE_STRING 1
#define M2G_VALUE_STRING_STR "string"

#define M2G_VALUE_BOOL 2
#define M2G_VALUE_BOOL_STR "bool"

#define M2G_VALUE_INT 3
#define M2G_VALUE_INT_STR "int"

#define M2G_VALUE_LONG 4
#define M2G_VALUE_LONG_STR "long"

#define M2G_VALUE_FLOAT 5
#define M2G_VALUE_FLOAT_STR "float"

#define M2G_VALUE_DOUBLE 6
#define M2G_VALUE_DOUBLE_STR "double"

#if 0
/* -------------------------
 * Array value types
 * ------------------------- */
#endif
#define M2G_VALUE_STRING_ARRAY 101
#define M2G_VALUE_STRING_ARRAY_STR "string-array"

#define M2G_VALUE_LONG_ARRAY 104
#define M2G_VALUE_LONG_ARRAY_STR "long-array"

#define M2G_VALUE_DOUBLE_ARRAY 106
#define M2G_VALUE_DOUBLE_ARRAY_STR "double-array"

#if 0
/* -------------------------
 * Dictionary kinds
 * ------------------------- */
#endif
#define M2G_VALUE_DICT_UNKNOWN 0
#define M2G_VALUE_DICT_UNKNOWN_STR "unknown"

#define M2G_VALUE_DICT_MARS 1
#define M2G_VALUE_DICT_MARS_STR "mars"

#define M2G_VALUE_DICT_GEOM 2
#define M2G_VALUE_DICT_GEOM_STR "geom"

#define M2G_VALUE_DICT_MISC 3
#define M2G_VALUE_DICT_MISC_STR "misc"

#define M2G_VALUE_DICT_OPT 4
#define M2G_VALUE_DICT_OPT_STR "opt"

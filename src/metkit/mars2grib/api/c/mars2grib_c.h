#pragma once
#include <stdint.h>

#include "mars2grib_constants.h"

#ifdef __cplusplus
extern "C" {
#endif


// mars2grib dictionary
int mars2grib_dict_create(void** dict, const char* dict_type);
int mars2grib_dict_destroy(void** dict);

// mars2grib dictionary iterator
int mars2grib_dict_iterator_begin(void* dict, void** iterator);
int mars2grib_dict_iterate(void* dict, void** it, char** key, void** value, int* type_id, int* len);
int mars2grib_dict_iterator_destroy(void* dict, void** iterator);


// mars2grib dictionary has key
int mars2grib_dict_type(void* dict, const char** dictType, int* dictTypeId);
int mars2grib_dict_has(void* dict, const char* key, int* type_id);

// mars2grib dictionary string getter
int mars2grib_dict_get_string(void* dict, const char* key, char** value);
int mars2grib_dict_get_bool(void* dict, const char* key, long* value);
int mars2grib_dict_get_long(void* dict, const char* key, long* value);
int mars2grib_dict_get_double(void* dict, const char* key, double* value);
int mars2grib_dict_get_float(void* dict, const char* key, float* value);

int mars2grib_dict_get_string_array(void* dict, const char* key, const char*** value, int* vlen);
int mars2grib_dict_get_long_array(void* dict, const char* key, const long** value, int* vlen);
int mars2grib_dict_get_double_array(void* dict, const char* key, const double** value, int* vlen);
int mars2grib_dict_get_float_array(void* dict, const char* key, const float** value, int* vlen);


// mars2grib dictionary string setter
int mars2grib_dict_set_string(void* dict, const char* key, const char* value);
int mars2grib_dict_set_bool(void* dict, const char* key, long value);
int mars2grib_dict_set_long(void* dict, const char* key, long value);
int mars2grib_dict_set_double(void* dict, const char* key, double value);
int mars2grib_dict_set_float(void* dict, const char* key, float value);

int mars2grib_dict_set_string_array(void* dict, const char* key, const char* const* value, int vlen);
int mars2grib_dict_set_long_array(void* dict, const char* key, const long* value, int vlen);
int mars2grib_dict_set_double_array(void* dict, const char* key, const double* value, int vlen);
int mars2grib_dict_set_float_array(void* dict, const char* key, const float* value, int vlen);

int mars2grib_dict_to_yaml(void* dict, const char* fname);
int mars2grib_dict_to_json(void* dict, char** value);


// options dict can be null, in this case default options can be applied
int mars2grib_encoder_open(void* opt_dict, void** mars2grib);
int mars2grib_encoder_encode64(void* mars2grib, void* mars_dict, void* misc_dict, void* geom_dict, const double* data,
                               long data_len, void** out_handle);
int mars2grib_encoder_encode32(void* mars2grib, void* mars_dict, void* misc_dict, void* geom_dict, const float* data,
                               long data_len, void** out_handle);
int mars2grib_encoder_close(void** mars2grib);

// Utility to free allocated memory
void mars2grib_free(void* p);

#ifdef __cplusplus
} /* extern "C" */
#endif
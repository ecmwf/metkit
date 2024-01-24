#ifndef METKIT_API_METKIT_C_H
#define METKIT_API_METKIT_C_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------------------------------------------------
 * TYPES
 * -----*/

struct Request;
typedef struct Request metkit_request_t;

struct metkit_requestiterator_t;
typedef struct metkit_requestiterator_t metkit_requestiterator_t;

struct StringVecIterator;
typedef struct StringVecIterator metkit_paramiterator_t;
typedef struct StringVecIterator metkit_valueiterator_t;

/* ---------------------------------------------------------------------------------------------------------------------
 * ERROR HANDLING
 * -------------- */

typedef enum metkit_error_values_t
{
    METKIT_SUCCESS            = 0, /* Operation succeded. */
    METKIT_ITERATION_COMPLETE = 1, /* All elements have been returned */
    METKIT_ERROR              = 2, /* Operation failed. */
    METKIT_ERROR_UNKNOWN      = 3, /* Failed with an unknown error. */
    METKIT_ERROR_USER         = 4, /* Failed with an user error. */
    METKIT_ERROR_ASSERT       = 5  /* Failed with an assert() */
} metkit_error_enum_t;

const char* metkit_get_error_string(int err);

/* -----------------------------------------------------------------------------
 * HELPERS
 * ------- */

int metkit_version(const char** version); 

int metkit_vcs_version(const char** sha1);

int metkit_initialise();

/* ---------------------------------------------------------------------------------------------------------------------
 * PARSING
 * --- */

int metkit_parse_mars(metkit_requestiterator_t** requests, const char* str);

/* ---------------------------------------------------------------------------------------------------------------------
 * REQUEST
 * --- */

int metkit_free_request(const metkit_request_t* request);

int metkit_request_get_verb(const metkit_request_t* request, const char** verb);

int metkit_request_get_params(const metkit_request_t* request, metkit_paramiterator_t** params);

int metkit_request_get_values(const metkit_request_t* request, const char* param, metkit_valueiterator_t** values);

/* ---------------------------------------------------------------------------------------------------------------------
 * REQUEST ITERATOR
 * --- */

int metkit_free_requestiterator(const metkit_requestiterator_t* list);

int metkit_requestiterator_next(metkit_requestiterator_t* list);

int metkit_requestiterator_request(const metkit_requestiterator_t* list, metkit_request_t** request);

/* ---------------------------------------------------------------------------------------------------------------------
 * PARAM ITERATOR
 * --- */

int metkit_free_paramiterator(const metkit_paramiterator_t* list);

int metkit_paramiterator_next(metkit_paramiterator_t* list);

int metkit_paramiterator_param(const metkit_paramiterator_t* list, const char** param);

/* ---------------------------------------------------------------------------------------------------------------------
 * VALUE ITERATOR
 * --- */

int metkit_free_valueiterator(const metkit_valueiterator_t* list);

int metkit_valueiterator_next(metkit_valueiterator_t* list);

int metkit_valueiterator_value(const metkit_valueiterator_t* list, const char** value);

/* -------------------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* METKIT_API_METKIT_C_H */
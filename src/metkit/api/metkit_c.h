#ifndef METKIT_API_METKIT_C_H
#define METKIT_API_METKIT_C_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------------------------------------------------
 * TYPES
 * -----*/

struct metkit_marsrequest_t;
typedef struct metkit_marsrequest_t metkit_marsrequest_t;

struct metkit_requestiterator_t;
/** RequestIterator for iterating over vector of Request instances */
typedef struct metkit_requestiterator_t metkit_requestiterator_t;

struct metkit_paramiterator_t;
/** Iterator for iterating over parameters in Request */
typedef struct metkit_paramiterator_t metkit_paramiterator_t;

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

/**
 * @brief Set MetKit version.
 *
 * @param version Version string
 * @return int Error code
 */
int metkit_version(const char** version);

/**
 * @brief Set MetKit git sha1 version.
 *
 * @param sha1 SHA1 version string
 * @return int Error code
 */
int metkit_vcs_version(const char** sha1);

/**
 * @brief Initialise Main() context.
 *
 * @note This is ONLY required when Main() is NOT initialised, such as loading
 * the MetKit as shared library in Python.
 * @return int Error code
 */
int metkit_initialise();

/* ---------------------------------------------------------------------------------------------------------------------
 * PARSING
 * --- */

/**
 * Parse MARS requests into RequestIterator of Request instances. Resulting RequestIterator
 * must be deallocated with metkit_free_requestiterator
 * @param str MARS requests
 * @param requests Allocates RequestIterator object
 * @return int Error code
 */
int metkit_parse_marsrequest(const char* str, metkit_requestiterator_t** requests, bool strict = false);

/* ---------------------------------------------------------------------------------------------------------------------
 * REQUEST
 * --- */

/** Allocates new Request object. Must be deallocated with mekit_free_request
 * @param request new Request instance
 * @return int Error code
 */
int metkit_new_marsrequest(metkit_marsrequest_t** request);

/** Deallocates Request object and associated resources.
 * @param request Request instance
 * @return int Error code
 */
int metkit_free_marsrequest(const metkit_marsrequest_t* request);

/** Add parameter and values to request
 * @param request Request instance
 * @param param parameter name
 * @param values array of values for parameter
 * @param numValues number of values
 * @return int Error code
 */
int metkit_marsrequest_add(metkit_marsrequest_t* request, const char* param, const char* values[], int numValues);

/** Add parameter and values to request
 * @param request Request instance
 * @param param parameter name
 * @param values value to add
 * @return int Error code
 */
int metkit_marsrequest_add1(metkit_marsrequest_t* request, const char* param, const char* value);

/** Set verb in Request object
 * @param request Request instance
 * @param verb verb to set
 * @return int Error code
 */
int metkit_marsrequest_set_verb(metkit_marsrequest_t* request, const char* verb);

/** Returns the verb in Request object
 * @param request Request instance
 * @param verb verb in request
 * @return int Error code
 */
int metkit_marsrequest_verb(const metkit_marsrequest_t* request, const char** verb);

/** Returns whether parameter is in Request object
 * @param request Request instance
 * @param param parameter name
 * @param has whether parameter exists in request
 * @return int Error code
 */
int metkit_marsrequest_has_param(const metkit_marsrequest_t* request, const char* param, bool* has);

/** Returns ParamIterator of parameters in request. Resulting ParamIterator
 * must be deallocated with metkit_free_paramiterator
 * @param request Request instance
 * @param params Allocates ParamIterator object for parameter names in request
 * @return int Error code
 */
int metkit_marsrequest_params(const metkit_marsrequest_t* request, metkit_paramiterator_t** params);

/** Returns number of values for specific parameter in Request object
 * @param request Request instance
 * @param param parameter name in request
 * @param count number of values for param in request
 * @return int Error code
 */
int metkit_marsrequest_count_values(const metkit_marsrequest_t* request, const char* param, size_t* count);

/** Returns value for specific parameter and index in Request object
 * @param request Request instance
 * @param param parameter name in request
 * @param index index of value to retrieve for param in request
 * @param value retrieved value
 * @return int Error code
 */
int metkit_marsrequest_value(const metkit_marsrequest_t* request, const char* param, int index, const char** value);

/** Returns values for specific parameter Request object
 * @param request Request instance
 * @param param parameter name in request
 * @param values array of values for param in request
 * @param numValues number of values for param in request
 * @return int Error code
 */
int metkit_marsrequest_values(const metkit_marsrequest_t* request, const char* param, const char** values[], size_t* numValues);

/** Populates empty Request object by expanding existing request
 * @param request Request instance to be expanded
 * @param expandedRequest empty Request instance to be populated
 * @param inherit if true, populate expanded request with default values
 * @param strict it true, raise error rather than warning on invalid values
 * @return int Error code
 */
int metkit_marsrequest_expand(const metkit_marsrequest_t* request, metkit_marsrequest_t* expandedRequest, bool inherit = true, bool strict = false);

/** Merges other Request object into existing request
 * @param request Request instance to contain result of merge
 * @param otherRequest other Request instance to merge
 * @return int Error code
 */
int metkit_marsrequest_merge(metkit_marsrequest_t* request, const metkit_marsrequest_t* otherRequest);

/* ---------------------------------------------------------------------------------------------------------------------
 * REQUEST ITERATOR
 * --- */

/** Deallocates RequestIterator object and associated resources.
 * @param it RequestIterator instance
 * @return int Error code
 */
int metkit_free_requestiterator(const metkit_requestiterator_t* it);

/** Moves to the next Request element in RequestIterator
 * @param it RequestIterator instance
 * @return int Error code
 */
int metkit_requestiterator_next(metkit_requestiterator_t* it);

/** Populates empty Requestion object with data from current element in RequestIterator
 * @param it RequestIterator instance
 * @param request empty Request instance to populate with data
 * @return int Error code
 */
int metkit_requestiterator_request(metkit_requestiterator_t* it, metkit_marsrequest_t* request);

/* ---------------------------------------------------------------------------------------------------------------------
 * PARAM ITERATOR
 * --- */

/** Deallocates ParamIterator object and associated resources.
 * @param it ParamIterator instance
 * @return int Error code
 */
int metkit_free_paramiterator(const metkit_paramiterator_t* it);

/** Moves to the next string element in ParamIterator
 * @param it ParamIterator instance
 * @return int Error code
 */
int metkit_paramiterator_next(metkit_paramiterator_t* it);

/** Returns the current parameter name in ParamIterator
 * @param it ParamIterator instance
 * @param param current parameter name in iterator
 * @return int Error code
 */
int metkit_paramiterator_param(metkit_paramiterator_t* it, const char** param);

#ifdef __cplusplus
}
#endif

#endif /* METKIT_API_METKIT_C_H */
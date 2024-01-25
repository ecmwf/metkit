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
/** RequestIterator for iterating over vector of Request instances */
typedef struct metkit_requestiterator_t metkit_requestiterator_t;

struct StringVecIterator;
/** Iterators for vector of std::string for iterating over parameters and values in Request */
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
 * @brief Parse MARS requests into RequestIterator of MarsRequest instances
 *
 * @param requests RequestIterator instance
 * @param str MARS requests
 * @return int Error code
 */
int metkit_parse_mars(metkit_requestiterator_t** requests, const char* str);

/* ---------------------------------------------------------------------------------------------------------------------
 * REQUEST
 * --- */

/** Deallocates Request object and associated resources.
 * @param request Request instance
 * @return int Error code
 */
int metkit_free_request(const metkit_request_t* request);

/** Returns the verb in Request object
 * @param request Request instance
 * @param verb verb in request
 * @return int Error code
 */
int metkit_request_get_verb(const metkit_request_t* request, const char** verb);

/** Returns list of parameter names in Request object
 * @param request Request instance
 * @param params ParamIterator instance for parameter names in request
 * @return int Error code
 */
int metkit_request_get_params(const metkit_request_t* request, metkit_paramiterator_t** params);

/** Returns list of values for specific parameter in Request object
 * @param request Request instance
 * @param param parameter name in request
 * @param values ValueIterator instance for values for param in request
 * @return int Error code
 */
int metkit_request_get_values(const metkit_request_t* request, const char* param, metkit_valueiterator_t** values);

/* ---------------------------------------------------------------------------------------------------------------------
 * REQUEST ITERATOR
 * --- */

/** Deallocates RequestIterator object and associated resources.
 * @param list RequestIterator instance
 * @return int Error code
 */
int metkit_free_requestiterator(const metkit_requestiterator_t* list);

/** Moves to the next Request element in RequestIterator
 * @param list RequestIterator instance
 * @return int Error code
 */
int metkit_requestiterator_next(metkit_requestiterator_t* list);

/** Returns the current Request element in RequestIterator
 * @param list RequestIterator instance
 * @param request current Request element in list
 * @return int Error code
 */
int metkit_requestiterator_request(const metkit_requestiterator_t* list, metkit_request_t** request);

/* ---------------------------------------------------------------------------------------------------------------------
 * PARAM ITERATOR
 * --- */

/** Deallocates ParamIterator object and associated resources.
 * @param list ParamIterator instance
 * @return int Error code
 */
int metkit_free_paramiterator(const metkit_paramiterator_t* list);

/** Moves to the next string element in ParamIterator
 * @param list ParamIterator instance
 * @return int Error code
 */
int metkit_paramiterator_next(metkit_paramiterator_t* list);

/** Returns the current parameter name in ParamIterator
 * @param list ParamIterator instance
 * @param param current parameter name in list
 * @return int Error code
 */
int metkit_paramiterator_param(const metkit_paramiterator_t* list, const char** param);

/* ---------------------------------------------------------------------------------------------------------------------
 * VALUE ITERATOR
 * --- */

/** Deallocates ValueIterator object and associated resources.
 * @param list ValueIterator instance
 * @returns int Error code
 */
int metkit_free_valueiterator(const metkit_valueiterator_t* list);

/** Moves to the next string element in ValueIterator
 * @param list ValueIterator instance
 * @return int Error code
 */
int metkit_valueiterator_next(metkit_valueiterator_t* list);

/** Returns the current value in ValueIterator
 * @param list ValueIterator instance
 * @param value current value in list
 * @return int Error code
 */
int metkit_valueiterator_value(const metkit_valueiterator_t* list, const char** value);

/* -------------------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* METKIT_API_METKIT_C_H */
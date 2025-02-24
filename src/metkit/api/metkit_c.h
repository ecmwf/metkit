#pragma once

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------------------------------------------------
 * TYPES
 * -----*/

struct metkit_marsrequest_t;
typedef struct metkit_marsrequest_t metkit_marsrequest_t;

struct metkit_selection_t;
typedef struct metkit_selection_t metkit_selection_t;

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
    METKIT_ERROR              = 1, /* Operation failed. */
    METKIT_ERROR_UNKNOWN      = 2, /* Failed with an unknown error. */
    METKIT_ERROR_USER         = 3, /* Failed with an user error. */
    METKIT_ERROR_ASSERT       = 4  /* Failed with an assert() */
} metkit_error_t;

typedef enum metkit_iterator_status_t
{
    METKIT_ITERATOR_SUCCESS  = 0, /* Operation succeded. */
    METKIT_ITERATOR_COMPLETE = 1, /* All elements have been returned */
    METKIT_ITERATOR_ERROR    = 2  /* Operation failed. */
} metkit_iterator_status_t;


const char* metkit_get_error_string(enum metkit_error_values_t err);

/* -----------------------------------------------------------------------------
 * HELPERS
 * ------- */

/**
 * @brief Get metkit version.
 *
 * @return const char* version string
 */
const char* metkit_version();

/**
 * @brief Get metkit git sha1 version.
 *
 * @return const char* git sha1 version string
 */
const char* metkit_git_sha1();

/**
 * @brief Initialise Main() context.
 *
 * @note This is ONLY required when Main() is NOT initialised, such as loading
 * the MetKit as shared library in Python.
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_initialise();

/* ---------------------------------------------------------------------------------------------------------------------
 * PARSING
 * --- */

/**
 * Parse MARS requests into RequestIterator of Request instances. Resulting RequestIterator
 * must be deallocated with metkit_delete_requestiterator
 * @param str MARS requests
 * @param[out] requests Allocates RequestIterator object
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_parse_marsrequests(const char* str, metkit_requestiterator_t** requests, bool strict);

/* ---------------------------------------------------------------------------------------------------------------------
 * REQUEST
 * --- */

/** Allocates new Request object. Must be deallocated with mekit_delete_request
 * @param[out] request new Request instance
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_marsrequest_new(metkit_marsrequest_t** request);

/** Deallocates Request object and associated resources.
 * @param request Request instance
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_marsrequest_delete(const metkit_marsrequest_t* request);

/** Add parameter and values to request
 * @param request Request instance
 * @param param parameter name
 * @param values array of values for parameter
 * @param numValues number of values
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_marsrequest_set(metkit_marsrequest_t* request, const char* param, const char* values[], int numValues);

/** Add parameter and values to request
 * @param request Request instance
 * @param param parameter name
 * @param values value to add
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_marsrequest_set_one(metkit_marsrequest_t* request, const char* param, const char* value);

/** Set verb in Request object
 * @param request Request instance
 * @param verb verb to set
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_marsrequest_set_verb(metkit_marsrequest_t* request, const char* verb);

/** Returns the verb in Request object
 * @param request Request instance
 * @param[out] verb verb in request
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_marsrequest_verb(const metkit_marsrequest_t* request, const char** verb);

/** Returns whether parameter is in Request object
 * @param request Request instance
 * @param param parameter name
 * @param[out] has whether parameter exists in request
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_marsrequest_has_param(const metkit_marsrequest_t* request, const char* param, bool* has);


/** Returns number of parameters in Request object
 * @param request Request instance
 * @param[out] count number of parameters in request
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_marsrequest_count_params(const metkit_marsrequest_t* request, size_t* count);

/** Returns parameter name for specific index in Request object
 * @param request Request instance
 * @param index index of parameter to retrieve
 * @param[out] param parameter name
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_marsrequest_param(const metkit_marsrequest_t* request, size_t index, const char** param);


/** Returns number of values for specific parameter in Request object
 * @param request Request instance
 * @param param parameter name in request
 * @param[out] count number of values for param in request
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_marsrequest_count_values(const metkit_marsrequest_t* request, const char* param, size_t* count);

/** Returns value for specific parameter and index in Request object
 * @param request Request instance
 * @param param parameter name in request
 * @param index index of value to retrieve for param in request
 * @param[out] value retrieved value
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_marsrequest_value(const metkit_marsrequest_t* request, const char* param, int index, const char** value);

/** Populates empty Request object by expanding existing request
 * @param request Request instance to be expanded
 * @param inherit if true, populate expanded request with default values
 * @param strict it true, raise error rather than warning on invalid values
 * @param[out] expandedRequest empty Request instance to be populated
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_marsrequest_expand(const metkit_marsrequest_t* request,  bool inherit, bool strict, metkit_marsrequest_t* expandedRequest);


/** Mutate an input Request object by expanding it.
 * @param[out] request Request instance to be expanded
 * @param inherit if true, populate expanded request with default values
 * @param strict it true, raise error rather than warning on invalid values
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_marsrequest_expand_self(metkit_marsrequest_t* request,  bool inherit, bool strict);

/** Merges other Request object into existing request
 * @param request Request instance to contain result of merge
 * @param otherRequest other Request instance to merge
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_marsrequest_merge(metkit_marsrequest_t* request, const metkit_marsrequest_t* otherRequest);



// XXXXX
// TODO 
// UPDATE THESE DOCS FOR SELECTION
// XXXXXX

/* ---------------------------------------------------------------------------------------------------------------------
 * SELECTION
 * --- */

/** Allocates new Request object. Must be deallocated with mekit_delete_request
 * @param[out] request new Request instance
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_selection_new(metkit_selection_t** request);

/** Deallocates Request object and associated resources.
 * @param request Request instance
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_selection_delete(const metkit_selection_t* request);

/** Add parameter and values to request
 * @param request Request instance
 * @param param parameter name
 * @param values array of values for parameter
 * @param numValues number of values
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_selection_set(metkit_selection_t* request, const char* param, const char* values[], int numValues);

/** Returns number of values for specific parameter in Request object
 * @param request Request instance
 * @param param parameter name in request
 * @param[out] count number of values for param in request
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_selection_count_values(const metkit_selection_t* request, const char* param, size_t* count);

/** Returns value for specific parameter and index in Request object
 * @param request Request instance
 * @param param parameter name in request
 * @param index index of value to retrieve for param in request
 * @param[out] value retrieved value
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_selection_value(const metkit_selection_t* request, const char* param, int index, const char** value);

/** Mutate an input Request object by expanding it.
 * @param request Request instance to be expanded
 * @param inherit if true, populate expanded request with default values
 * @param strict it true, raise error rather than warning on invalid values
 * @param[out] expandedRequest empty Request instance to be populated
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_selection_expand_self(metkit_selection_t* request,  bool inherit, bool strict);



/** Populates empty Request object by expanding existing request
 * @param request Request instance to be expanded
 * @param inherit if true, populate expanded request with default values
 * @param strict it true, raise error rather than warning on invalid values
 * @param[out] expandedRequest empty Request instance to be populated
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_selection_expand(const metkit_selection_t* request,  bool inherit, bool strict, metkit_selection_t* expandedRequest);


/* ---------------------------------------------------------------------------------------------------------------------
 * REQUEST ITERATOR
 * --- */

/** Deallocates RequestIterator object and associated resources.
 * @param it RequestIterator instance
 * @return metkit_error_t Error code
 */
metkit_error_t metkit_delete_requestiterator(const metkit_requestiterator_t* it);

/** Moves to the next Request element in RequestIterator
 * @param it RequestIterator instance
 * @return metkit_iterator_status_t Status of iterator
 */
// metkit_error_t metkit_requestiterator_next(metkit_requestiterator_t* it);
metkit_iterator_status_t metkit_requestiterator_next(metkit_requestiterator_t* it);

/** Populates empty Requestion object with data from current element in RequestIterator
 * @param it RequestIterator instance
 * @param request empty Request instance to populate with data
 * @return metkit_iterator_status_t Status of iterator
 * @note must call metkit_requestiterator_next before calling metkit_requestiterator_current.
 *
 * Example usage:
    * while (metkit_requestiterator_next(it) == METKIT_ITERATOR_SUCCESS) {
    *     metkit_marsrequest_t* req{};
    *     metkit_marsrequest_new(&req);
    *     metkit_requestiterator_current(it, req);
    *     // use req ...
    * }
 */
metkit_iterator_status_t metkit_requestiterator_current(metkit_requestiterator_t* it, metkit_marsrequest_t* request);

#ifdef __cplusplus
}
#endif

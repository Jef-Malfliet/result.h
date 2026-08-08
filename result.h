/*
 * result.h — single-header Result<T> type for C.
 *
 * Usage:
 *     REGISTER_RESULT_TYPE(int)   // once per type, per translation unit
 *
 *     Result_int divide(int a, int b) {
 *         if (b == 0) return Result_int_failure_value("division by zero");
 *         return Result_int_success_value(a / b);
 *     }
 *
 *     Result_int r = divide(10, 0);
 *     if (Result_int_is_success(&r)) {
 *         printf("%d\n", Result_int_unwrap(&r));
 *     } else {
 *         fprintf(stderr, "%s\n", Result_int_unwrap_error(&r));
 *     }
 */

#ifndef RESULT_H_
#define RESULT_H_
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*
    Internal helper macro that builds the name of the generated Result
    struct for a given type, e.g. RESULT_TYPE_INTERNAL(int) -> Result_int.
    Not intended to be used directly outside this file.
*/
#define RESULT_TYPE_INTERNAL(t_name) Result_##t_name

// #region Result type registration macros

/*
Generates a `Result_<t_name>` type plus its constructors, mutators, and unwrap functions.

Must only be registered once per t_name per translation unit. Calling `REGISTER_RESULT_TYPE(int)` twice in the same .c file will emit a duplicate-typedef / redefinition compiler error. If multiple .c files need Result_int, put the `REGISTER_RESULT_TYPE(int)` call in a single shared header/unit, or guard it so it's only expanded once.

`t_name` must be a single identifier token (e.g. int, float, MyStruct). Multi-word built-in types such as `unsigned int` or `long long` cannot be used directly, since t_name is pasted into identifiers via `##` and the preprocessor cannot paste multiple tokens into one. Typedef the type first and register the typedef instead:

```C
    typedef unsigned int uint;
    REGISTER_RESULT_TYPE(uint)
```
*/
#define REGISTER_RESULT_TYPE(t_name) \
    SETUP_RESULT_STRUCT(t_name)      \
    SETUP_RESULT_SUCCESS(t_name)     \
    SETUP_RESULT_FAILURE(t_name)

// #endregion

// #region Result struct

/*
    Tag identifying which state a Result is in and, by extension, which
    member of its `contents` union (if any) is valid to read.
*/
typedef enum
{
    RESULT_TAG_SUCCESS,
    RESULT_TAG_SUCCESS_EMPTY,
    RESULT_TAG_FAILED,
    RESULT_TAG_FAILED_EMPTY
} ResultTag;

/*
    `contents.value` is valid only when Result.tag == RESULT_TAG_SUCCESS. `contents.error_message` is valid only when tag == RESULT_TAG_FAILED. When tag == RESULT_TAG_SUCCESS_EMPTY or RESULT_TAG_FAILED_EMPTY, `contents` is unset, so do not read it.
*/
#define SETUP_RESULT_STRUCT(t_name)    \
    typedef struct                     \
    {                                  \
        ResultTag tag;                 \
        union                          \
        {                              \
            t_name value;              \
            const char *error_message; \
        } contents;                    \
    } RESULT_TYPE_INTERNAL(t_name);

// #endregion

// #region Success

/*
Generates the success-side API for a Result_<t_name>:

  `Result_<t_name>_success(void)`
      Builds a successful Result with no payload (tag = RESULT_TAG_SUCCESS_EMPTY).

  `Result_<t_name>_success_value(t_name value)`
      Builds a successful Result carrying `value` (tag = RESULT_TAG_SUCCESS).

  `Result_<t_name>_set_success(Result_<t_name> *result)`
      Overwrites *result in place with an empty success, for callers that already own a Result_<t_name> slot (e.g. a local variable) rather than receiving one as a return value.

  `Result_<t_name>_set_success_value(Result_<t_name> *result, t_name value)`
      Overwrites *result in place with a success and a value, for callers that already own a Result_<t_name> slot (e.g. a local variable) rather than receiving one as a return value.

  `Result_<t_name>_unwrap(Result_<t_name> *r)`
      Returns the wrapped value. Aborts with a diagnostic message if `r` is not in the RESULT_TAG_SUCCESS state, so callers should check Result_<t_name>_is_success() first if failure is expected.

  `Result_<t_name>_is_success(Result_<t_name> *r)`
      Returns true if the Result has a success tag.
*/
#define SETUP_RESULT_SUCCESS(t_name)                                                                            \
    static inline RESULT_TYPE_INTERNAL(t_name) Result_##t_name##_success(void)                                  \
    {                                                                                                           \
        Result_##t_name result;                                                                                 \
        result.tag = RESULT_TAG_SUCCESS_EMPTY;                                                                  \
                                                                                                                \
        return result;                                                                                          \
    }                                                                                                           \
                                                                                                                \
    static inline RESULT_TYPE_INTERNAL(t_name) Result_##t_name##_success_value(t_name value)                    \
    {                                                                                                           \
        Result_##t_name result = Result_##t_name##_success();                                                   \
        result.tag = RESULT_TAG_SUCCESS;                                                                        \
        result.contents.value = value;                                                                          \
                                                                                                                \
        return result;                                                                                          \
    }                                                                                                           \
                                                                                                                \
    static inline void Result_##t_name##_set_success(RESULT_TYPE_INTERNAL(t_name) * result)                     \
    {                                                                                                           \
        *result = Result_##t_name##_success();                                                                  \
    }                                                                                                           \
                                                                                                                \
    static inline void Result_##t_name##_set_success_value(RESULT_TYPE_INTERNAL(t_name) * result, t_name value) \
    {                                                                                                           \
        *result = Result_##t_name##_success_value(value);                                                       \
    }                                                                                                           \
                                                                                                                \
    static inline t_name Result_##t_name##_unwrap(RESULT_TYPE_INTERNAL(t_name) * r)                             \
    {                                                                                                           \
        if (r->tag != RESULT_TAG_SUCCESS)                                                                       \
        {                                                                                                       \
            fprintf(stderr, "Result_" #t_name "_unwrap called on non-success or empty success result\n");       \
            abort();                                                                                            \
        }                                                                                                       \
                                                                                                                \
        return r->contents.value;                                                                               \
    }                                                                                                           \
                                                                                                                \
    static inline bool Result_##t_name##_is_success(RESULT_TYPE_INTERNAL(t_name) * r)                           \
    {                                                                                                           \
        return r->tag == RESULT_TAG_SUCCESS || r->tag == RESULT_TAG_SUCCESS_EMPTY;                              \
    }

// #endregion

// #region Failure

/*
Generates the failure-side API for a Result_<t_name>, mirroring SETUP_RESULT_SUCCESS:

  `Result_<t_name>_failure(void)`
      Builds a failed Result with no message (tag = RESULT_TAG_FAILED_EMPTY).

  `Result_<t_name>_failure_value(const char *error_message)`
      Builds a failed Result carrying `error_message` (tag = RESULT_TAG_FAILED).

  `Result_<t_name>_set_failure(Result_<t_name> *result)`
      Overwrites *result in place with an empty failure.

  `Result_<t_name>_set_failure_value(Result_<t_name> *result, const char *error_message)`
       Overwrites *result in place with a failure and a message.

  `Result_<t_name>_unwrap_error(Result_<t_name> *r)`
      Returns the error message. Aborts with a diagnostic message if `r` is not in the RESULT_TAG_FAILED state, so callers should check Result_<t_name>_is_failure() first if success is expected.

  `Result_<t_name>_is_failure(Result_<t_name> *r)`
      Returns true if the Result has a failure tag.
*/
#define SETUP_RESULT_FAILURE(t_name)                                                                                         \
    static inline RESULT_TYPE_INTERNAL(t_name) Result_##t_name##_failure(void)                                               \
    {                                                                                                                        \
        Result_##t_name result;                                                                                              \
        result.tag = RESULT_TAG_FAILED_EMPTY;                                                                                \
                                                                                                                             \
        return result;                                                                                                       \
    }                                                                                                                        \
                                                                                                                             \
    static inline RESULT_TYPE_INTERNAL(t_name) Result_##t_name##_failure_value(const char *error_message)                    \
    {                                                                                                                        \
        Result_##t_name result = Result_##t_name##_failure();                                                                \
        result.tag = RESULT_TAG_FAILED;                                                                                      \
        result.contents.error_message = error_message;                                                                       \
                                                                                                                             \
        return result;                                                                                                       \
    }                                                                                                                        \
                                                                                                                             \
    static inline void Result_##t_name##_set_failure(RESULT_TYPE_INTERNAL(t_name) * result)                                  \
    {                                                                                                                        \
        *result = Result_##t_name##_failure();                                                                               \
    }                                                                                                                        \
                                                                                                                             \
    static inline void Result_##t_name##_set_failure_value(RESULT_TYPE_INTERNAL(t_name) * result, const char *error_message) \
    {                                                                                                                        \
        *result = Result_##t_name##_failure_value(error_message);                                                            \
    }                                                                                                                        \
                                                                                                                             \
    static inline const char *Result_##t_name##_unwrap_error(RESULT_TYPE_INTERNAL(t_name) * r)                               \
    {                                                                                                                        \
        if (r->tag != RESULT_TAG_FAILED)                                                                                     \
        {                                                                                                                    \
            fprintf(stderr, "Result_" #t_name "_unwrap_error called on non-error or empty error result\n");                  \
            abort();                                                                                                         \
        }                                                                                                                    \
                                                                                                                             \
        return r->contents.error_message;                                                                                    \
    }                                                                                                                        \
                                                                                                                             \
    static inline bool Result_##t_name##_is_failure(RESULT_TYPE_INTERNAL(t_name) * r)                                        \
    {                                                                                                                        \
        return r->tag == RESULT_TAG_FAILED || r->tag == RESULT_TAG_FAILED_EMPTY;                                             \
    }

// #endregion

#endif // RESULT_H_
# result.h

A single-header `Result<T>` type for C.

result.h gives you a small `Result_<T>` struct. A Result shows the result of an operation. A Result can be a success or a failure. A success can have a value. A failure can have an error message.

## Function of result.h

result.h gives you:

- The macro `REGISTER_RESULT_TYPE(t_name)`. This macro makes a Result type for the type `t_name`.
- Utility functions to 
  - make a success Result.
  - make a failure Result.
  - change an existing Result to a success or a failure.
  - get the value from a success Result.
  - get the error message from a failure Result.
  - check if a Result is a success or a failure.

## How to Start

1. Put the file `result.h` in your project.
2. Add this line to your C file: `#include "result.h"`.
3. Use the macro `REGISTER_RESULT_TYPE` to make a Result type for the type you need.

```c
#include "result.h"

REGISTER_RESULT_TYPE(int)

Result_int divide(int a, int b)
{
    if (b == 0)
        return Result_int_failure_value("division by zero");

    return Result_int_success_value(a / b);
}

int main(void)
{
    Result_int r = divide(10, 0);

    if (Result_int_is_success(&r))
        printf("%d\n", Result_int_unwrap(&r));
    else
        fprintf(stderr, "%s\n", Result_int_unwrap_error(&r));

    return 0;
}
```

## How to Register a Type

Use the macro `REGISTER_RESULT_TYPE(t_name)` to make a Result type for the type `t_name`.

Follow these steps:

1. Call `REGISTER_RESULT_TYPE(t_name)` one time only for each type, in each C file. If you call it two times for the same type in the same C file, the compiler will show an error.
2. If more than one C file needs the same Result type, put the call to `REGISTER_RESULT_TYPE` in one shared header file. Do not put the same call in more than one C file.
3. The value of `t_name` must be one word. Do not use a type name with more than one word, for example `unsigned int` or `long long`.

If your type has more than one word in its name, do this:

1. Make a `typedef` for the type. Give the `typedef` a name with one word only.
2. Call `REGISTER_RESULT_TYPE` with the name of the `typedef`.

This is an example:

```c
typedef unsigned int uint;
REGISTER_RESULT_TYPE(uint)
```

## The Result Struct

When you call `REGISTER_RESULT_TYPE(t_name)`, the macro makes a struct with the name `Result_<t_name>`.

The struct has two parts:

- A tag. The tag shows the state of the Result.
- A union with the name `contents`, which has two parts:
  - `value`, with the type `t_name`. Read `value` only if the tag is `RESULT_TAG_SUCCESS`.
  - `error_message`, with the type `const char *`. Read `error_message` only if the tag is `RESULT_TAG_FAILED`.

If the tag is `RESULT_TAG_SUCCESS_EMPTY` or `RESULT_TAG_FAILED_EMPTY`, the union `contents` is not set. Do not read the union `contents` in this condition.

## Functions for a Success Result

When you call `REGISTER_RESULT_TYPE(t_name)`, the macro makes these functions:

| Function | Function of the Function |
|---|---|
| `Result_<t_name>_success(void)` | This function makes a success Result with no value. |
| `Result_<t_name>_success_value(t_name value)` | This function makes a success Result with the value `value`. |
| `Result_<t_name>_set_success(Result_<t_name> *result)` | This function changes `*result` to a success Result with no value. |
| `Result_<t_name>_set_success_value(Result_<t_name> *result, t_name value)` | This function changes `*result` to a success Result with the value `value`. |
| `Result_<t_name>_unwrap(Result_<t_name> *r)` | This function gives you the value of a success Result. If `r` is not a success Result with a value, the function stops the program. Before you call this function, call `Result_<t_name>_is_success(r)` to check the state of `r`. |
| `Result_<t_name>_is_success(Result_<t_name> *r)` | This function tells you if `r` is a success Result. |

## Functions for a Failure Result

When you call `REGISTER_RESULT_TYPE(t_name)`, the macro also makes these functions:

| Function | Function of the Function |
|---|---|
| `Result_<t_name>_failure(void)` | This function makes a failure Result with no error message. |
| `Result_<t_name>_failure_value(const char *error_message)` | This function makes a failure Result with the error message `error_message`. |
| `Result_<t_name>_set_failure(Result_<t_name> *result)` | This function changes `*result` to a failure Result with no error message. |
| `Result_<t_name>_set_failure_value(Result_<t_name> *result, const char *error_message)` | This function changes `*result` to a failure Result with the error message `error_message`. |
| `Result_<t_name>_unwrap_error(Result_<t_name> *r)` | This function gives you the error message of a failure Result. If `r` is not a failure Result with an error message, the function stops the program. Before you call this function, call `Result_<t_name>_is_failure(r)` to check the state of `r`. |
| `Result_<t_name>_is_failure(Result_<t_name> *r)` | This function tells you if `r` is a failure Result. |

## Tags of a Result

A Result has one of four tags:

- `RESULT_TAG_SUCCESS`: a success Result with a value.
- `RESULT_TAG_SUCCESS_EMPTY`: a success Result with no value.
- `RESULT_TAG_FAILED`: a failure Result with an error message.
- `RESULT_TAG_FAILED_EMPTY`: a failure Result with no error message.

Use the empty tags when you do not need to send a value or an error message. For example, use an empty tag when a function would otherwise return `void` or when the reason an operation failed doesn't matter.

The function `Result_<t_name>_is_success` gives the result `true` for the tags `RESULT_TAG_SUCCESS` and `RESULT_TAG_SUCCESS_EMPTY`.

The function `Result_<t_name>_is_failure` gives the result `true` for the tags `RESULT_TAG_FAILED` and `RESULT_TAG_FAILED_EMPTY`.

The function `Result_<t_name>_unwrap` works only for the tag `RESULT_TAG_SUCCESS`. The function `Result_<t_name>_unwrap_error` works only for the tag `RESULT_TAG_FAILED`. For all other tags, these two functions stop the program.

## How to Register More Than One Type

You can call `REGISTER_RESULT_TYPE` more than one time. Call it one time for each type. Each type gets its own set of functions.

This is an example:

```c
REGISTER_RESULT_TYPE(int)
REGISTER_RESULT_TYPE(float)

typedef struct { int x, y; } Point;
REGISTER_RESULT_TYPE(Point)
```
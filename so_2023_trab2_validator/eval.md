# Using the `eval` tools

The `eval.c` and `eval.h` files provide 


## The `EVAL_CATCH()` macro

The `EVAL_CATCH()` macro allows an arbitrary function to be called catching all (most?) situations that would lead the program to halt or block. Specifically:

1. Calling `exit()` inside the function will cause the function to stop and return execution after the macro
2. `SIGSEGV`, `SIGBUS`, `SIGFPE` and `SIGILL` signals received while executing the function are caught
3. The function will be stopped after a time set by the `_eval_env.timeout` variable.

The `_eval_env.stat` variable can be used to determine how the function was terminated:

+ `0` - Indicates normal termination
+ `1` - Indicates the function was terminated by a call to `exit()`. In this case, the `_eval_exit_data.status` variable will store the exit status.
+ `2` - Indicates the function was terminated by a signal. In this case, the variable `_eval_env.signal` holds the value of the signal that caused the termination.

In case of failure, the macro will print out information regarding the cause of failure. The macro must be called as:

```C
EVAL_CATH( _code )
```

Where `_code` is any function or sequence of instructions that we wish to test. Here is a very simple example:

```C
#include "eval.h"

int main() {

    char *p, a;

    printf("Test with p = -1...\n");
    eval_reset();
    EVAL_CATCH( p = (char *) -1; *p = 0; );
    printf("%s\n", _eval_env.stat ? "Test failed" : "Test ok" );

    printf("Test with p = &a...\n");
    eval_reset();
    EVAL_CATCH( p = &a; *p = 0; );
    printf("%s\n", _eval_env.stat ? "Test failed" : "Test ok" );
}
}
```

Compiling and running the code gives:

```bash
$ gcc -Wall -lm test-eval.c eval.c 
$ ./a.out 
Test with p = -1...
[✗] Segmentation fault (SIGSEGV)
Test failed
Test with p = &a...
Test ok
```

### Timeouts

The code being tested in the `EVAL_CATCH()` macro will need to be completed before `_eval_env.timeout` seconds have passed. Setting this variable to 0 will disable this behavior and the function will be allowed to run indefinitely.

The `eval_reset()` command will set the timeout value (`_eval_env.timeout`) to the compile time constant `EVAL_TIMEOUT` which defaults to 1.0s. You can change this value either at compile time by adding `-DEVAL_TIMEOUT=time` to the compiler options, or before calling the `EVAL_CATCH()` macro.

Note that in the case of a timeout the function will be terminated by a `SIGPROF` signal. For this reason, the user code is not allowed to use `SIGPROF`.


## The `EVAL_CATCH_IO()` macro

The `EVAL_CATH_IO()` macro works just like the `EVAL_CATH()` macro, but it also allows redirecting (standard) input and/or output from/to specific files. You should use this macro to test functions that will accept input from stdin and/or output to stdout.

The macro should be called as follows:

```C
EVAL_CATCH_IO( _code, _fstdin, _fstdout )
````

Where `_code` has the same behavior as in the `EVAL_CATCH()` macro and `_fstdin`/`_fstdout` are the names (strings) of the files to be used for `stdin` and `stdout`, respectively. If either are set to `NULL` then no redirection takes place.

Here is a very simple example:

```C
#include "eval.h"

int read_int() {
    int res;
    if ( scanf("%d", &res ) < 1 ) {
        fprintf(stderr,"Invalid integer\n");
        exit(1);
    };
    return res;
}

int main() {

    int res = 0;

    // Use "1234\n" as stdin
    FILE *f = fopen("test.stdin","w");
    fputs("1234", f );
    fclose( f );

    eval_reset();
    EVAL_CATCH_IO( res = read_int(); , "test.stdin", NULL )
    if ( _eval_env.stat ) {
        fprintf(stderr,"Test failed\n");
    } else {
        printf("Read value %d\n", res );
    }

    unlink( "test.stdin");
}
```

## Function Wrappers

Besides the `EVAL_CATCH*()` family of macros, the `eval` toolkit provides a set of function wrappers for multiple system functions that allow building simple tests to evaluate if these functions are being called correctly by the function being tested. In their simplest form, these macros allow capturing the arguments and return values used when calling these functions, so that these may be checked later against correct values. Additionally, these macros can be configured for other behaviors, such as returning error or success values without actually calling the underlying function, and counting how many times the function was called.

The specific behavior of each of the functions in the `eval` toolkit is controlled through specific `_eval_*_data` global variables that are described in the next section. The toolkit provides an `eval_reset()` function that resets all the functions to the default behavior, except for the `pause()`, `execl()`, `raise()`, and `kill()` functions. See the `eval_reset()` function for details.

### The `_eval_*_data` variables

For every function wrapped by the `eval` toolkit, there is an associated `_eval_func_data` global variable, where `func` is the name of the function that was wrapped. They all have the same general format:

```C
typedef struct {
    int action;         // User configurable function action
    int status;         // Number of times the function has been called
    
    return_type ret;    // Return value for the function (if the function is not void) 

    type_a param_a;     // Arguments used when calling the function, if any
    type_b param_b;
    ...

    extra fields;       // Additional optional fields to capture additional information
    ...
} _eval_func_type;
```

For example, the variable for the `int sleep( int seconds )` function is defined as:

```C
typedef struct {
    int action;
    int status;
    int ret;

    unsigned int seconds;
} _eval_sleep_type;

extern _eval_sleep_type _eval_sleep_data;
```

#### `.action` field

The `.action` field is used to control how the function wrapper is supposed to behave. The default behavior (`.action = 0`) is to capture the function arguments, call the normal function, and store the return value (if any) in the `.ret` field.

Specific wrappers may define alternate behaviors (see the documentation for each function). For example, if the user calls the `sleep(5)` function with `_eval_sleep_data.action = 1`, the function will return immediately, setting `_eval_sleep_data.seconds = 5` and `_eval_sleep_data.ret = 0`.

#### `.status` field

The `.status` field is used to count how many times a function is called. Each time a function is called, the corresponding `.status` field is incremented.

The only exception is the `exit()` function, which will be called at most once. For this function, the `.status` field will hold the exit status intended (i.e. the parameter used when calling `exist()`).

#### `.ret` field

If the function is not `void` then the variable will also include a `.ret` field of the appropriate type to store the return value of the function (if the actual function is called) or some other value depending on the `.action` field. 

Note that if a function is called multiple times only the last call will be kept. If you need to keep track of multiple function calls you will need to use the logging functionality (see for example the `signal()` function). 

#### Function parameters

If a function accepts any parameters (e.g. `int seconds` for the `sleep()` function) then the associated variable will also capture the values used when issuing the function call.

Again, as in the behavior of the `.ret` field, only the values for the last function call will be kept.



## Reporting test errors

When evaluating the implementation of some specific function, we will typically call the function inside an `EVAL_CATCH()` macro several times with different combinations of parameters and other options (e.g. missing files). If the function does not behave as expected, an error should be logged using the `eval_error()` command, which uses the same syntax as the `printf()` function, but also increments the `_eval_stats.error` variable (see below for more details on the `eval_error()` function).

Once all tests have been run, you should call the `eval_complete(char msg[])` function. If all tests worked (i.e. `_eval_stats.error` is 0) the routine will print out a success message, otherwise it will print an error message indicating how many errors were found. See below for more details on the `eval_complete()` function.

Here is a simple example:

```C
...

eval_reset(); // This also resets _eval_stats.error

// Test with paramA

... // Set test conditions

EVAL_CATCH( test_function( paramA ) );
if ( ! proper_behavior_A )
    eval_error("Testing test_function( paramA ) failed" );

// Test with paramB

... // Set test conditions

EVAL_CATCH( test_function( paramB ) );
if ( ! proper_behavior_B )
    eval_error("Testing test_function( paramB ) failed" );

...

// nerr has the total number of errors
nerr += eval_complete("test_function()");

```

## Logs

There are some specific tests where a given function must be called several times with different parameters. Since the `_eval_*_data` variables will only store information regarding the last time a function was called, the toolkit adds a simple logging functionality that can be used to test for these situations.

The toolkit makes available 3 logs (success, error and data). The first two (success and error) are generally used by the functions being tested for issuing success and error messages, and the last (data) is generally used by the wrapper functions to log the function name, calling parameters and/or return values.

### Data log



## Wrapped functions

With only a few exceptions (e.g. the `exit()` function) all of the functions in the eval toolkit have as a default behavior:

1. Capturing the parameters of the function call in `_eval_*_data` elements of the same name.
2. Calling the underlying function
3. Capturing the return value and storing it in `_eval_*_data.ret` (if the function is not `void`)

The `_eval_*_data.status` field will be incremented by 1.

### `exit( int status )`

The `exit()` function is a special case of the `eval` toolkit given that the underlying function will never be called. Instead, the function will be terminated and execution returned to the `EVAL_CATCH()` macro.

Function options:
+ `.action = 1` - Print info message with exit status

Fields in `_eval_exit_data`:
+ `.status` - Function exit status, i.e., `exit()` function parameter

### `sleep( unsigned int seconds )`

Function options:
+ `.action = 1` - Return 0 immediately without calling `sleep()`

Fields in `_eval_sleep_data`:
+ `.ret`     - Return value of the function
+ `.seconds` - Value of the `seconds` parameter

### `fork( )`

Function options:
+ `.action = 3` - Return -1 immediately without calling `fork()` (i.e. issue error)
+ `.action = 2` - Return 1 immediately without calling `fork()` (i.e. behave as parent process)
+ `.action = 1` - Return 0 immediately without calling `fork()` (i.e. behave as child process)

Fields in `_eval_fork_data`:
+ `.ret`     - Return value of the function

### `wait(int *stat_loc)`

__Note__: The wrapper does not modify `*stat_loc` which means that the use of the `WIFEXITED()`, `WEXITSTATUS()`, etc. macros are not supported.

Function options:
+ `.action = 2` - Return immediately without changing `.ret` (i.e. behave as if the process with pid `.ret` had finished)
+ `.action = 1` - Return 0 immediately without calling `wait()`

Fields in `_eval_wait_data`:
+ `.ret`      - Return value of the function
+ `.stat_loc` - Value of the `stat_loc` parameter

### `waitpid( pid_t pid, int *stat_loc, int options )`

__Note__: The wrapper does not modify `*stat_loc` which means that the use of the `WIFEXITED()`, `WEXITSTATUS()`, etc. macros are not supported.

Function options:
+ `.action = 1` - Return `pid` immediately without calling `waitpid()`

Fields in `_eval_waitpid_data`:
+ `.ret`      - Return value of the function
+ `.pid`      - Value of the `pid` parameter
+ `.stat_loc` - Value of the `stat_loc` parameter
+ `.options`  - Value of the `options` parameter

### `kill(pid_t pid, int sig)`

Function options:
+ `.action = 3` - Log call parameters in `datalog` and return 0 without calling `kill()`
+ `.action = 2` - Return 0 immediately without calling `kill()`
+ `.action = 1` - Prevent routine from signaling self, parent, process group and every process belonging to process owner

Fields in `_eval_kill_data`:
+ `.ret`      - Return value of the function
+ `.pid`      - Value of the `pid` parameter
+ `.sig`      - Value of the `sig` parameter

### `raise( int sig )`

Function options:
+ `.action = 2` - Return 0 immediately without calling `raise()`
+ `.action = 1` - Issue error and return 0 without calling `raise()`

Fields in `_eval_raise_data`:
+ `.ret`      - Return value of the function
+ `.sig`      - Value of the `sig` parameter

### `signal(int signum, sighandler_t handler )`

The function will prevent arming the `SIGPROF` signal, which is used by the `EVAL_CATCH()` macros to implement timeouts. Should the user try to arm `SIGPROF` the routine will issue an error, returning `SIG_ERR` and setting `errno = EINVAL`.

Function options:
+ `.action = 2` - Log call parameters in `datalog` and return `SIG_DFL` without calling `signal()`
+ `.action = 1` - Return `SIG_DFL` without calling `signal()`

Fields in `_eval_signal_data`:
+ `.ret`      - Return value of the function
+ `.signum`   - Value of the `signum` parameter
+ `.handler`  - Value of the `handler` parameter


### `sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)`

The function will prevent arming the `SIGPROF` signal, which is used by the `EVAL_CATCH()` macros to implement timeouts. Should the user try to arm `SIGPROF` the routine will issue an error, returning -1 and setting `errno = EINVAL`.

Function options:
+ `.action = 2` - Log call parameters in `datalog` and return 0 without calling `sigaction()`. Note: unless `act -> sa_flags` had the `SA_SIGINFO` bit set, the function will be logged as if the corresponding `signal()` call was made.
+ `.action = 1` - Return 0 without calling `sigaction()`

Fields in `_eval_sigaction_data`:
+ `.ret`      - Return value of the function
+ `.signum`   - Value of the `signum` parameter
+ `.act`      - Value of the `act` parameter
+ `.oldact`   - Value of the `oldact` parameter


### `pause()`

Function options:
+ `.action = 2` - Issue error message and abort function.
+ `.action = 1` - Return -1 without calling `pause()`

Fields in `_eval_pause_data`:
+ `.ret`      - Return value of the function

### `alarm( unsigned int seconds )`

Function options:
+ `.action = 1` - Return `seconds` without calling `alarm()`

Fields in `_eval_alarm_data`:
+ `.ret`      - Return value of the function
+ `.seconds`  - Value of the `seconds` parameter

### `mkfifo(const char *path, mode_t mode)`

Function options:
+ `.action = 1` - Return 0 without calling `mkfifo()`

Fields in `_eval_mkfifo_data`:
+ `.ret`      - Return value of the function
+ `.path`     - Copy of the value of the `path` parameter
+ `.mode`     - Value of the `mode` parameter

### `S_ISFIFO(mode)`

Function options:
+ `.action = 1` - Return 1 without calling `S_ISFIFO()`

Fields in `_eval_isfifo_data`:
+ `.ret`      - Return value of the macro
+ `.mode`     - Value of the `mode` parameter

### `remove(const char * path)`

Function options:
+ `.action = 2` - Log call parameters and return 0 without calling `remove()`. Note that the function call is logged as if the corresponding `unlink()` function call was made.
+ `.action = 1` - Return 0 without calling `remove()`

Fields in `_eval_remove_data`:
+ `.ret`      - Return value of the function
+ `.path`     - Copy of the value of the `path` parameter

### `unlink(const char * path)`

Function options:
+ `.action = 2` - Log call parameters and return 0 without calling `unlink()`
+ `.action = 1` - Return 0 without calling `unlink()`

Fields in `_eval_unlink_data`:
+ `.ret`      - Return value of the function
+ `.path`     - Copy of the value of the `path` parameter

### `atoi(const char *nptr )`

Since `atoi()` is unsafe, the routine will first check for a valid string. If the supplied string is NULL or is longer than 32 characters the routine will issue an error message and return -1.

Function options:
+ none

Fields in `_eval_atoi_data`:
+ `.ret`      - Return value of the function
+ `.nptr`     - Copy of the value of the `nptr` parameter

### `fclose( FILE* stream )`

The routine will first check for a valid pointer. On error, the routine will issue an error message and return -1.

Function options:
+ none

Fields in `_eval_fclose_data`:
+ `.ret`      - Return value of the function

### `fread( void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream )`

The routine will validate the parameters (valid pointers, etc.) before calling `fread()`. On error, the function will return 0 without calling `fread()`.

Function options:
+ none

Fields in `_eval_fread_data`:
+ `.ret`      - Return value of the function
+ `.ptr`      - Value of the `ptr` parameter
+ `.size`     - Value of the `size` parameter
+ `.nmemb`    - Value of the `nmemb` parameter
+ `.stream`   - Value of the `stream` parameter


### `execl(const char *path, ... )``

The default settings implemented by the `eval_reset()` function will prevent the test function from calling this function, instead issuing an error and terminating the function.

The function can be made to call `execl()` by setting `.action = 0` but unless the call fails this will replace the current process with a new one. Unless a new process was forked before this happens, this will of course stop the tests.

Function options:
+ `.action = 2` - Issue an error message and terminate the function. This is the behavior set by `eval_reset()`
+ `.action = 1` - Return -1 without calling `execl()`

Fields in `_eval_unlink_data`:
+ `.ret`      - Return value of the function. Note that this will only be updated in case of failure.




## Additional functions

### `eval_reset()`

The `eval_reset()` function resets all functions to the default behavior and clears all counters. Specifically:

+ All `.action` and `.status` variables are set to 0, as are all remaining fields of the `_eval_*_data` variables
+ `_eval_stats.error` and `_eval_stats.info` are set to 0

Additionally, the function will:

+ Set the timeout value to the `EVAL_TIMEOUT` macro. To disable timeouts you can compile the code with `-DEVAL_TIMEOUT=0``
+ Block the execution of `pause()` and `execl()` by setting their `.action` values to 2, as these would stop or destroy the current test.
+ Prevent signals to self by setting the `raise()` and `kill()` `.action` values to 1, to avoid stopping the test.

Note that you can change all of these behaviors by modifying the corresponding `_eval_*_data` variable before calling the `EVAL_CATCH()` macro.

### `eval_checkptr()`

The `eval_checkptr()` checks if a pointer is valid by reading and writing 1 byte from/to the specified address. The syntax is as follows:

```C
int eval_checkptr( void* ptr )
````

The function will return one of the following values:

+ 0 - pointer is valid
+ 1 - NULL pointer
+ 2 - (-1) pointer
+ 3 - Segmentation fault when accessing pointer
+ 4 - Bus Error when accessing pointer
+ 5 - Invalid signal caught (should never happen)

### `eval_error()`

Prints out an information message. It precedes the message with "[✗]" and sets the color to red. The `_eval_stats.error` variable is incremented.

The syntax is the same as the `prinft()` function:

```C
int eval_error(const char *restrict format, ...)
```

The function returns the updated value of `_eval_stats.error`

### `eval_info()`

Prints out an information message. It precedes the message with "[ℹ︎]" and sets the color to blue. The `_eval_stats.info` variable is incremented.

The syntax is the same as the `prinft()` function:

```C
int eval_info(const char *restrict format, ...)
```

The function returns the updated value of `_eval_stats.info`

### `eval_success()`

Prints out a success message. It precedes the message with "[✔]" and sets the color to green. The `_eval_stats.info` variable is incremented.

The syntax is the same as the `printf()` function:

```C
int eval_success(const char *restrict format, ...)
```

The function returns the updated value of `_eval_stats.info`

### `create_lockfile()` / `remove_lockfile()`

These functions allow creating/removing a "locked" file, i.e., a file with `000` permissions. The syntaxes are as follows:

```C
int create_lockfile( const char * filename );
int remove_lockfile( const char * filename );
```

These are usually used to test the detection of write or read errors. The file is created with 4 bytes and the characters `LOCK`. 

Please note that the file __can still be removed by `unlink()` or `remove()`__. The `remove_lockfile()` will simply check if the file still exists before removing it.

Both functions return 0 on success, and -1 on error.
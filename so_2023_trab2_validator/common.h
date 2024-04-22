#ifndef __COMMON_EVAL_H__
#define __COMMON_EVAL_H__

#include "/home/so/trabalho-2023-2024/utils/parte-2/common.h"
#include "eval.h"

/**
 * Disable warning: ‘%s’ directive output may be truncated
 * which may be triggered by the sucesso() and erro() macros
 **/
#ifdef __GNUC__
#if __GNUC__ > 7
#pragma GCC diagnostic ignored "-Wformat-truncation"
#pragma GCC diagnostic push
#endif
#endif

#undef so_success
#undef so_error
// #undef so_debug

#ifdef _EVAL_DEBUG

// #define so_debug(fmt, ...) do { printf(COLOUR_BACK_FAINT_GRAY "@DEBUG:%s:%d:%s():" COLOUR_GRAY " [" fmt "]" COLOUR_NONE "\n", __FILE__, __LINE__, __func__, ## __VA_ARGS__); } while (0)

#define so_success(passo,fmt, ...) do {\
    printf(COLOUR_BACK_GREEN "@SUCCESS {" passo "}" COLOUR_GREEN " [" fmt "]" COLOUR_NONE "\n", ## __VA_ARGS__);\
    snprintf( newline( &_success_log ), LOGLINE,"(" passo ") " fmt, ## __VA_ARGS__); \
} while(0)

#define so_error(passo,fmt, ...) do {\
    printf(COLOUR_BACK_BOLD_RED "@ERROR {" passo "}" COLOUR_RED " [" fmt "]" COLOUR_NONE "\n", ## __VA_ARGS__); if (errno) perror("(SO_Erro)");\
    snprintf( newline( &_error_log ), LOGLINE,"(" passo ") " fmt, ## __VA_ARGS__); \
} while(0)

#else  // _EVAL_DEBUG

// #define so_debug(fmt, ...) do { } while (0)

#define so_success(passo,fmt, ...) do {\
    snprintf( newline( &_success_log ), LOGLINE,"(" passo ") " fmt, ## __VA_ARGS__); \
} while(0)

#define so_error(passo,fmt, ...) do {\
    snprintf( newline( &_error_log ), LOGLINE,"(" passo ") " fmt, ## __VA_ARGS__); \
} while(0)

#endif // _EVAL_DEBUG


/**
 * The replacement versions of exit_on_error() and exit_on_null() work the same way
 * but silence the ouput
 */

#undef so_exit_on_error
#define so_exit_on_error(status, errorMsg) do { \
    if (status < 0) { \
        exit(-1); \
    } \
} while (0)

#undef so_exit_on_null
#define so_exit_on_null(status, errorMsg) do { \
    if (NULL == status) { \
        exit(-1); \
    } \
} while (0)

#undef so_rand
#define so_rand() (RAND_MAX/2)

#undef so_rand_between_values
#define so_rand_between_values( a, b ) (b)

#endif // __COMMON_EVAL_H__
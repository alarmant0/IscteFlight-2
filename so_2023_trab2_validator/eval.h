/******************************************************************************
 ** ISCTE-IUL: Trabalho prático de Sistemas Operativos
 ** Avaliação automática
 ******************************************************************************/

#ifndef __EVAL_H__
#define __EVAL_H__

// Default timeout for student code
#ifndef EVAL_TIMEOUT
#define EVAL_TIMEOUT 1.0
#endif

// Enable printing additional messages
//#define _EVAL_DEBUG 1

#define _EVAL_DEBUG_PREFIX "\033[33m  ⇢ \033[0m"
#define _NO_EXIT 0
#define _EXIT 1
#define _SUCCESS 0
#define _ERROR -1
#define _NOT_FOUND 0

#ifndef _EVAL_DEBUG
    #define SO_HIDE_DEBUG
#endif

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <sys/stat.h>
#include <sys/time.h>

#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <setjmp.h>

#ifdef __linux__
// LINUX definition of PATH_MAX
#include <linux/limits.h>
#else
// FreeBSD / Darwin definitino of PATH_MAX
#include <sys/syslimits.h>
#endif

#define LOGSIZE 128
#define LOGLINE 256

typedef struct {
    char key[16];
    char text[128];
    float grade;
} question_t;

#define MAX_QUESTIONS 100

int question_find( question_t questions[], char key[] );
char *question_text( question_t questions[], char key[] );
int question_setgrade( question_t questions[], char key[], float grade );
int question_list( question_t questions[], char* msg );
void question_export( question_t questions[], char msg[] );

typedef struct {
    char buffer[LOGSIZE][LOGLINE];
    int start;
    int end;
} log_t;

extern log_t _success_log;
extern log_t _error_log;
extern log_t _data_log;

void initlog( log_t * );
char *newline( log_t * );
void printlog( log_t * );
int rmheadmsg( log_t*, const char [] );
int findinlog( log_t*, const char *restrict, ... );

const char* loghead( log_t* );

int create_lockfile( char * );
int remove_lockfile( char * );

typedef struct {
    int error;
    int info;
    int success;
} eval_stats_t;

extern eval_stats_t _eval_stats;

void eval_reset_stats();


int eval_error(const char *restrict format, ...);
int eval_info(const char *restrict format, ...);
int eval_success(const char *restrict format, ...);
int eval_complete( char[] );

int datalog(const char *restrict format, ...);
int successlog(const char *restrict format, ...);
int errorlog(const char *restrict format, ...);

int eval_check_successlog( const char *restrict format, ...);
int eval_check_errorlog( const char *restrict format, ...);


void eval_clear_logs( void );
void eval_close_logs( char msg[] );

#define EVAL_VAR( name ) _eval_##name##_type _eval_##name##_data

#define RESET_VAR( name ) memset(&(_eval_##name##_data), 0, sizeof(_eval_##name##_type))

void eval_reset_vars();
void eval_reset();

/**
 * Convert a a macro definition to a string
 * This requires a double-expansion
 **/
#define _TOSTRING(x) #x
#define TOSTRING(x) _TOSTRING(x)

typedef struct {
    jmp_buf jmp;
    int signal;
    int stat;

    struct sigaction sigactions[5];
    float timeout;
} _eval_env_type;

extern _eval_env_type _eval_env;

void _eval_sighandler( int sig, siginfo_t *info, void *ucontext);

void _eval_arm_signals( );
void _eval_disarm_signals( );


int eval_checkptr( void* ptr );


typedef struct {
    int stdin;
    int stdout;
    int stdin_old;
    int stdout_old;
} _eval_stdio_t;

int _eval_io_redirect( const char* , const char*  );
int _eval_io_restore();

#define FILE_STDIN "eval.stdin"
#define FILE_STDOUT "eval.stdout"
#define FILE_DEVNULL "/dev/null"

#ifdef _EVAL_DEBUG

/**
 * EVAL_CATCH( code ) macro
 * Runs the specified code catching:
 * - exit() calls
 * - SIGSEGV, SIGBUS, SIGFPE and SIGILL signals
 **/

#define EVAL_CATCH( _code ) { \
    _eval_exit_data.status = -1; \
    _eval_env.signal = -1; \
    _eval_arm_signals(); \
    printf("\033[1;33m ⊢ \033[0m %s running...\n", #_code ); \
    _eval_env.stat = sigsetjmp( _eval_env.jmp, 1 ); \
    if ( !_eval_env.stat ) { \
        {_code;} \
        printf("\033[1;33m ⊣ \033[0m %s completed normally.\n", #_code); \
    } else { \
        switch( _eval_env.stat ) { \
        case(1): \
            printf("\033[1;33m ⊣ \033[0m %s terminated by exit(%d)\n", #_code, _eval_exit_data.status ); \
            break; \
        case(2): \
            if ( _eval_env.signal == SIGPROF ) { \
                printf("\033[1;33m ⊣ \033[0m %s timeout after %g second(s)\n", #_code, _eval_env.timeout);\
            } else { \
                printf("\033[1;33m ⊣ \033[0m %s terminated by signal %d\n", #_code, _eval_env.signal);\
            } \
            break; \
        default: \
            eval_error("%s abnormal termination", #_code); \
        } \
    } \
    _eval_disarm_signals(); \
}

/**
 * EVAL_CATCH_IO( code, stdin, stdout ) macro
 * Same as EVAL_CATCH but redirects input/output from/to specified files
 **/

#define EVAL_CATCH_IO( _code, _stdin, _stdout ) { \
    _eval_exit_data.status = -1; \
    _eval_env.signal = -1; \
    _eval_arm_signals(); \
    printf("\033[1;33m ⊢ \033[0m %s running...\n", #_code ); \
    _eval_io_redirect( _stdin, _stdout ); \
    _eval_env.stat = sigsetjmp( _eval_env.jmp, 1 ); \
    if ( !_eval_env.stat ) { \
        {_code;} \
        _eval_io_restore();\
        printf("\033[1;33m ⊣ \033[0m %s completed normally\n", #_code); \
    } else { \
        _eval_io_restore();\
        switch( _eval_env.stat ) { \
        case(1): \
            printf("\033[1;33m ⊣ \033[0m %s terminated by exit(%d)\n", #_code, _eval_exit_data.status ); \
            break; \
        case(2): \
            if ( _eval_env.signal == SIGPROF ) { \
                printf("\033[1;33m ⊣ \033[0m %s timeout after %g second(s)\n", #_code, _eval_env.timeout);\
            } else { \
                printf("\033[1;33m ⊣ \033[0m %s terminated by signal %d\n", #_code, _eval_env.signal);\
            } \
            break; \
        default: \
            eval_error("%s abnormal termination", #_code); \
        } \
    } \
    _eval_disarm_signals(); \
}

#else

/**
 * EVAL_CATCH( code ) macro
 * Runs the specified code catching:
 * - exit() calls
 * - SIGSEGV, SIGBUS, SIGFPE and SIGILL signals
 **/

#define EVAL_CATCH( _code ) { \
    _eval_exit_data.status = -1; \
    _eval_env.signal = -1; \
    _eval_arm_signals(); \
    _eval_env.stat = sigsetjmp( _eval_env.jmp, 1 ); \
    if ( !_eval_env.stat ) { \
        {_code;} \
    } else { \
        switch( _eval_env.stat ) { \
        case(1): \
            break; \
        case(2): \
            break; \
        default: \
            eval_error("%s abnormal termination", #_code); \
        } \
    } \
    _eval_disarm_signals(); \
}

/**
 * EVAL_CATCH_IO( code, stdin, stdout ) macro
 * Same as EVAL_CATCH but redirects input/output from/to specified files
 **/


#define EVAL_CATCH_IO( _code, _stdin, _stdout ) { \
    _eval_exit_data.status = -1; \
    _eval_env.signal = -1; \
    _eval_arm_signals(); \
    _eval_io_redirect( _stdin, _stdout ); \
    _eval_env.stat = sigsetjmp( _eval_env.jmp, 1 ); \
    if ( !_eval_env.stat ) { \
        {_code;} \
        _eval_io_restore();\
    } else { \
        _eval_io_restore();\
        switch( _eval_env.stat ) { \
        case(1): \
            break; \
        case(2): \
            break; \
        default: \
            eval_error("%s abnormal termination", #_code); \
        } \
    } \
    _eval_disarm_signals(); \
}

#endif

/******************************************************************************
 * exit
 *****************************************************************************/
typedef struct {
    int action;
    int status; // Actual exit status, not number of times called
} _eval_exit_type;

extern _eval_exit_type _eval_exit_data;

void _eval_exit(int);

#define exit( status ) _eval_exit( status );

/******************************************************************************
 * sleep
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    int ret;

    unsigned int seconds;
} _eval_sleep_type;

extern _eval_sleep_type _eval_sleep_data;

unsigned int _eval_sleep(unsigned int seconds);

#define sleep( seconds ) _eval_sleep( seconds )

/******************************************************************************
 * fork
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    pid_t ret;
} _eval_fork_type;

extern _eval_fork_type _eval_fork_data;

pid_t _eval_fork(void);

#define fork() _eval_fork()

/******************************************************************************
 * wait
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    pid_t ret;

    int *stat_loc;
} _eval_wait_type;

extern _eval_wait_type _eval_wait_data;

pid_t _eval_wait(int *stat_loc);

#define wait( stat_loc ) _eval_wait( stat_loc )

/******************************************************************************
 * waitpid
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    pid_t ret;

    pid_t pid;
    int *stat_loc;
    int options;
} _eval_waitpid_type;

extern _eval_waitpid_type _eval_waitpid_data;

pid_t _eval_waitpid(pid_t pid, int *stat_loc, int options);

#define waitpid( pid, stat_loc, options ) _eval_waitpid( pid, stat_loc, options )

/******************************************************************************
 * kill
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    int ret;

    pid_t pid;
    int sig;
} _eval_kill_type;

extern _eval_kill_type _eval_kill_data;

int _eval_kill(pid_t pid, int sig);

#define kill( pid, sig ) _eval_kill( pid, sig )

/******************************************************************************
 * raise
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    int ret;

    int sig;
} _eval_raise_type;

extern _eval_raise_type _eval_raise_data;

int _eval_raise(int sig);

#define raise( sig ) _eval_raise( sig )

/******************************************************************************
 * signal
 *****************************************************************************/
typedef void (*sighandler_t)(int);

typedef struct {
    int action;
    int status;
    sighandler_t ret;

    int signum;
    sighandler_t handler;
} _eval_signal_type;

extern _eval_signal_type _eval_signal_data;

sighandler_t _eval_signal(int signum, sighandler_t handler);

#define signal( signum, handler ) _eval_signal( signum, handler )

/******************************************************************************
 * sigaction
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    int ret;

    int signum;
    struct sigaction *act;
    struct sigaction *oldact;
} _eval_sigaction_type;

extern _eval_sigaction_type _eval_sigaction_data;

int _eval_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);

#define sigaction( signum, act, oldact ) _eval_sigaction( signum, act, oldact )

/******************************************************************************
 * pause
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    int ret;
} _eval_pause_type;

extern _eval_pause_type _eval_pause_data;

int _eval_pause(void);

#define pause( ) _eval_pause( )

/******************************************************************************
 * msgget
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    int ret;

    int msqid;

    key_t key;
    int msgflg;
} _eval_msgget_type;
extern _eval_msgget_type _eval_msgget_data;

int _eval_msgget(key_t key, int msgflg);

#define msgget(key, msgflg) _eval_msgget(key, msgflg)

/******************************************************************************
 * msgsnd
 *****************************************************************************/

typedef struct {
    int action;
    int status;
    int ret;

    int msqid;
    void *msgp;
    size_t msgsz;
    int msgflg;
} _eval_msgsnd_type;

extern _eval_msgsnd_type _eval_msgsnd_data;

int _eval_msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);

#define msgsnd( msqid, msgp, msgsz, msgflg) _eval_msgsnd( msqid, msgp, msgsz, msgflg)

/******************************************************************************
 * msgrcv
 *****************************************************************************/

typedef struct {
    int action;
    int status;
    ssize_t ret;

    int msqid;
    void *msgp;
    size_t msgsz;
    long msgtyp;
    int msgflg;
} _eval_msgrcv_type;

extern _eval_msgrcv_type _eval_msgrcv_data;

ssize_t _eval_msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);

#define msgrcv( msqid, msgp, msgsz, msgtyp ,msgflg) _eval_msgrcv( msqid, msgp, msgsz, msgtyp, msgflg)

/******************************************************************************
 * msgctl
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    int ret;

    int msqid;
    int cmd;
    struct msqid_ds *buf;
} _eval_msgctl_type;

extern _eval_msgctl_type _eval_msgctl_data;

int _eval_msgctl(int msqid, int cmd, struct msqid_ds *buf);

#define msgctl(msqid, cmd, buf) _eval_msgctl(msqid, cmd, buf)

/******************************************************************************
 * semget
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    int ret;

    int semid; // for injection

    key_t key;
    int nsems;
    int semflg;
} _eval_semget_type;

extern _eval_semget_type _eval_semget_data;

int _eval_semget(key_t key, int nsems, int semflg);

#define semget( key, nsems, semflg) _eval_semget( key, nsems, semflg )

/******************************************************************************
 * semctl
 *****************************************************************************/
#ifdef _SEM_SEMUN_UNDEFINED

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};

#endif

typedef struct {
    int action;
    int status;
    int ret;
    int semid;
    int semnum;
    int cmd;
    union semun arg;
} _eval_semctl_type;

extern _eval_semctl_type _eval_semctl_data;

int _eval_semctl(int semid, int semnum, int cmd, ... );

#define semctl( semid, semnum, ... ) _eval_semctl( semid, semnum, ## __VA_ARGS__ )

/******************************************************************************
 * semop
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    int ret;
    int semid;
    struct sembuf *sops;
    size_t nsops;
} _eval_semop_type;

extern _eval_semop_type _eval_semop_data;

int _eval_semop(int semid, struct sembuf *sops, size_t nsops);

#define semop( semid, sops, nsops ) _eval_semop( semid, sops, nsops )

/******************************************************************************
 * shmget
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    int ret;

    key_t key;
    size_t size;
    int shmflg;

    int shmid;
} _eval_shmget_type;

extern _eval_shmget_type _eval_shmget_data;

int _eval_shmget(key_t key, size_t size, int shmflg);

#define shmget( key, size, shmflg) _eval_shmget( key, size, shmflg )

/******************************************************************************
 * shmat
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    void *ret;

    int shmid;
    void *shmaddr;
    int shmflg;
} _eval_shmat_type;

extern _eval_shmat_type _eval_shmat_data;

void *_eval_shmat( int shmid, const void *shmaddr, int shmflg);

#define shmat( shmid, shmaddr, shmflg ) _eval_shmat( shmid, shmaddr, shmflg )

/******************************************************************************
 * shmdt
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    int ret;
    void *shmaddr;
} _eval_shmdt_type;

extern _eval_shmdt_type _eval_shmdt_data;

int _eval_shmdt(const void *shmaddr);

#define shmdt(shmaddr) _eval_shmdt(shmaddr)

/******************************************************************************
 * shmctl
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    int ret;
    int shmid;
    int cmd;
    struct shmid_ds *buf;
} _eval_shmctl_type;

extern _eval_shmctl_type _eval_shmctl_data;

int _eval_shmctl(int shmid, int cmd, struct shmid_ds *buf);

#define shmctl( shmid, cmd, buf ) _eval_shmctl( shmid, cmd, buf )


/******************************************************************************
 * mkfifo
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    char path[PATH_MAX];
    mode_t mode;
    int ret;
} _eval_mkfifo_type;

extern _eval_mkfifo_type _eval_mkfifo_data;

int _eval_mkfifo(const char *path, mode_t mode);

#define mkfifo( path, mode ) _eval_mkfifo( path, mode )

/******************************************************************************
 * S_ISFIFO
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    mode_t mode;
    int ret;
} _eval_isfifo_type;

extern _eval_isfifo_type _eval_isfifo_data;

int _eval_isfifo(mode_t mode);

#undef S_ISFIFO
#define S_ISFIFO(st_mode) _eval_isfifo(st_mode)

/******************************************************************************
 * alarm
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    unsigned int seconds;
    unsigned int ret;
} _eval_alarm_type;

extern _eval_alarm_type _eval_alarm_data;

unsigned int _eval_alarm( unsigned int seconds );

#define alarm( seconds ) _eval_alarm( seconds )


/******************************************************************************
 * remove
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    char path[PATH_MAX];
    int ret;
} _eval_remove_type;

extern _eval_remove_type _eval_remove_data;

int _eval_remove(const char * path);

#define remove( path ) _eval_remove( path )

/******************************************************************************
 * unlink
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    char path[PATH_MAX];
    int ret;
} _eval_unlink_type;

extern _eval_unlink_type _eval_unlink_data;

int _eval_unlink(const char * path);

#define unlink( path ) _eval_unlink( path )

/******************************************************************************
 * atoi
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    char nptr[33];
    int ret;
} _eval_atoi_type;

extern _eval_atoi_type _eval_atoi_data;

int _eval_atoi(const char *nptr);

#define atoi( nptr ) _eval_atoi( nptr )

/******************************************************************************
 * fclose
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    FILE* stream;
    int ret;
} _eval_fclose_type;

extern _eval_fclose_type _eval_fclose_data;

int _eval_fclose(FILE* stream);

#define fclose( stream ) _eval_fclose( stream )

/******************************************************************************
 * execl
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    int ret;
} _eval_execl_type;

extern _eval_execl_type _eval_execl_data;

// The declaration must be different from execl() so that we can then call
// execv() instead. See the implementation for details.
int _eval_execl(const char *path, ... );

#define execl( path, arg0, ... ) _eval_execl( path, arg0, __VA_ARGS__ )

/******************************************************************************
 * fread
 *****************************************************************************/
typedef struct {
    int action;
    int status;
    void *ptr;
    size_t size;
    size_t nmemb;
    FILE *stream;
    int ret;
} _eval_fread_type;

extern _eval_fread_type _eval_fread_data;

size_t _eval_fread(void *restrict ptr, size_t size, size_t nmemb,
                    FILE *restrict stream);

#define fread( ptr, size, nmemb, stream ) _eval_fread( ptr, size, nmemb, stream )


/******************************************************************************
 * fwrite
 *****************************************************************************/

typedef struct {
    int action;
    int status;
    void *ptr;
    size_t size;
    size_t nmemb;
    FILE *stream;
    int ret;
} _eval_fwrite_type;

extern _eval_fwrite_type _eval_fwrite_data;

size_t _eval_fwrite(const void *ptr, size_t size, size_t nmemb,
                     FILE *stream);

#define fwrite( ptr, size, nmemb, stream ) _eval_fwrite( ptr, size, nmemb, stream )

/******************************************************************************
 * fseek
 *****************************************************************************/

typedef struct {
    int action;
    int status;
    FILE *stream;
    long offset;
    int whence;
    int ret;
} _eval_fseek_type;

extern _eval_fseek_type _eval_fseek_data;

int _eval_fseek(FILE *stream, long offset, int whence);

#define fseek( stream, offset, whence ) _eval_fseek( stream, offset, whence )

/******************************************************************************
 * Undefine wrapper macros
 *
 * This allows code to call the base functions and not the wrappers. The
 * EVAL_NOWRAP macro must not be defined in the code being tested.
 *****************************************************************************/

#ifdef EVAL_NOWRAP

#undef exit
#undef sleep

#undef signal
#undef kill
#undef raise
#undef fork
#undef signal
#undef sigaction
#undef pause
#undef alarm

// IPC Message queues
#undef msgget
#undef msgsnd
#undef msgrcv
#undef msgctl

// IPC semaphores
#undef semget
#undef semctl
#undef semop

// IPC shared memory
#undef shmget
#undef shmat
#undef shmdt
#undef shmctl

#undef mkfifo
#undef S_ISFIFO
#define S_ISFIFO(m)     (((m) & S_IFMT) == S_IFIFO)

#undef remove
#undef unlink

#undef atoi
#undef fclose
#undef execl
#undef fread
#undef fwrite
#undef fseek

#endif // EVAL_NOWRAP

#endif // __EVAL_H__
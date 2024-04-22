#define _CLIENTE 1

#include <fcntl.h>
#include <sys/stat.h>

/**
 * Undefine the replacement macros defined in eval.h so we may call the base
 * functions
 */
#define EVAL_NOWRAP 1
#include "eval.h"

/**
 * Base project definitions
 */
#include "common.h"

/**
 * _student_* function prototypes
 */
#include "proto-eval.h"

// Question list
question_t questions[] = {
    {"1.1",   "File exists and is FIFO", 0.0 },
    {"1.2",   "File exists and is not FIFO", 0.0 },
    {"1.3",   "File does not exist", 0.0 },

    {"2",     "Arma sinais", 0.0 },

    {"3.1",     "Pede dados ao utilizador (ok)", 0.0 },
    {"3.2",     "Pede dados ao utilizador (bad NIF)", 0.0 },

    {"4.1",     "Cria FIFO", 0.0 },
    {"4.2",     "Cria FIFO (erro)", 0.0 },

    {"5.1",     "Escreve pedido", 0.0 },
    {"5.2",     "Escreve pedido (erro)", 0.0 },

    {"6",     "Configura alarme", 0.0 },

    {"7",       "Fica à espera que haja um evento", 0.0 },

    {"8",       "Trata SIGUSR1", 0.0 },
    {"9",       "Trata SIGHUP", 0.0 },
    {"10",       "Trata SIGINT", 0.0 },
    {"11",       "Trata SIGALARM", 0.0 },


    {"---", "_end_",0.0}
};

void checkExistsFifoServidor_C1 (char * nameFifo) {
    _student_checkExistsFifoServidor_C1(nameFifo);
}

/**
 * Evaluate C1
 **/
int eval_c1( ) {

    eval_reset();
    eval_info("Evaluating C1.1 - %s...", question_text(questions,"1.1"));

    char nameFifo[] = "eval.fifo";
    mode_t mode = 0666;

    // C1.1 File exists and is FIFO
    mkfifo( nameFifo, mode );

    EVAL_CATCH( checkExistsFifoServidor_C1(nameFifo) );
    if ( 0 != _eval_env.stat ) {
        eval_error( "(C1) bad termination");
        printf(" _eval_env.stat = %d\n", _eval_env.stat);
    }

    eval_check_successlog( "C1" );
    eval_close_logs( "(C1)" );

    unlink( nameFifo );

    eval_info("Evaluating C1.2 - %s...", question_text(questions,"1.2"));

    // C1.2 File exists and is not FIFO
    FILE *f = fopen( nameFifo, "w" );
    fclose(f);

    EVAL_CATCH( checkExistsFifoServidor_C1(nameFifo) );
    if ( 1 != _eval_env.stat ) {
        eval_error("C1 - checkExistsFifoServidor_C1 did not stop with non FIFO file");
    }

    eval_check_errorlog( "C1" );
    eval_close_logs( "(C1)" );

    unlink( nameFifo );

    eval_info("Evaluating C1.3 - %s...", question_text(questions,"1.3"));

    // C1.3 File does not exist
    EVAL_CATCH( checkExistsFifoServidor_C1(nameFifo) );
    if ( 1 != _eval_env.stat ) {
        eval_error("C1 - checkExistsFifoServidor_C1 did not stop with missing file");
    }
    eval_check_errorlog( "C1" );
    eval_close_logs( "(C1)" );

    return eval_complete("(C1)");
}

/**
 * Wrapper for triggerSignals_C2()
 **/

void triggerSignals_C2() {
    _student_triggerSignals_C2();
}

int eval_c2( ) {

    eval_reset();
    eval_info("Evaluating C2 - %s...", question_text(questions,"2"));

    initlog(&_data_log);
    _eval_signal_data.action = 2;

    EVAL_CATCH( triggerSignals_C2() );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(C2) bad termination");
    }

    if ( 0 > findinlog( &_data_log, "signal,%d,%p", SIGUSR1, trataSinalSIGUSR1_C8 )) {
        eval_error("(C2) SIGUSR1 not armed properly");
    }

    if ( 0 > findinlog( &_data_log, "signal,%d,%p", SIGHUP, trataSinalSIGHUP_C9 )) {
        eval_error("(C2) SIGHUP not armed properly");
    }

    if ( 0 > findinlog( &_data_log, "signal,%d,%p", SIGINT, trataSinalSIGINT_C10 )) {
        eval_error("(C2) SIGINT not armed properly");
    }

    if ( 0 > findinlog( &_data_log, "signal,%d,%p", SIGALRM, trataSinalSIGALRM_C11 )) {
        eval_error("(C2) SIGALRM not armed properly");
    }

    eval_close_logs( "(C2)" );
    return eval_complete("(C2)");
}



/**
 * Wrapper for getDadosPedidoUtilizador_C3_C4()
 **/
struct {
    int action;
    CheckIn request;
} _getDadosPedidoUtilizador_data;

CheckIn  getDadosPedidoUtilizador_C3_C4() {
    _getDadosPedidoUtilizador_data.request = (CheckIn) {.nif = -1};
    _getDadosPedidoUtilizador_data.request = _student_getDadosPedidoUtilizador_C3_C4();
    return _getDadosPedidoUtilizador_data.request;
}

int eval_c3_c4( ) {

    eval_reset();
    eval_info("Evaluating C3_C4 - %s...", question_text(questions,"3.1"));

    int nif = 12345678;
    const char senha[] = "abc123";

    FILE* ft;
    ft = fopen(FILE_STDIN,"w");
    fprintf(ft,"%d\n", nif );
    fprintf(ft,"%s\n", senha );
    for(int i = 0; i < 5; i++ ) fprintf(ft,"\n" );
    fclose(ft);

    unlink( FILE_STDOUT );

    // Redirect standard I/O to files
    EVAL_CATCH_IO( getDadosPedidoUtilizador_C3_C4(), FILE_STDIN, FILE_STDOUT );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(C3) bad termination");
    }

    // Remove I/O files
    unlink( FILE_STDIN );
    unlink( FILE_STDOUT );

    if ( nif != _getDadosPedidoUtilizador_data.request.nif ) {
        eval_error( "(C3) invalid request.nif" );
    }

    if ( strncmp( senha, _getDadosPedidoUtilizador_data.request.senha, strlen(senha) ) ) {
        eval_error( "(C3) invalid request.senha");
    }

    if ( getpid() != _getDadosPedidoUtilizador_data.request.pidCliente ) {
        eval_error( "(C3) invalid request.pidCliente");
    }

    if ( eval_check_successlog( "%d %s %d", nif, senha, getpid() )) {}
    eval_close_logs( "(C3)" );

    eval_info("Evaluating C3.2 - %s...", question_text(questions,"3.2"));

    nif = 1234516780;

    ft = fopen(FILE_STDIN,"w");
    fprintf(ft,"%d\n", nif );
    fprintf(ft,"%s\n", senha );
    for(int i = 0; i < 5; i++ ) fprintf(ft,"\n" );
    fclose(ft);

    unlink( FILE_STDOUT );

    // Redirect standard I/O to files
    EVAL_CATCH_IO( getDadosPedidoUtilizador_C3_C4(), FILE_STDIN, FILE_STDOUT );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(C3) Accepted invalid NIF");
    }

    // Remove I/O files
    unlink( FILE_STDIN );
    unlink( FILE_STDOUT );

    eval_check_errorlog( "C3" );

    eval_close_logs( "(C3)" );
    return eval_complete("(C3)");
}


/**
 * Wrapper for writeRequest_C5()
 **/

void writeRequest_C5 (CheckIn request, char *nameFifo) {
    _student_writeRequest_C5( request, nameFifo );
}

int eval_c5( ) {

    eval_reset();
    eval_info("Evaluating C5.1 - %s...", question_text(questions,"5.1"));

    CheckIn request = {
        .nif = 12345678,
        .senha = "abc123xyz",
        .nome = "José Silva",
        .nrVoo = "TP1234",
        .pidCliente = 16384,
        .pidServidorDedicado = 32768
    };

    char nameFifo[] = "eval.fifo";
    FILE *f = fopen( nameFifo, "w" );
    fclose(f);

    EVAL_CATCH( writeRequest_C5( request, nameFifo ) );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(C5) bad termination");
    }

    f = fopen( nameFifo, "r" );
    if ( f == NULL ) {
        eval_error("(C5) Unable to open output file for testing");
    } else {
        int nif;
        char senha[40];
        int  pidCliente;

        if ( fscanf( f, "%d", &nif ) < 1 ) {
            eval_error("(C5) Error reading NIF");
        }

        if ( fscanf( f, "%40s", senha ) < 1 ) {
            eval_error("(C5) Error reading senha");
        }

        if ( fscanf( f, "%d", &pidCliente ) < 1 ) {
            eval_error("(C5) Error reading pidCliente");
        }

        if ( request.nif != nif ||
            request.pidCliente != pidCliente ||
            strncmp( request.senha, senha, 40 ) ) {
            eval_error("(C5) Data in file does not match supplied data");
        }

        fclose(f);
    }

    // Remove I/O files
    unlink( nameFifo );

    eval_check_successlog( "C5" );

    eval_info("Evaluating C5.2 - %s...", question_text(questions,"5.2"));

    // Check with locked file
    if ( create_lockfile( nameFifo ) == 0 ) {

        EVAL_CATCH( writeRequest_C5( request, nameFifo ) );

        if ( 1 != _eval_env.stat ) {
            eval_error( "(C5) writeRequest_C5 did not stop on write error");
        }

        eval_check_errorlog( "C5" );

        remove_lockfile( nameFifo );
    } else {
        eval_error("Problem creating lockfile, unable to test C5.2");
    }

    eval_close_logs( "(C5)" );
    return eval_complete("(C5)");
}

void configureTimer_C6 (int tempoEspera) {
    _student_configureTimer_C6( tempoEspera );
}

int eval_c6( ) {

    eval_reset();
    eval_info("Evaluating C6 - %s...", question_text(questions,"6"));

    _eval_alarm_data.status = 0;
    _eval_alarm_data.action = 1;

    int tempoEspera = rand() % 100;

    EVAL_CATCH( configureTimer_C6(tempoEspera) );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(C6) bad termination");
    }

    if ( 1 != _eval_alarm_data.status ) {
        eval_error("(C6) alarm() should have been called exactly once");
    } else {
        if ( _eval_alarm_data.seconds != tempoEspera ) {
            eval_error("(C6) alarm() was not called with the correct parameters");
        }

        if ( eval_check_successlog( "Espera resposta em %d segundos", tempoEspera )) {}
    }

    eval_close_logs( "(C6)" );
    return eval_complete("(C6)");
}

void waitForEvents_C7 () {
    _student_waitForEvents_C7();
}

int eval_c7( ) {

    eval_reset();
    eval_info("Evaluating C7 - %s...", question_text(questions,"7"));

    // Return silently if pause command was called
    _eval_pause_data.action = 1;
    _eval_pause_data.status = 0;

    EVAL_CATCH( waitForEvents_C7() );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(C7) bad termination");
    }

    if ( 1 != _eval_pause_data.status ) {
        eval_error("(C7) Incorrect implementation");
    }

    eval_close_logs( "(C7)" );
    return eval_complete("(C7)");

}

void trataSinalSIGUSR1_C8 (int sinalRecebido) {
    _student_trataSinalSIGUSR1_C8( sinalRecebido );
}

int eval_c8( ) {

    eval_reset();
    eval_info("Evaluating C8 - %s...", question_text(questions,"8"));

    EVAL_CATCH( trataSinalSIGUSR1_C8(SIGUSR1) );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(C8) Client was not shutdown properly");
    } else if ( 0 != _eval_exit_data.status ) {
        eval_error("(C8) Invalid exit status");
    }

    eval_check_successlog( "C8" );

    eval_close_logs( "(C8)" );
    return eval_complete("(C8)");

}


void trataSinalSIGHUP_C9 (int sinalRecebido) {
    _student_trataSinalSIGHUP_C9( sinalRecebido );
}

int eval_c9( ) {

    eval_reset();
    eval_info("Evaluating C9 - %s...", question_text(questions,"9"));

    EVAL_CATCH( trataSinalSIGHUP_C9(SIGHUP) );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(C9) Client was not shutdown properly");
    } else if ( 1 != _eval_exit_data.status ) {
        eval_error("(C9) Invalid exit status");
    }

    eval_check_successlog( "C9" );

    eval_close_logs( "(C9)" );
    return eval_complete("(C9)");

}

void trataSinalSIGINT_C10 (int sinalRecebido) {
    _student_trataSinalSIGINT_C10( sinalRecebido );
}

int eval_c10( ) {

    eval_reset();
    eval_info("Evaluating C10 - %s...", question_text(questions,"10"));

    EVAL_CATCH( trataSinalSIGINT_C10(SIGINT) );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(C10) Client was not shutdown properly");
    } else if ( 0 != _eval_exit_data.status ) {
        eval_error("(C10) Invalid exit status");
    }

    eval_check_successlog( "(C10) Cliente: Shutdown" );

    eval_close_logs( "(C10)" );
    return eval_complete("(C10)");

}

void trataSinalSIGALRM_C11 (int sinalRecebido) {
    _student_trataSinalSIGALRM_C11( sinalRecebido );
}

int eval_c11( ) {

    eval_reset();
    eval_info("Evaluating C11 - %s...", question_text(questions,"11"));

    EVAL_CATCH( trataSinalSIGALRM_C11(SIGUSR1) );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(C11) Client was not shutdown properly");
    }

    eval_check_errorlog( "(C11) Cliente: Timeout" );

    eval_close_logs( "(C11)" );
    return eval_complete("(C11)");
}

/**
 * @brief Print help message
 *
 * @param arg0 Should be set to argv[0]
 */
void eval_help( char * arg0 ) {
    printf("Usage: %s [OPTION]\n", arg0 );
    printf(
        "Evaluate student code.\n"
        "\n"
        "  -h   display this help and exit\n"
        "  -e   stop when an error is found; default is to continue\n"
        "       checking the code\n"
        "  -x   export evaluation (single line grades)\n"
        "  -l   list all questions and grades\n"
        "\n"
    );
}

/**
 * @brief Macro to run the test
 *
 * Will add the number of errors found to the nerr variable.
 * If opts.stop_on_error is set and an error is found the code will
 * stop immediately with exit status 2.
 */
#define RUNTEST( _test ) { \
    int err = (_test); \
    if ( opts.stop_on_error && err > 0 ) { \
        fprintf(stderr,"Error(s) found, stopping evaluation...\n"); \
        exit(2); \
    } else nerr += err; \
}

int main(int argc, char *argv[]) {

    /* Process command line options */

    struct {
        int stop_on_error;
        int export;
        int list;
    } opts = {0}; ///< Command line options

    int c;
    while( (c = getopt( argc, argv, "hexl")) != -1 )
        switch (c) {
            case 'h':
                eval_help( argv[0] );
                exit(0);
                break;
            case 'e':
                opts.stop_on_error = 1;
                break;
            case 'x':
                opts.export = 1;
                break;
            case 'l':
                opts.list = 1;
                break;
            default:
                printf("\n");
                eval_help(  argv[0] );
                exit(1);
        }

    if ( optind < argc ) {
        fprintf(stderr, "%s: no arguments allowed --'%s'\n", argv[0], argv[optind]);
        printf("\n");
        eval_help(  argv[0] );
        exit(1);
    }

    /* Initialize logs */
    initlog( &_success_log );
    initlog( &_error_log );

    /* Run evaluation */
    eval_info(" %s/cliente.c\n", TOSTRING( _EVAL ) );

    int nerr = 0;

    RUNTEST( eval_c1() );
    RUNTEST( eval_c2() );
    RUNTEST( eval_c3_c4() );
    RUNTEST( eval_c5() );
    RUNTEST( eval_c6() );
    RUNTEST( eval_c7() );
    RUNTEST( eval_c8() );
    RUNTEST( eval_c9() );
    RUNTEST( eval_c10() );
    RUNTEST( eval_c11() );

    eval_info("Finished." );

    /* Printout error summary report */
    _eval_stats.error = nerr;
    eval_complete("cliente");

    /* Print out full evaluation report */
    if ( opts.export ) question_export(questions, "cliente" );
    if ( opts.list ) question_list(questions, "cliente" );

}
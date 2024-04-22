#define _SERVIDOR 1

#include <sys/stat.h>
#include <fcntl.h>

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

question_t questions[] = {
    {"1.1",   "Ficheiro nameDB existe", 0.0 },
    {"1.2",   "Ficheiro nameDB não existe", 0.0 },

    {"2.1",   "Cria FIFO", 0.0 },
    {"2.2",   "Cria FIFO (existing file)", 0.0 },
    {"2.3",   "Cria FIFO (erro)", 0.0 },

    {"3",   "Arma sinais", 0.0 },

    {"4.1",   "Lê pedido", 0.0 },
    {"4.2",   "Lê pedido (erro)", 0.0 },

    {"5.1",   "Cria servidor dedicado (parent)", 0.0 },
    {"5.2",   "Cria servidor dedicado (child)", 0.0 },
    {"5.3",   "Cria servidor dedicado (erro)", 0.0 },

    {"6.a",   "Shutdown servidor (ok)", 0.0 },
    {"6.b",   "Shutdown servidor (missing DB)", 0.0 },

    {"7",   "Apaga Fifo e sai", 0.0 },

    {"8",   "Termina servidor dedicado", 0.0 },

    {"9",   "Arma sinais", 0.0 },

    {"10.a",   "Procura utilizador (ok)", 0.0 },
    {"10.b",   "Procura utilizador (bad password)", 0.0 },
    {"10.c",   "Procura utilizador (not found)", 0.0 },

    {"11.a",   "Reserva utilizador (ok)", 0.0 },
    {"11.b",   "Reserva utilizador (unable to open file)", 0.0 },
    {"11.c",   "Reserva utilizador (seek error)", 0.0 },
    {"11.d",   "Reserva utilizador (check full DB)", 0.0 },

    {"12",     "Espera e envia sinal", 0.0 },

    {"13.a",   "Termina servidor dedicado", 0.0 },
    {"13.b",   "Termina servidor dedicado (bad write)", 0.0 },
    {"13.c",   "Termina servidor dedicado (bad write)", 0.0 },

    {"14",     "Trata sinal servidor para cancelar", 0.0 },

    {"---", "_end_",0.0}
};

/* Variáveis globais */
CheckIn clientRequest; // Variável que tem o pedido enviado do Cliente para o Servidor


/**
 * @brief Imprime o conteúdo de uma variável do tipo CheckIn
 *
 * @param s     String a imprimir antes do conteúdo da variável. Se NULL será
 *              ignorada
 * @param r     Variável a imprimir
 */
void print_checkin( char *s, CheckIn r ) {
    if (s) printf("%s\n", s);
    printf("nif                   : %d\n", r.nif);
    printf("senha                 : %s\n", r.senha);
    printf("nome                  : %s\n", r.nome);
    printf("nrVoo                 : %s\n", r.nrVoo);
    printf("pidCliente            : %d\n", r.pidCliente);
    printf("pidServidorDedicado   : %d\n", r.pidServidorDedicado);
}


/**
 * @brief Inicializa a base de dados com um conjunto de valores de teste
 *
 * @param db        Base de dados
 * @param size      Número de elementos
 */
void initTestDB( CheckIn db[], int size ) {
    for( int i = 0; i < size; i++ ) {
        int idx = i+1;
        db[i].nif = 1000000 + idx;
        snprintf( db[i].senha, 40, "pass_%d", idx );
        snprintf( db[i].nome, 60, "Name%d Surname%d", idx, idx );
        snprintf( db[i].nrVoo, 8, "TAP%03d", idx % 1000 );
        db[i].pidCliente = -1;
        db[i].pidServidorDedicado = -1;
    }
}

/**
 * @brief Grava a base de dados num ficheiro binário
 *
 * @param fname     Nome do ficheiro
 * @param db        Base de dados
 * @param nitems    Número de elementos
 */
void saveDB( char *fname, CheckIn *db, int nitems ) {

    FILE *f = fopen( fname, "w" );
    if ( f != NULL ) {
        if ( fwrite( db, sizeof(CheckIn), nitems, f ) < nitems ) {
            fprintf(stderr, "Unable to write file %s, aborting...\n", fname );
            exit(1);
        };
    } else {
        fprintf(stderr, "Unable to create file %s, aborting...\n", fname );
        exit(1);
    }
    fclose( f );
}

/**
 * @brief Cria uma base de dados de teste e grava num ficheiro
 *
 * A base de dados tem 4 elementos
 *
 * @param fname     Nome do ficheiro a gravar
 */
void createTestDB( char *fname ) {

    CheckIn testDB[10];

    initTestDB( testDB, 4 );
    testDB[3].pidServidorDedicado = 2003;
    saveDB( fname, testDB, 4 );
}

/**
 * @brief Compares 2 database files
 *
 * Comparisons are done field by field because of string comparisons
 *
 * @param fname1    File 1
 * @param fname2    File 2
 * @return int      0 if files match, -1 on error or mismatch
 */
int compareDBfiles( char *fname1, char *fname2 ) {

    struct stat st1, st2;
    if ( stat( fname1, &st1 ) < 0 ) {
        fprintf(stderr, "Unable to stat file %s\n", fname1 );
        return -1;
    }

    if ( stat( fname2, &st2 ) < 0 ) {
        fprintf(stderr, "Unable to stat file %s\n", fname2 );
        return -1;
    }

    if ( st1.st_size != st2.st_size ) {
        fprintf(stderr, "Files have different sizes %s(%ld) : %s(%ld)\n",
            fname1, st1.st_size, fname2, st2.st_size );
        return -1;
    }

    FILE *f1 = fopen( fname1, "r" );
    if ( f1 == NULL ) {
        fprintf(stderr, "Unable to open file %s for reading\n", fname1 );
    }

    FILE *f2 = fopen( fname2, "r" );
    if ( f2 == NULL ) {
        fprintf(stderr, "Unable to open file %s for reading\n", fname2 );
    }

    int ret = 0;

    if ( f1 && f2 ) {
        CheckIn l1, l2;

        while( ( fread( &l1, sizeof(CheckIn), 1, f1 ) > 0 ) &&
               ( fread( &l2, sizeof(CheckIn), 1, f2 ) > 0 ) ) {

            if ( l1.nif != l2.nif ||
                strcmp( l1.senha, l2.senha ) ||
                strcmp( l1.nome, l2. nome ) ||
                strcmp( l1.nrVoo, l2. nrVoo ) ||
                l1.pidCliente != l2.pidCliente ||
                l1.pidServidorDedicado != l2.pidServidorDedicado ) {

                fprintf(stderr, "Files %s and %s don't match\n", fname1, fname2 );

                print_checkin( fname1, l1 );
                print_checkin( fname2, l2 );

                ret = -1; break;
            }
        }
    } else {
        ret = -1;
    }

    fclose( f1 );
    fclose( f2 );
    return ret;
}

/* ******************************************************************

   ****************************************************************** */

void checkExistsDB_S1 (char *nameDB) {
    _student_checkExistsDB_S1( nameDB );
}

/**
 * Evaluate S1
 **/
int eval_s1( ) {

    eval_reset();
    eval_info("Evaluating S1.1 - %s...", question_text(questions,"1.1"));

    char nameDB[] = "fake.db";
    FILE *f = fopen( nameDB, "w" );
    fclose(f);

    EVAL_CATCH( checkExistsDB_S1( nameDB ) );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(S1) Servidor fechado com ficheiro existente");
    }

    eval_check_successlog( "S1" );

    unlink( nameDB );


    eval_info("Evaluating S1.2 - %s...", question_text(questions,"1.2"));

    EVAL_CATCH( checkExistsDB_S1( nameDB ) );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(S1) Servidor não terminou com ficheiro inexistente");
    }

    eval_check_errorlog( "S1" );

    eval_close_logs( "(S1)" );
    return eval_complete("(S1)");
}



void createFifo_S2 (char *nameFifo) {
    _student_createFifo_S2( nameFifo );
}

/**
 * Evaluate S2
 **/
int eval_s2( ) {

    eval_reset();
    eval_info("Evaluating S2.1 - %s...", question_text(questions,"2.1"));

    char nameFifo[] = "eval.fifo";

    _eval_mkfifo_data.action = 1;
    _eval_mkfifo_data.status = 0;

    EVAL_CATCH( createFifo_S2( nameFifo ) );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(S2) bad termination");
    }

    if ( 1 == _eval_mkfifo_data.status ) {
        if ( strncmp( nameFifo, _eval_mkfifo_data.path, strlen(nameFifo) ) ) {
            eval_error( "(S2) FIFO created with invalid name");
        }
        if ( ! ( 0600 & _eval_mkfifo_data.mode ) ) {
            eval_error( "(S2) FIFO created with invalid mode, must be r/w by owner");
        }

        if ( eval_check_successlog( "S2" )) {}

    } else {
        eval_error("(S2) mkfifo() should have been called exactly once");
    }

    eval_close_logs( "(S2)" );

    eval_info("Evaluating S2.2 - %s...", question_text(questions,"2.2"));

    initlog(&_data_log);

    _eval_mkfifo_data.action = 1;
    _eval_mkfifo_data.status = 0;

    _eval_remove_data.action = 2;
    _eval_remove_data.status = 0;

    _eval_unlink_data.action = 2;
    _eval_unlink_data.status = 0;

    // Check with existing file
    FILE *f = fopen( nameFifo, "w" );
    fwrite("SO2023", 6, 1 ,f );
    fclose( f );

    EVAL_CATCH( createFifo_S2( nameFifo ) );

    unlink( nameFifo );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(S2) bad termination");
    }

    if ( 1 != _eval_unlink_data.status + _eval_remove_data.status ) {
        eval_error("(S2) unlink/remove should have been called exactly once");
    }

    if ( 0 != findinlog( &_data_log, "unlink,%s", nameFifo ) ) {
        eval_error("(S2) unlink/remove called with bad parameters");
    }

    if ( 1 == _eval_mkfifo_data.status ) {
        if ( strncmp( nameFifo, _eval_mkfifo_data.path, strlen(nameFifo) ) ) {
            eval_error( "(S2) FIFO created with invalid name");
        }
        if ( ! ( 0600 & _eval_mkfifo_data.mode ) ) {
            eval_error( "(S2) FIFO created with invalid mode, must be r/w by owner");
        }

        if ( eval_check_successlog( "S2" )) {}

    } else {
        eval_error("(S2) mkfifo() should have been called exactly once");
    }

    eval_close_logs( "(S2)" );

    eval_info("Evaluating S2.3 - %s...", question_text(questions,"2.3"));

    _eval_mkfifo_data.action = 2;
    _eval_mkfifo_data.status = 0;

    EVAL_CATCH( createFifo_S2( nameFifo ) );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(S2) bad termination");
    }

    if ( 1 != _eval_mkfifo_data.status ) {
         eval_error("(S2) mkfifo() should have been called exactly once");
    }

    eval_check_errorlog("S2");

    eval_close_logs( "(S2)" );
    return eval_complete("(S2)");
}



void triggerSignals_S3 () {
    _student_triggerSignals_S3();
}

/**
 * Evaluate S3
 **/
int eval_s3( ) {

    eval_reset();
    eval_info("Evaluating S3 - %s...", question_text(questions,"3"));

    initlog(&_data_log);
    _eval_signal_data.action = 2;

    EVAL_CATCH( triggerSignals_S3( ) );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(S3) bad termination");
    }

    if ( 0 > findinlog( &_data_log, "signal,%d,%p", SIGINT, trataSinalSIGINT_S6 ) ) {
        eval_error("(S3) SIGINT not armed properly");
    }

    if ( 0 > findinlog( &_data_log, "signal,%d,%p", SIGCHLD, trataSinalSIGCHLD_S8 )) {
        eval_error("(S3) SIGCHLD not armed properly");
    }

    eval_close_logs( "(S3)" );
    return eval_complete("(S3)");
}

struct {
    CheckIn r;
} _readRequest_S4_data;

CheckIn readRequest_S4 (char *nameFifo) {
    _readRequest_S4_data.r = (CheckIn) {.nif = -1234};
    _readRequest_S4_data.r = _student_readRequest_S4( nameFifo );
    return _readRequest_S4_data.r;
}

/**
 * Evaluate S4
 **/
int eval_s4( ) {

    eval_reset();
    eval_info("Evaluating S4.1 - %s...", question_text(questions,"4.1"));

    CheckIn r = {
        .nif = 12345678,
        .senha = "abc123xyz",
        .nome = "José Silva",
        .nrVoo = "TP123",
        .pidCliente = 16384,
        .pidServidorDedicado = 32768
    };

    char nameFifo[] = "eval.fifo";
    FILE *f = fopen( nameFifo, "w" );
    fprintf(f,"%d\n%s\n%d\n", r.nif, r.senha, r.pidCliente);
    fclose(f);

    EVAL_CATCH( readRequest_S4( nameFifo ) );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(S4) bad termination");
    }

    if ( _readRequest_S4_data.r.nif < 0 ) {
        eval_error("(S4) Bad return value");
    }

    if ( _readRequest_S4_data.r.nif != r.nif ||
        strcmp(_readRequest_S4_data.r.senha, r.senha) ||
        _readRequest_S4_data.r.pidCliente != r.pidCliente ) {
        eval_error("(S4) Data read from file does not match supplied data");
    }

    eval_check_successlog( "%d %s %d", r.nif, r.senha, r.pidCliente );

    unlink( nameFifo );

    eval_info("Evaluating S4.2 - %s...", question_text(questions,"4.2"));

    EVAL_CATCH( readRequest_S4(nameFifo) );

    if ( 0 == _eval_env.stat ) {
        eval_error( "(S4) should have terminated");
    }

    eval_check_errorlog( "S4" );

    eval_close_logs( "(S4)" );
    return eval_complete("(S4)");
}



struct {
    int pid_filho;
} _createServidorDedicado_S5_data;

int createServidorDedicado_S5() {
    _createServidorDedicado_S5_data.pid_filho = -1234;
    _createServidorDedicado_S5_data.pid_filho = _student_createServidorDedicado_S5();
    return _createServidorDedicado_S5_data.pid_filho;
}

/**
 * Evaluate S5
 **/
int eval_s5( ) {

    eval_reset();
    eval_info("Evaluating 5.1 - %s...", question_text(questions,"5.1"));

    // Test parent (dummy fork() will return 1)
    _eval_fork_data.action = 2;
    EVAL_CATCH( createServidorDedicado_S5( ) );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(S5) bad termination");
    }

    if ( 1 != _createServidorDedicado_S5_data.pid_filho ) {
        eval_error("(S5) Bad return value");
    }

    eval_check_successlog( "Servidor: Iniciei SD %d", 1 );

    eval_info("Evaluating 5.2 - %s...", question_text(questions,"5.2"));

    // Test child (dummy fork() will return 0)
    _eval_fork_data.action = 1;
    EVAL_CATCH( createServidorDedicado_S5( ) );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(S5) bad termination");
    }

    if ( 0 != _createServidorDedicado_S5_data.pid_filho ) {
        eval_error("(S5) Bad return value");
    }

    eval_info("Evaluating 5.3 - %s...", question_text(questions,"5.3"));

    // Test error (dummy fork() will return -1)
    _eval_fork_data.action = 3;
    EVAL_CATCH( createServidorDedicado_S5( ) );

    if ( 0 == _eval_env.stat ) {
        eval_error( "(S5) should have terminated");
    }

    eval_check_errorlog( "S5" );

    eval_close_logs( "(S5)" );
    return eval_complete("(S5)");
}

/**
 * These need to be defined before trataSinalSIGINT_S6
 * because the routine may call them
 */


struct {
    int status;
    int action;
} _deleteFifoAndExit_S7_data;

void deleteFifoAndExit_S7() {
    _deleteFifoAndExit_S7_data.status++;
    if ( _deleteFifoAndExit_S7_data.action == 0 )
        _student_deleteFifoAndExit_S7();
}



void trataSinalSIGINT_S6(int sinalRecebido) {
    _student_trataSinalSIGINT_S6( sinalRecebido );
}

/**
 * Evaluate S6
 **/
int eval_s6( ) {

    eval_reset();
    eval_info("Evaluating S6.a - %s...", question_text(questions,"6.a"));

    _eval_unlink_data.status = 0;
    _eval_unlink_data.action = 1;

    _deleteFifoAndExit_S7_data.status = 0;
    _deleteFifoAndExit_S7_data.action = 0;

    _eval_kill_data.status = 0;
    _eval_kill_data.action = 2;

    // Only 1 request is active
    createTestDB( FILE_DATABASE );

    EVAL_CATCH( trataSinalSIGINT_S6( SIGINT ) );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(S6) bad termination");
    }

    if ( 1 != _deleteFifoAndExit_S7_data.status ) {
        eval_error("(S6) deleteFifoAndExit_S7() should have been called");
    }

    if ( 1 != _eval_kill_data.status ) {
        eval_error("(S6) kill() should have been called once for this test data");
    } else {
        if ( 2003 != _eval_kill_data.pid ) {
            eval_error("(S6) kill() called with wrong PID");
        }
        if ( SIGUSR2 != _eval_kill_data.sig ) {
            eval_error("(S6) kill() called with wrong signal number");
        }
    }
    eval_check_successlog( "(S6) Servidor: Start Shutdown" );

    // Database opened ok
    eval_check_successlog( "S6.1" );

    // Shutdown SD
    eval_check_successlog( "(S6.3) Servidor: Shutdown SD %d", 2003 );

    // Completed processing
    eval_check_successlog( "S6.2" );

    // Messages from S7 are ignored
    eval_clear_logs();

    unlink( FILE_DATABASE );

    eval_info("Evaluating S6.b - %s...", question_text(questions,"6.b"));

    _eval_kill_data.status = 0;
    _eval_kill_data.action = 2;

    EVAL_CATCH( trataSinalSIGINT_S6( SIGINT ) );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(S6) bad termination");
    }

    eval_check_successlog( "(S6) Servidor: Start Shutdown" );

    // Unable to open file
    eval_check_errorlog( "S6.1" );

    if ( 0 != _eval_kill_data.status ) {
        eval_error("(S6) kill() should not have been called in this case");
    }

    // Messages from S7 are ignored
    eval_clear_logs();

    eval_close_logs( "(S6)" );
    return eval_complete("(S6)");
}


/**
 * Evaluate S7
 **/
int eval_s7( ) {

    eval_reset();
    eval_info("Evaluating S7 - %s...", question_text(questions,"7"));

    _eval_unlink_data.status = 0;
    _eval_unlink_data.action = 1;

    _deleteFifoAndExit_S7_data.action = 0;

    EVAL_CATCH( deleteFifoAndExit_S7() );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(S7) bad termination, should have ended with exit call");
    } else {
        if ( 0 != _eval_exit_data.status ) {
            eval_error( "(S7) exit() called with wrong value");
        }
    }

    if ( 1 != _eval_unlink_data.status ) {
        eval_error("(S7) unlink() should have been called exactly once");
    } else {
        if ( strncmp( _eval_unlink_data.path, FILE_REQUESTS, 1024 ) ) {
            eval_error("(S6) unlink() called with invalid path");
        };
    }

    eval_check_successlog("(S7) Servidor: End Shutdown" );

    eval_close_logs( "(S7)" );
    return eval_complete("(S7)");
}

void trataSinalSIGCHLD_S8 (int sinalRecebido) {
    _student_trataSinalSIGCHLD_S8( sinalRecebido );
}

/**
 * Evaluate S8
 **/
int eval_s8( ) {

    eval_reset();
    eval_info("Evaluating S8 - %s...", question_text(questions,"8"));

    int pid_servidor_dedicado = 16384;

    _eval_wait_data.status = 0;
    _eval_wait_data.action = 2;
    _eval_wait_data.ret = pid_servidor_dedicado;

    EVAL_CATCH( trataSinalSIGCHLD_S8( SIGCHLD ) );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(S8) bad termination");
    }

    if ( 1 != _eval_wait_data.status ) {
        eval_error("(S1) wait() should have been called exactly once");
    }

    eval_check_successlog( "Servidor: Confirmo fim de SD %d", pid_servidor_dedicado );

    eval_close_logs( "(S8)" );
    return eval_complete("(S8)");
}

void triggerSignals_SD9() {
    _student_triggerSignals_SD9();
}

/**
 * Evaluate SD9
 **/
int eval_sd9( ) {

    eval_reset();
    eval_info("Evaluating SD9 - %s...", question_text(questions,"9"));

    initlog(&_data_log);
    _eval_signal_data.action = 2;

    EVAL_CATCH( triggerSignals_SD9( ) );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(SD9) bad termination");
    }

    if ( 0 > findinlog( &_data_log, "signal,%d,%p", SIGUSR2, trataSinalSIGUSR2_SD14 )) {
        eval_error("(SD9) SIGUSR2 not armed properly");
    }

    if ( 0 > findinlog( &_data_log, "signal,%d,%p", SIGINT, SIG_IGN )) {
        eval_error("(SD9) SIGINT not armed properly");
    }

    eval_close_logs( "(SD9)" );
    return eval_complete("(SD9)");
}


struct {
    int index_client;
} _searchClientDB_SD10_data;

int searchClientDB_SD10(CheckIn request, char *nameDB, CheckIn *itemDB) {
    _searchClientDB_SD10_data.index_client = -1234;
    _searchClientDB_SD10_data.index_client = _student_searchClientDB_SD10( request, nameDB, itemDB );
    return _searchClientDB_SD10_data.index_client;
};

/**
 * Evaluate SD10
 **/
int eval_sd10( ) {

    eval_reset();
    eval_info("Evaluating SD10.a - %s...", question_text(questions,"10.a"));

    createTestDB( FILE_DATABASE );

    CheckIn request = {.nif = 1000004, .senha = "pass_4", .pidCliente = 1000 };
    char nameDB[] = FILE_DATABASE;
    CheckIn itemDB;

    // don't send signal, just capture data
    _eval_kill_data.status = 0;
    _eval_kill_data.action = 2;

    EVAL_CATCH( searchClientDB_SD10( request, nameDB, &itemDB ) );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(SD10) bad termination");
    }

    // User is in position 3
    if ( 3 != _searchClientDB_SD10_data.index_client ) {
        eval_error("(SD10) Bad return value");
    }

    eval_check_successlog( "(SD10.3) %d", 3 );

    if ( 0 != _eval_kill_data.status ) {
        eval_error( "(SD10) No signal should be sent with valid user");
    }

    // eval_check_successlog( "SD10" );

    eval_info("Evaluating SD10.b - %s...", question_text(questions,"10.b"));

    // Test with bad password
    strcpy( request.senha, "wrong!" );

    // don't send signal, just capture data
    _eval_kill_data.status = 0;
    _eval_kill_data.action = 2;

    EVAL_CATCH( searchClientDB_SD10( request, nameDB, &itemDB ) );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(SD10) Servidor dedicado não terminou com password errada");
    }

    eval_check_errorlog( "SD10.3" );

    if ( 1 != _eval_kill_data.status ) {
        eval_error( "(SD10) exactly 1 signal should be sent");
    } else if ( _eval_kill_data.pid != request.pidCliente || _eval_kill_data.sig != SIGHUP ) {
        eval_error( "(SD10) signal sent with invalid value and/or target");
    }

    eval_info("Evaluating SD10.c - %s...", question_text(questions,"10.c"));

    _eval_kill_data.status = 0;
    _eval_kill_data.action = 2; // don't send signal, just capture data

    request.nif = 2000000;

    EVAL_CATCH( searchClientDB_SD10( request, nameDB, &itemDB ) );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(SD10) Servidor dedicado não terminou com utilizador inválido");
    }

    eval_check_errorlog( "SD10.1" );

    if ( 1 != _eval_kill_data.status ) {
        eval_error( "(SD10) exactly 1 signal should be sent");
    } else if ( _eval_kill_data.pid != request.pidCliente || _eval_kill_data.sig != SIGHUP ) {
        eval_error( "(SD10) signal sent with invalid value and/or target");
    }

    unlink( FILE_DATABASE );

    eval_close_logs( "(SD10)" );
    return eval_complete("(SD10)");
}


void checkinClientDB_SD11 (CheckIn *request, char *nameDB, int indexClient, CheckIn itemDB) {
    _student_checkinClientDB_SD11( request, nameDB, indexClient, itemDB );
}

/**
 * Evaluate SD11
 **/
int eval_sd11( ) {

    eval_reset();
    eval_info("Evaluating SD11.a - %s...", question_text(questions,"11.a"));

    CheckIn dummyDB[10];
    int index_client = 1;

    initTestDB( dummyDB, 4 );
    CheckIn itemDB = dummyDB[ index_client ];
    // dummyDB[index_client].pidServidorDedicado = getpid();
    saveDB( FILE_DATABASE, dummyDB, 4 );

    CheckIn request = {
        .nif = 1000002,
        .senha = "pass_2",
        .pidCliente = -123,
        .pidServidorDedicado = -1234,
    };

    _eval_kill_data.status = 0;
    _eval_kill_data.action = 2; // don't send signal, just capture data

    EVAL_CATCH( checkinClientDB_SD11( &request, FILE_DATABASE, index_client, itemDB ) );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(SD11) bad termination");
    }

    if ( getpid() != request.pidServidorDedicado ) {
        eval_error("(SD11) Invalid return value for .pidServidorDedicado ");
    }

    eval_check_successlog( "(SD11.1) %s %s %d", request.nome, request.nrVoo, request.pidServidorDedicado );

    eval_check_successlog( "SD11.4" );

    // Check if file was updated correctly
    FILE *f = fopen( FILE_DATABASE, "r" );
    if ( f == NULL ) {
        eval_error("(SD11) Unable to open file %s for testing", FILE_DATABASE);
    } else {
        CheckIn r;

        fseek( f, index_client * sizeof(CheckIn), SEEK_SET );
        fread( &r, sizeof(CheckIn), 1, f );

        if ( r.nif != 1000002 )
            eval_error("(SD11) .nif is not correct in file record");
        if ( strcmp(r.senha, "pass_2") )
            eval_error("(SD11) .senha is not correct in file record");
        if ( strcmp(r.nome, "Name2 Surname2") )
            eval_error("(SD11) .nome is not correct in file record");
        if ( strcmp(r.nrVoo, "TAP002") )
            eval_error("(SD11) .nrVoo is not correct in file record" );
        if ( r.pidCliente != -123 )
            eval_error("(SD11) .pidCliente is not correct in file record");
        if ( r.pidServidorDedicado != getpid() )
            eval_error("(SD11) .pidServidorDedicado is not correct in file record");

        fclose(f);
    }

    eval_close_logs( "(SD11)" );


    eval_info("Evaluating SD11.b - %s...", question_text(questions,"11.b"));

    _eval_kill_data.status = 0;
    _eval_kill_data.action = 2; // don't send signal, just capture data

    unlink( FILE_DATABASE );

    EVAL_CATCH( checkinClientDB_SD11( &request, FILE_DATABASE, index_client, itemDB ) );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(SD11) Servidor não terminou sem ficheiro DB");
    }

    eval_check_successlog( "(SD11.1) %s %s %d", request.nome, request.nrVoo, request.pidServidorDedicado );

    eval_check_errorlog( "SD11.2" );

    if ( 1 != _eval_kill_data.status ) {
        eval_error( "(SD11) Exactly 1 signal should be sent");
    } else if ( _eval_kill_data.sig != SIGHUP || _eval_kill_data.pid != request.pidCliente ) {
        eval_error( "(SD11) signal sent with invalid value and/or target");
    }


    eval_info("Evaluating SD11.c - %s...", question_text(questions,"11.c"));

    saveDB( FILE_DATABASE, dummyDB, 4 );

    _eval_kill_data.status = 0;
    _eval_kill_data.action = 2; // don't send signal, just capture data

    EVAL_CATCH( checkinClientDB_SD11( &request, FILE_DATABASE, -1, itemDB ) );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(SD11) Servidor não terminou com erro em fseek()");
    }

    eval_check_successlog( "(SD11.1) %s %s %d", request.nome, request.nrVoo, request.pidServidorDedicado );

    eval_check_errorlog( "SD11.3" );

    if ( 1 != _eval_kill_data.status ) {
        eval_error( "(SD11) Exactly 1 signal should be sent");
    } else if ( _eval_kill_data.sig != SIGHUP || _eval_kill_data.pid != request.pidCliente ) {
        eval_error( "(SD11) signal sent with invalid value and/or target");
    }


    initTestDB( dummyDB, 4 );
    itemDB = dummyDB[ index_client ];
    saveDB( FILE_DATABASE, dummyDB, 4 );

    dummyDB[index_client].pidServidorDedicado = getpid();
    saveDB( "test_sd11d.dat", dummyDB, 4 );

    request = (CheckIn) {
        .nif = 1000002,
        .senha = "pass_2",
        .pidCliente = -1,
        .pidServidorDedicado = -1234,
    };

    _eval_kill_data.status = 0;
    _eval_kill_data.action = 2; // don't send signal, just capture data

    EVAL_CATCH( checkinClientDB_SD11( &request, FILE_DATABASE, index_client, itemDB ) );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(SD11) bad termination");
    }

    if ( compareDBfiles( FILE_DATABASE, "test_sd11d.dat" ) ) {
        eval_error("(SD11) Corrupted database file (bad write)");
    }

    eval_check_successlog( "SD11.1" );
    eval_check_successlog( "SD11.4" );

    unlink( "test_sd11d.dat" );
    unlink( FILE_DATABASE );

    eval_close_logs( "(SD11)" );
    return eval_complete("(SD11)");
}

void sendAckCheckIn_SD12 (int pidCliente) {
    _student_sendAckCheckIn_SD12( pidCliente );
}

/**
 * Evaluate SD12
 **/
int eval_sd12( ) {

    eval_reset();
    eval_info("Evaluating SD12 - %s...", question_text(questions,"12"));

    _eval_kill_data.status = 0;
    _eval_kill_data.action = 2; // don't send signal, just capture data

    _eval_sleep_data.status = 0;
    _eval_sleep_data.action = 1;

    int pidCliente = 1234;

    EVAL_CATCH( sendAckCheckIn_SD12( pidCliente ) );

    if ( 0 != _eval_env.stat ) {
        eval_error( "(SD12) bad termination");
    }

    if ( 1 !=  _eval_sleep_data.status ) {
        eval_error( "(SD12) sleep should have been called exactly once");
    } else {
        if ( _eval_sleep_data.seconds < 1 || _eval_sleep_data.seconds > MAX_ESPERA ) {
            eval_error( "(SD12) sleep called with wrong time");
        }
        eval_check_successlog( "(SD12) %d", _eval_sleep_data.seconds );
    }

    if ( 1 != _eval_kill_data.status ) {
        eval_error( "(SD12) Exactly 1 signal should be sent");
    } else if ( _eval_kill_data.sig != SIGUSR1 || _eval_kill_data.pid != pidCliente ) {
        eval_error( "(SD12) signal sent with invalid value and/or target");
    }


    eval_close_logs( "(SD12)" );
    return eval_complete("(SD12)");
}

void closeSessionDB_SD13 (CheckIn clientRequest, char *nameDB, int indexClient) {
    _student_closeSessionDB_SD13( clientRequest, nameDB, indexClient );
}


/**
 * Evaluate SD13
 **/
int eval_sd13( ) {

    eval_reset();
    eval_info("Evaluating SD13.a - %s...", question_text(questions,"13.a"));

    char nameDB[] = FILE_DATABASE;

    CheckIn dummyDB[10];
    initTestDB( dummyDB, 4 );

    int index_client = 2;

    dummyDB[ index_client ].pidCliente = 16384;
    dummyDB[ index_client ].pidServidorDedicado = 32768;

    saveDB( nameDB, dummyDB, 4 );

    CheckIn l = dummyDB[ index_client ];

    EVAL_CATCH( closeSessionDB_SD13( l, nameDB, index_client ) );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(SD13) bad termination");
    }

    FILE * f = fopen( nameDB, "r" );
    if ( f == NULL ) {
        eval_error("(SD13) Unable to open %s file for testing", nameDB );
    } else {
        CheckIn r;

        fseek( f, index_client * sizeof(CheckIn), SEEK_SET );
        fread( &r, sizeof(CheckIn), 1, f );

        if ( r.nif != l.nif )
            eval_error("(SD13) .nif is not correct in file record");

        if ( strcmp(r.senha, l.senha ) )
            eval_error("(SD13) .nif is not correct in file record");
        if ( strcmp(r.nome, l.nome ) )
            eval_error("(SD13) .nome is not correct in file record");
        if ( strcmp(r.nrVoo, l.nrVoo ) )
            eval_error("(SD13) .nrVoo is not correct in file record");
        if ( r.pidCliente != -1 )
            eval_error("(SD13) .pidCliente is not correct in file record");
        if ( r.pidServidorDedicado != -1 )
            eval_error("(SD13) .pidServidorDedicado is not correct in file record");

        fclose(f);
    }

    eval_check_successlog( "SD13" );
    eval_check_successlog( "SD13" );
    eval_check_successlog( "SD13" );

    unlink( nameDB );

    eval_close_logs( "(SD13)" );

    // Test with DB corruption
    eval_info("Evaluating SD13.b - %s...", question_text(questions,"13.b"));

    _eval_unlink_data.action = 1;

    initTestDB( dummyDB, 4 );
    dummyDB[ index_client ].pidCliente = 16384;
    dummyDB[ index_client ].pidServidorDedicado = 2004;
    saveDB( nameDB, dummyDB, 4 );

    EVAL_CATCH( closeSessionDB_SD13( l, nameDB, index_client ) );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(SD13) bad termination");
    }

    dummyDB[ index_client ].pidCliente = -1;
    dummyDB[ index_client ].pidServidorDedicado = -1;
    saveDB( "test_sd13b.dat", dummyDB, 4 );

    if ( compareDBfiles( nameDB, "test_sd13b.dat" ) ) {
        eval_error("(SD13) Corrupted database file (bad write)");
    }

    unlink( "test_sd13b.dat" );
    unlink( nameDB );

    eval_check_successlog( "SD13" );
    eval_check_successlog( "SD13" );
    eval_check_successlog( "SD13" );

    eval_close_logs( "(SD13)" );

    // Test with missing DB
    eval_info("Evaluating SD13.c - %s...", question_text(questions,"13.c"));

    EVAL_CATCH( closeSessionDB_SD13( l, nameDB, index_client ) );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(SD13) bad termination");
    }

    eval_check_errorlog( "SD13" );

    eval_close_logs( "(SD13)" );
    return eval_complete("(SD13)");
}

void trataSinalSIGUSR2_SD14 (int sinalRecebido) {
    _student_trataSinalSIGUSR2_SD14 (sinalRecebido);
}

int eval_sd14( ) {

    eval_reset();
    eval_info("Evaluating SD14 - %s...", question_text(questions,"14"));

    EVAL_CATCH( trataSinalSIGUSR2_SD14(SIGINT) );

    if ( 1 != _eval_env.stat ) {
        eval_error( "(SD14) Dedicated server was not shutdown properly");
    } else if ( 0 != _eval_exit_data.status ) {
        eval_error("(SD14) Invalid exit status");
    }

    eval_check_successlog( "(SD14) SD: Recebi pedido do Servidor para terminar" );

    eval_close_logs( "(SD14)" );
    return eval_complete("(SD14)");

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
 * Will add the number errors found to the nerr variable.
 * If opts.stop_on_error is set and an error is found the code will
 * stop immediately
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
    eval_info(" %s/servidor.c\n", TOSTRING( _EVAL ) );

    int nerr = 0;

    RUNTEST( eval_s1() );
    RUNTEST( eval_s2() );
    RUNTEST( eval_s3() );
    RUNTEST( eval_s4() );
    RUNTEST( eval_s5() );
    RUNTEST( eval_s6() );
    RUNTEST( eval_s7() );
    RUNTEST( eval_s8() );
    RUNTEST( eval_sd9() );
    RUNTEST( eval_sd10() );
    RUNTEST( eval_sd11() );
    RUNTEST( eval_sd12() );
    RUNTEST( eval_sd13() );
    RUNTEST( eval_sd14() );

    eval_info("Finished." );

    // Printout error summary report
    _eval_stats.error = nerr;
    eval_complete("servidor");

    /* Print out full evaluation report */
    if ( opts.export ) question_export(questions, "servidor" );
    if ( opts.list ) question_list(questions, "servidor" );

}
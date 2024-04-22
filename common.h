/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos 2023/2024, Enunciado Versão 3+
 **
 ** Este Módulo não deverá ser alterado, e não precisa ser entregue
 ** Nome do Módulo: common.h
 ** Descrição/Explicação do Módulo:
 **     Definição das estruturas de dados comuns aos módulos servidor e cliente
 **
 ******************************************************************************/
#ifndef __COMMON_H__
#define __COMMON_H__

#include "/home/so/utils/include/so_utils.h"
#include <signal.h>     // Header para as constantes SIG_* e as funções signal() e kill()
#include <unistd.h>     // Header para as funções alarm(), pause(), sleep(), fork(), exec*() e get*pid()
#include <sys/wait.h>   // Header para a função wait()
#include <sys/stat.h>   // Header para as constantes S_ISFIFO e as funções stat() e mkfifo()
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

#define MAX_ESPERA  5   // Tempo máximo de espera por parte do Cliente

typedef struct {
    int  nif;                   // Número de contribuinte do passageiro
    char senha[40];             // Senha do passageiro
    char nome[60];              // Nome do passageiro
    char nrVoo[8];              // Número do voo escolhido
    int  pidCliente;            // PID do processo Cliente
    int  pidServidorDedicado;   // PID do processo Servidor Dedicado
} CheckIn;

#define FILE_SUFFIX_FIFO ".fifo"                   // Sufixo (extensão) para os nomes dos FIFOs (Named Pipes)
#define FILE_REQUESTS    "server" FILE_SUFFIX_FIFO // Nome do FIFO (Named Pipe) que serve para o Cliente fazer os pedidos ao Servidor
#define FILE_DATABASE    "bd_passageiros.dat"      // Ficheiro de acesso direto que armazena a lista de passageiros

/* Protótipos de funções */
void checkExistsDB_S1 (char *);                              // S1:   Função a ser implementada pelos alunos
void createFifo_S2 (char *);                                 // S2:   Função a ser implementada pelos alunos
void triggerSignals_S3 ();                                   // S3:   Função a ser implementada pelos alunos
CheckIn readRequest_S4 (char *);                             // S4:   Função a ser implementada pelos alunos
int createServidorDedicado_S5 ();                            // S5:   Função a ser implementada pelos alunos
void trataSinalSIGINT_S6 (int);                              // S6:   Função a ser implementada pelos alunos
void deleteFifoAndExit_S7 ();                                // S7:   Função a ser implementada pelos alunos
void trataSinalSIGCHLD_S8 (int);                             // S8:   Função a ser implementada pelos alunos
void triggerSignals_SD9 ();                                  // SD9:  Função a ser implementada pelos alunos
int searchClientDB_SD10 (CheckIn, char *, CheckIn *);        // SD10: Função a ser implementada pelos alunos
void checkinClientDB_SD11 (CheckIn *, char *, int, CheckIn); // SD11: Função a ser implementada pelos alunos
void sendAckCheckIn_SD12 (int);                              // SD12: Função a ser implementada pelos alunos
void closeSessionDB_SD13 (CheckIn, char *, int);             // SD13: Função a ser implementada pelos alunos
void trataSinalSIGUSR2_SD14 (int);                           // SD14: Função a ser implementada pelos alunos

void checkExistsFifoServidor_C1 (char *);                    // C1:   Função a ser implementada pelos alunos
void triggerSignals_C2 ();                                   // C2:   Função a ser implementada pelos alunos
CheckIn getDadosPedidoUtilizador_C3_C4 ();                   // C3+C4:Função a ser implementada pelos alunos
void writeRequest_C5 (CheckIn, char *);                      // C5:   Função a ser implementada pelos alunos
void configureTimer_C6 (int);                                // C6:   Função a ser implementada pelos alunos
void waitForEvents_C7 ();                                    // C7:   Função a ser implementada pelos alunos
void trataSinalSIGUSR1_C8 (int);                             // C8:   Função a ser implementada pelos alunos
void trataSinalSIGHUP_C9 (int);                              // C9:   Função a ser implementada pelos alunos
void trataSinalSIGINT_C10 (int);                             // C10:  Função a ser implementada pelos alunos
void trataSinalSIGALRM_C11 (int);                            // C11:  Função a ser implementada pelos alunos

#endif  // __COMMON_H__
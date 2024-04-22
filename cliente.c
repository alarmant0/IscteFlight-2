/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos 2023/2024, Enunciado Versão 3+
 **
 ** Aluno: Nº: a105448      Nome: David Pinheiro
 ** Nome do Módulo: cliente.c
 **
 ******************************************************************************/

#define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "common.h"

/**
 * @brief Processamento do processo Cliente
 *        "os alunos não deverão alterar a função main(), apenas compreender o que faz.
 *         Deverão, sim, completar as funções seguintes à main(), nos locais onde está claramente assinalado
 *         '// Substituir este comentário pelo código da função a ser implementado pelo aluno' "
 */
int main () {
    // C1
    checkExistsFifoServidor_C1(FILE_REQUESTS);
    // C2
    triggerSignals_C2();
    // C3 + C4
    CheckIn clientRequest = getDadosPedidoUtilizador_C3_C4();
    // C5
    writeRequest_C5(clientRequest, FILE_REQUESTS);
    // C6
    configureTimer_C6(MAX_ESPERA);
    // C7
    waitForEvents_C7();
    so_exit_on_error(-1, "ERRO: O cliente nunca devia chegar a este ponto");
    return 0; // MUDAR
}

/**
 *  "O módulo Cliente é responsável pela interação com o utilizador.
 *   Após o login do utilizador, este poderá realizar atividades durante o tempo da sessão.
 *   Assim, definem-se as seguintes tarefas a desenvolver:"
 */

/**
 * @brief C1       Ler a descrição da tarefa C1 no enunciado
 * @param nameFifo Nome do FIFO servidor (i.e., FILE_REQUESTS)
 */
void checkExistsFifoServidor_C1 (char *nameFifo) {
    so_debug("< [@param nameFifo:%s]", nameFifo);
    struct stat statbuf;
    if (access(nameFifo, F_OK) != -1) {
        if (stat(nameFifo, &statbuf) == -1) {
            so_error("C1", "Erro ao obter informações sobre o arquivo '%s'", nameFifo);
            exit(1);
        }
        if ((statbuf.st_mode & S_IFMT) == S_IFIFO) {
            so_success("C1", "O ficheiro existe e é um FIFO.");
        } else {
            so_error("C1", "O arquivo '%s' não é um FIFO.", nameFifo);
            exit(1);
        }
    } else {
        so_error("C1", "O ficheiro '%s' não existe!", nameFifo);
        exit(1);
    }
    so_debug(">");
}

/**
 * @brief C2   Ler a descrição da tarefa C2 no enunciado
 */
void triggerSignals_C2 () {
    so_debug("<");

    if (signal(SIGUSR1, trataSinalSIGUSR1_C8)) {
        so_error("C2", "Erro no SIGUSR1");
        exit(1);
    }
    if (signal(SIGHUP, trataSinalSIGHUP_C9)) {
        so_error("C2", "Erro no SIGHUP");
        exit(1);
    }
    if (signal(SIGINT, trataSinalSIGINT_C10)) {
        so_error("C2", "Erro no SIGINT");
        exit(1);
    }
    if (signal(SIGALRM, trataSinalSIGALRM_C11)) {
        so_error("C2", "Erro no SIGALRM");
    }
    so_success("C2", "Sinais armados");
    so_debug(">");
}

/**
 * @brief C3+C4    Ler a descrição das tarefas C3 e C4 no enunciado
 * @return CheckIn Elemento com os dados preenchidos. Se nif=-1, significa que o elemento é inválido
 */
CheckIn getDadosPedidoUtilizador_C3_C4 () {
    CheckIn request;
    request.nif = -1;   // Por omissão retorna erro
    so_debug("<");
    printf("IscteFlight: Check-in Online");
    printf("----------------------------");
    printf("Introduxa o seu NIF: "); 
    scanf("%d", &request.nif);  

    if(request.nif <= 0 || request.nif > 999999999){
        so_error("C3","NIF INVÁLIDO");
        exit(1);
    } 

    printf("Introduza a sua senha: "); 
    scanf("%s", request.senha);

    request.pidCliente = getpid();
    so_success("C4", "%d %s %d", request.nif, request.senha, request.pidCliente);

    so_debug("> [@return nif:%d, senha:%s, pidCliente:%d]", request.nif, request.senha, request.pidCliente);
    return request;
}

/**
 * @brief C5       Ler a descrição da tarefa C5 no enunciado
 * @param request  Elemento com os dados a enviar
 * @param nameFifo O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 */
void writeRequest_C5 (CheckIn request, char *nameFifo) {
    int fd;
    char buffer[sizeof(CheckIn)]; 
    so_debug("< [@param request.nif:%d, request.senha:%s, request.pidCliente:%d, nameFifo:%s]",
                                        request.nif, request.senha, request.pidCliente, nameFifo);
    fd = open(nameFifo, O_WRONLY);
    if (fd == -1) {
        so_error("C5", "ERRO EM ABRIR O FIFO %s", nameFifo);
        exit(1);
    }
    else {
        so_success("C5", "SUCESSO EM ABRIR O FIFO %s", nameFifo);
    }

    snprintf(buffer, sizeof(buffer), "%d\n%s\n%d\n", request.nif, request.senha, request.pidCliente);

    if (write(fd, buffer, strlen(buffer)) == -1) {
        so_error("C5", "ERRO NA ESCRITA DO FIFO %s", nameFifo);
        exit(1);
    } else {
        so_success("C5", "SUCESSO NA ESCRITA DO FIFO %s", nameFifo);
    }

    close(fd);
    so_debug(">");
}

/**
 * @brief C6          Ler a descrição da tarefa C6 no enunciado
 * @param tempoEspera o tempo em segundos que queremos pedir para marcar o timer do SO (i.e., MAX_ESPERA)
 */
void configureTimer_C6 (int tempoEspera) {
    so_debug("< [@param tempoEspera:%d]", tempoEspera);
    alarm(tempoEspera);
    so_success("C6", "Espera resposta em %d segundos", tempoEspera);
    so_debug(">");
}

/**
 * @brief C7 Ler a descrição da tarefa C7 no enunciado
 */
void waitForEvents_C7 () {
    so_debug("<");
    pause();
    so_debug(">");
}

/**
 * @brief C8            Ler a descrição da tarefa C8 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGUSR1_C8 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);
    so_success("C8", "Check-in concluído com sucesso");
    exit(0);
    so_debug(">");
}

/**
 * @brief C9            Ler a descrição da tarefa C9 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGHUP_C9 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);
    so_success("C9", "Check-in concluído sem sucesso");
    exit(1);
    so_debug(">");
}

/**
 * @brief C10           Ler a descrição da tarefa C10 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGINT_C10 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);
    so_success("C10", "Cliente: Shutdown");
    exit(0);
    so_debug(">");
}

/**
 * @brief C11           Ler a descrição da tarefa C11 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGALRM_C11 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);
    so_error("C11", "Cliente: Timeout");
    exit(1);
    so_debug(">");
}
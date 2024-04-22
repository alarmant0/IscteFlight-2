/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos 2023/2024, Enunciado Versão 3+
 **
 ** Aluno: Nº: a105448      Nome: David Miguel Borges Pinheiro
 ** Nome do Módulo: servidor.c
 **
 ******************************************************************************/

#define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "common.h"

/*** Variáveis Globais ***/
CheckIn clientRequest; // Variável que tem o pedido enviado do Cliente para o Servidor

/**
 * @brief Processamento do processo Servidor e dos processos Servidor Dedicado
 *        "os alunos não deverão alterar a função main(), apenas compreender o que faz.
 *         Deverão, sim, completar as funções seguintes à main(), nos locais onde está claramente assinalado
 *         '// Substituir este comentário pelo código da função a ser implementado pelo aluno' "
 */
int main () {
    // S1
    checkExistsDB_S1(FILE_DATABASE);
    // S2
    createFifo_S2(FILE_REQUESTS);
    // S3
    triggerSignals_S3(FILE_REQUESTS);

    int indexClient;       // Índice do cliente que fez o pedido ao servidor/servidor dedicado na BD

    // S4: CICLO1
    while (TRUE) {
        // S4
        clientRequest = readRequest_S4(FILE_REQUESTS); // S4: "Se houver erro (...) clientRequest.nif == -1"
        if (clientRequest.nif < 0)   // S4: "Se houver erro na abertura do FIFO ou na leitura do mesmo, (...)"
            continue;                // S4: "(...) e recomeça o Ciclo1 neste mesmo passo S4, lendo um novo pedido"

        // S5
        int pidServidorDedicado = createServidorDedicado_S5();
        if (pidServidorDedicado > 0) // S5: "o processo Servidor (pai) (...)"
            continue;                // S5: "(...) recomeça o Ciclo1 no passo S4 (ou seja, volta a aguardar novo pedido)"
        // S5: "o Servidor Dedicado (que tem o PID pidServidorDedicado) segue para o passo SD9"

        // SD9
        triggerSignals_SD9();
        // SD10
        CheckIn itemBD;
        indexClient = searchClientDB_SD10(clientRequest, FILE_DATABASE, &itemBD);
        // SD11
        checkinClientDB_SD11(&clientRequest, FILE_DATABASE, indexClient, itemBD);
        // SD12
        sendAckCheckIn_SD12(clientRequest.pidCliente);
        // SD13
        closeSessionDB_SD13(clientRequest, FILE_DATABASE, indexClient);
        so_exit_on_error(-1, "ERRO: O servidor dedicado nunca devia chegar a este ponto");
    }
}

/**
 *  "O módulo Servidor é responsável pelo processamento do check-in dos passageiros. 
 *   Está dividido em duas partes, um Servidor (pai) e zero ou mais Servidores Dedicados (filhos).
 *   Este módulo realiza as seguintes tarefas:"
 */

/**
 * @brief S1     Ler a descrição da tarefa S1 no enunciado
 * @param nameDB O nome da base de dados (i.e., FILE_DATABASE)
 */
void checkExistsDB_S1 (char *nameDB) {
    so_debug("< [@param nameDB:%s]", nameDB);  
    if (access(nameDB, R_OK | W_OK | F_OK) == -1) {
        so_error("S1",""); 
        exit(1);
    }
    so_success("S1","");                       
    so_debug(">");                             
}
/**
 * @brief S2       Ler a descrição da tarefa S2 no enunciado
 * @param nameFifo O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 */
void createFifo_S2 (char *nameFifo) {
    so_debug("< [@param nameFifo:%s]", nameFifo); 
    if (mkfifo(nameFifo, 0666) == -1) {
        so_error("S2","");                        
        exit(1);
    }
    if (access(nameFifo, F_OK) != -1) {
        if (unlink(nameFifo) == -1) {
            so_error("S2","");
            exit(1);
        }
    }
    so_success("S2","");                          
    so_debug(">");                               
}

/**
 * @brief S3   Ler a descrição da tarefa S3 no enunciado
 */
void triggerSignals_S3 () {
    so_debug("<");                                
    if (signal(SIGCHLD,trataSinalSIGCHLD_S8)) {   // Armazena o manipulador para o sinal SIGCHLD
        so_error("S3","");                        
        exit(1);                                  // Encerra o programa devido ao erro
    }
    if (signal(SIGINT, trataSinalSIGINT_S6)) {    // Armazena o manipulador para o sinal SIGINT
        so_error("S3","");                        
        exit(1);                                  // Encerra o programa devido ao erro
    }
    so_success("S3","");                         
    so_debug(">");                                
}


/**
 * @brief S4       O CICLO1 já está a ser feito na função main(). Ler a descrição da tarefa S4 no enunciado
 * @param nameFifo O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 * @return CheckIn Elemento com os dados preenchidos. Se nif=-1, significa que o elemento é inválido
 */
CheckIn readRequest_S4(char *fifoName) {
    CheckIn request;                             // Cria uma variável para armazenar o pedido
    request.nif = -1;                            // Inicializa o campo NIF com -1, indicando invalidade
    int fileDescriptor, numBytesRead;            // Variáveis para manipular o arquivo FIFO
    char readBuffer[256];                        // Buffer para leitura dos dados do FIFO

    fileDescriptor = open(fifoName, O_RDONLY);   // Abre o FIFO para leitura
    if (fileDescriptor == -1) {                  // Verifica se a abertura falhou
        so_error("S4", "", fifoName); 
        deleteFifoAndExit_S7();                  // Chama a função para deletar o FIFO e sair
        return request;                          // Retorna o pedido como inválido
    }

    numBytesRead = read(fileDescriptor, readBuffer, sizeof(readBuffer) - 1); // Lê os dados do FIFO
    if (numBytesRead <= 0) {                     // Verifica se a leitura falhou ou se não há dados
        so_error("S4", "", fifoName); 
        close(fileDescriptor);                   // Fecha o descriptor do arquivo
        deleteFifoAndExit_S7();                  // Chama a função para deletar o FIFO e sair
        return request;                          // Retorna o pedido como inválido
    }

    readBuffer[numBytesRead] = '\0';             // Assegura o término da string lida
    sscanf(readBuffer, "%d %s %d", &request.nif, request.senha, &request.pidCliente); // Extrai dados do buffer

    if (request.nif > 0 && request.pidCliente > 0) { // Verifica se os dados extraídos são válidos
        so_success("S4", "%d %s %d", request.nif, request.senha, request.pidCliente); // Registra sucesso
    } else {
        so_error("S4", "", request.nif, request.pidCliente); 
        close(fileDescriptor);                   // Fecha o descriptor do arquivo
        deleteFifoAndExit_S7();                  // Chama a função para deletar o FIFO e sair
        return request;                          // Retorna o pedido como inválido
    }

    close(fileDescriptor);                       // Fecha o descriptor do arquivo após a leitura bem-sucedida
    return request;                              // Retorna o pedido extraído
}


/**
 * @brief S5   Ler a descrição da tarefa S5 no enunciado
 * @return int PID do processo filho, se for o processo Servidor (pai),
 *             0 se for o processo Servidor Dedicado (filho), ou -1 em caso de erro.
 */
int createServidorDedicado_S5() {
    int childPid = -1;   // Inicializa o PID do processo filho com -1
    int forkResult;      // Variável para armazenar o resultado do fork()

    so_debug("<");       

    forkResult = fork(); // Cria um novo processo (filho)
    if (forkResult == -1) { // Verifica se o fork falhou
        so_error("S5", "ERRO NO FORK"); // Registra um erro se o fork falhar
        deleteFifoAndExit_S7();         // Chama a função para deletar o FIFO e sair
        return childPid;                // Retorna -1 indicando erro
    }

    if (forkResult == 0) {              // Se for o processo filho
        childPid = 0;                   // Configura o PID do filho para 0
    } else {                            // Se for o processo pai
        so_success("S5", "Servidor: Iniciei SD %d", forkResult); // Registra sucesso no início do Servidor Dedicado
        childPid = forkResult;          // Atualiza childPid com o PID do processo filho
    }

    so_debug("> [@return:%d]", childPid); 
    return childPid;                      // Retorna o PID do processo filho ou 0 se for o filho
}

/**
 * @brief S6            Ler a descrição das tarefas S6 e S7 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGINT_S6(int signalReceived) {
    so_debug("< [@param signalReceived:%d]", signalReceived); 

    FILE *databaseFile;               // Variável para o arquivo da base de dados
    CheckIn checkInData;              // Variável para armazenar dados lidos do arquivo

    so_success("S6", "Servidor: Start Shutdown"); // Mensagem indicando início do desligamento

    databaseFile = fopen(FILE_DATABASE, "rb");    // Abre o arquivo da base de dados para leitura
    if (databaseFile == NULL) {                   // Verifica se a abertura falhou
        so_error("S6.1", "", FILE_DATABASE); 
        deleteFifoAndExit_S7();                   // Deleta o FIFO e sai
    }
    so_success("S6.1", "");                       // Registra sucesso na abertura do arquivo

    while (1) {
        ssize_t bytesRead = fread(&checkInData, sizeof(CheckIn), 1, databaseFile); // Lê dados do arquivo
        if (bytesRead < 0) {
            so_error("S6.2", "", FILE_DATABASE); 
            fclose(databaseFile);                   // Fecha o arquivo
            deleteFifoAndExit_S7();                 // Deleta o FIFO e sai
        }
        if (bytesRead == 0) {
            break;  // Sai do loop se não houver mais dados para ler
        }

        if (checkInData.pidServidorDedicado > 0) {
            kill(checkInData.pidServidorDedicado, SIGUSR2); // Envia sinal SIGUSR2 para cada Servidor Dedicado
            so_success("S6.3", "Servidor: Shutdown SD %d", checkInData.pidServidorDedicado); // Registra sucesso no envio do sinal
        }
    }

    so_success("S6.2", ""); 
    fclose(databaseFile);                         // Fecha o arquivo
    deleteFifoAndExit_S7();                       // Deleta o FIFO e sai
    so_debug(">");                                // Mensagem de debug para indicar fim da função
}


/**
 * @brief S7 Ler a descrição da tarefa S7 no enunciado
 */
void deleteFifoAndExit_S7() {
    so_debug("<");  

    if (unlink(FILE_REQUESTS) != -1) { // Tenta remover o FIFO
        so_success("S7", "Servidor: End Shutdown"); // Registra sucesso no término do desligamento
    } else {
        so_error("S7", "", FILE_REQUESTS); // Registra erro ao tentar remover o FIFO
    }

    so_debug(">"); 
    exit(0);      
}

/**
 * @brief S8            Ler a descrição da tarefa S8 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGCHLD_S8(int signalReceived) {
    so_debug("< [@param signalReceived:%d]", signalReceived); 

    int pid, status; // Variáveis para PID do processo e status de terminação
    pid = wait(&status); // Espera por mudanças de estado em qualquer processo filho

    if (pid > 0) { // Se um PID válido for retornado
        so_success("S8", "Servidor: Confirmo fim de SD %d", pid); // Confirma o término de um processo filho
    } else {
        so_error("S8", ""); // Registra erro se falhar ao esperar
    }

    so_debug(">");
}


/**
 * @brief SD9  Ler a descrição da tarefa SD9 no enunciado
 */
void triggerSignals_SD9() {
    so_debug("<"); 

    if (signal(SIGINT, SIG_IGN) == SIG_ERR) { // Ignora o sinal SIGINT
        so_error("SD9", ""); // Registra erro se falhar
    }

    if (signal(SIGUSR2, trataSinalSIGUSR2_SD14) == SIG_ERR) { // Define o manipulador para SIGUSR2
        so_error("SD9", ""); // Registra erro se falhar
    } else {
        so_success("SD9", "SINAIS ARMADOS: %d", getpid()); // Registra sucesso na armação dos sinais
    }

    so_debug(">"); 
}


/**
 * @brief SD10    Ler a descrição da tarefa SD10 no enunciado
 * @param request O pedido do cliente
 * @param nameDB  O nome da base de dados
 * @param itemDB  O endereço de estrutura CheckIn a ser preenchida nesta função com o elemento da BD
 * @return int    Em caso de sucesso, retorna o índice de itemDB no ficheiro nameDB.
 */
int searchClientDB_SD10(CheckIn clientRequest, char *nameDB, CheckIn *itemDB) {
    FILE *dbFile = fopen(nameDB, "rb"); // Abre a base de dados para leitura binária
    if (!dbFile) {
        so_error("SD10.1", "Erro ao abrir o arquivo: %s", nameDB); // Registra erro se falhar
        kill(clientRequest.pidCliente, SIGHUP); // Envia sinal de erro ao cliente
        exit(1); // Termina o servidor dedicado
    }

    int indexClient = 0; // Inicia o índice em 0 para uso no loop
    CheckIn checkInData;

    while (fread(&checkInData, sizeof(CheckIn), 1, dbFile)) {
        if (checkInData.nif == clientRequest.nif) {
            if (strcmp(checkInData.senha, clientRequest.senha) == 0) {
                fclose(dbFile);
                *itemDB = checkInData;  // Copia os dados lidos para itemDB
                so_success("SD10.3", "%d", indexClient);
                return indexClient;  // Sucesso, retorna o índice encontrado
            } else {
                so_error("SD10.3", "Cliente %d: Senha errada", clientRequest.nif);
                fclose(dbFile);
                kill(clientRequest.pidCliente, SIGHUP);  // Senha incorreta, sinal ao cliente
                exit(1);
            }
        }
        indexClient++;
    }

    so_error("SD10.1", "Cliente %d: não encontrado", clientRequest.nif); // Registra erro se cliente não for encontrado
    fclose(dbFile); // Fecha o arquivo
    kill(clientRequest.pidCliente, SIGHUP); // Envia sinal de erro ao cliente
    exit(1); // Termina o servidor dedicado
    return -1;
}

/**
 * @brief SD11        Ler a descrição da tarefa SD11 no enunciado
 * @param request     O endereço do pedido do cliente (endereço é necessário pois será alterado)
 * @param nameDB      O nome da base de dados
 * @param indexClient O índica na base de dados do elemento correspondente ao cliente
 * @param itemDB      O elemento da BD correspondente ao cliente
 */
void checkinClientDB_SD11(CheckIn *clientData, char *databaseName, int clientIndex, CheckIn databaseRecord) {
    FILE *databaseFile;
    long fileOffset;

    strcpy(clientData->nome, databaseRecord.nome); // Copia o nome do registro da BD para o cliente
    strcpy(clientData->nrVoo, databaseRecord.nrVoo); // Copia o número do voo do registro da BD para o cliente
    clientData->pidServidorDedicado = getpid(); // Atualiza o PID do servidor dedicado
    so_success("SD11.1", "%s %s %d", clientData->nome, clientData->nrVoo, clientData->pidServidorDedicado); // Registra sucesso

    databaseFile = fopen(databaseName, "r+"); // Abre a base de dados para leitura e escrita
    if (databaseFile == NULL) {
        so_error("SD11.2", "", databaseName); // Registra erro se falhar
        kill(clientData->pidCliente, SIGHUP); // Envia sinal de erro ao cliente
        exit(1); // Encerra após erro
    }

    fileOffset = clientIndex * sizeof(CheckIn); // Calcula o deslocamento para o registro do cliente
    if (fseek(databaseFile, fileOffset, SEEK_SET) != 0) {
        so_error("SD11.3", "", databaseName); // Registra erro se falhar
        fclose(databaseFile); // Fecha o arquivo
        kill(clientData->pidCliente, SIGHUP); // Envia sinal de erro ao cliente
        exit(1); // Encerra após erro
    }

    if (fwrite(clientData, sizeof(CheckIn), 1, databaseFile) == 1) {
        so_success("SD11.4", "Dados escritos com sucesso"); // Registra sucesso na escrita dos dados
    } else {
        so_error("SD11.4", "", databaseName); // Registra erro na escrita
        fclose(databaseFile); // Fecha o arquivo
        kill(clientData->pidCliente, SIGHUP); // Envia sinal de erro ao cliente
        return; // Encerra após erro
    }

    fclose(databaseFile); // Fecha o arquivo após a operação bem-sucedida
}



/**
 * @brief SD12       Ler a descrição da tarefa SD12 no enunciado
 * @param pidCliente PID (Process ID) do processo Cliente
 */
void sendAckCheckIn_SD12 (int pidCliente) {
    int tp;
    so_debug("< [@param pidCliente:%d]", pidCliente); 

    tp = rand() % MAX_ESPERA + 1; // Gera um tempo de espera aleatório
    so_success("SD12", "%d", tp); // Registra sucesso com o tempo gerado
    sleep(tp); // Espera pelo tempo aleatório
    kill(pidCliente, SIGUSR1); // Envia sinal SIGUSR1 ao cliente após a espera
    return; // Encerra a função

    so_debug(">"); 
}


/**
 * @brief SD13          Ler a descrição da tarefa SD13 no enunciado
 * @param clientRequest O endereço do pedido do cliente
 * @param nameDB        O nome da base de dados
 * @param indexClient   O índica na base de dados do elemento correspondente ao cliente
 */
void closeSessionDB_SD13 (CheckIn clientRequest, char *nameDB, int indexClient) {
    FILE *fileDB;
    long fileOffset;

    fileDB = fopen(nameDB, "r+"); // Abre a base de dados para leitura e escrita
    if (!fileDB) {
        so_error("SD13.1", "", nameDB); // Registra erro se falhar
        exit(1); // Encerra o programa devido ao erro
    }
    so_success("SD13.1", "", nameDB); // Registra sucesso na abertura do arquivo

    clientRequest.pidCliente = -1; // Redefine o PID do cliente
    clientRequest.pidServidorDedicado = -1; // Redefine o PID do servidor dedicado

    fileOffset = indexClient * sizeof(CheckIn); // Calcula o deslocamento no arquivo
    if (fseek(fileDB, fileOffset, SEEK_SET) != 0) {
        fclose(fileDB); // Fecha o arquivo se falhar no posicionamento
        so_error("SD13.2", "", nameDB); 
        exit(1); // Encerra o programa devido ao erro
    }
    so_success("SD13.2", "", nameDB); // Registra sucesso no posicionamento

    if (fwrite(&clientRequest, sizeof(CheckIn), 1, fileDB) != 1) {
        fclose(fileDB); // Fecha o arquivo se falhar na escrita
        so_error("SD13.3", "", nameDB); 
        exit(1); // Encerra o programa devido ao erro
    }
    so_success("SD13.3", "", nameDB); // Registra sucesso na remoção dos dados

    fclose(fileDB); // Fecha o arquivo após a operação bem-sucedida
    exit(0); // Encerra o programa
}


/**
 * @brief SD14          Ler a descrição da tarefa SD14 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGUSR2_SD14 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido); 

    so_success("SD14","SD: Recebi pedido do Servidor para terminar"); // Registra o sucesso ao receber o sinal para terminar
    exit(0); // Encerra o processo do servidor dedicado

    so_debug(">"); 
}
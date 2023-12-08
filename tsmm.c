#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <regex.h>
#include "tsmm.h"


/* Cria uma nova fila de descritores de tarefa.
   Aloca memória para a estrutura TaskDescriptorQueue e inicializa seus ponteiros front e rear como NULL,
   indicando que a fila está vazia.
   Retorna um ponteiro para a nova fila criada.
*/
TaskDescriptorQueue* createTaskDescriptorQueue() {
    TaskDescriptorQueue* queue = (TaskDescriptorQueue*)malloc(sizeof(TaskDescriptorQueue));
    queue->front = queue->rear = NULL;
    return queue;
}//createTaskDescriptorQueue()



/* Verifica se a fila de descritores de tarefa está vazia.
   Verifica se o ponteiro 'front' da fila passada como parâmetro é NULL,
   indicando que não há nenhum elemento na fila.
   Retorna TRUE se a fila estiver vazia, caso contrário retorna FALSE.
*/
boolean isTaskDescriptorQueueEmpty(TaskDescriptorQueue* queue) {
    return (queue->front == NULL);
}//isTaskDescriptorQueueEmpty()


/* Insere um descritor de tarefa na fila de descritores de tarefa.
   Adiciona um novo nó contendo o descritor de tarefa fornecido à fila.
   Se a fila estiver vazia, o novo nó é inserido como o primeiro e último elemento da fila.
   Caso contrário, o novo nó é adicionado ao final da fila, atualizando o ponteiro 'rear'.
   Parâmetros:
   - queue: Ponteiro para a fila de descritores de tarefa.
   - taskDescPtr: Ponteiro para o descritor de tarefa a ser inserido na fila.
*/
void enqueueTaskDescriptor(TaskDescriptorQueue* queue, TaskDescriptor* taskDescPtr) {
    TaskDescriptorNode* newNode = (TaskDescriptorNode*)malloc(sizeof(TaskDescriptorNode));
    newNode->taskDescriptorPtr = taskDescPtr;
    newNode->next = NULL;

    if (isTaskDescriptorQueueEmpty(queue)) {
        queue->front = queue->rear = newNode;
    } 
    else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}//enqueueTaskDescriptor()

/* Remove e retorna o descritor de tarefa no início da fila de descritores de tarefa.
   Se a fila estiver vazia, retorna NULL, indicando que a fila está sem elementos.
   Caso contrário, remove o primeiro elemento da fila, atualizando o ponteiro 'front'.
   Parâmetros:
   - queue: Ponteiro para a fila de descritores de tarefa.
   Retorno:
   - Ponteiro para o descritor de tarefa removido, ou NULL se a fila estiver vazia.
*/
TaskDescriptor* dequeueTaskDescriptor(TaskDescriptorQueue* queue) {
    if (isTaskDescriptorQueueEmpty(queue)) {
        return NULL;
    }

    TaskDescriptorNode* temp = queue->front;
    queue->front = queue->front->next;
    TaskDescriptor* result = temp->taskDescriptorPtr;
    free(temp);

    return result;
}//dequeueTaskDescriptor()

/* Destroi a fila de descritores de tarefa liberando toda a memória alocada.
   Remove cada descritor de tarefa da fila usando a função 'dequeueTaskDescriptor()'
   até que a fila esteja vazia e, em seguida, libera a memória da própria fila.
   Parâmetros:
   - queue: Ponteiro para a fila de descritores de tarefa a ser destruída.
*/
void destroyTaskDescriptorQueue(TaskDescriptorQueue* queue) {
    while (!isTaskDescriptorQueueEmpty(queue)) {
        dequeueTaskDescriptor(queue);
    }
    free(queue);
}//destroyTaskDescriptorQueue()

/* Verifica se todas as tarefas foram concluídas.
   Percorre o vetor de descritores de tarefa verificando o status de cada uma.
   Parâmetros:
   - tasks: Array de descritores de tarefa.
   - numberOfTasks: Número total de tarefas no array.
   Retorno:
   - TRUE se todas as tarefas estiverem marcadas como 'FINISHED', FALSE caso contrário.
*/
boolean allTasksFinished(TaskDescriptor tasks[], int numberOfTasks) {
    for (int i = 0; i < numberOfTasks; i++) {
        if (tasks[i].status !=  FINISHED) {
            return FALSE;
        }
    } 
    return TRUE;
}//allTasksFinished()

/* Simula a leitura do disco por uma tarefa.
   Atualiza o status da tarefa para 'SUSPENDED' e adiciona o tempo de E/S ao tempo total de entrada/saída da tarefa.
   Parâmetros:
   - taskDescriptor: Ponteiro para o descritor de tarefa a ser modificado.
*/
void readDisk(TaskDescriptor* taskDescriptor) {
    taskDescriptor->status = SUSPENDED;
    taskDescriptor->suspendedTime += SUSPENDED_TIME;
    taskDescriptor->inputOutputTime += SUSPENDED_TIME;
}//readDisk()

/* Imprime os descritores das tarefas ativas.
   Itera sobre as tarefas verificando se foram abortadas. Se não foram abortadas,
   imprime o descritor da tarefa usando a função 'printTaskDescriptor'.
   Parâmetros:
   - tasks: Array de descritores de tarefa.
   - numberOfTasks: Número total de tarefas no array.
   - roundRobin: Estrutura RoundRobin contendo informações do sistema.
*/
void printTasks(TaskDescriptor tasks[], int numberOfTasks, RoundRobin roundRobin) {
    for (int i = 0; i < numberOfTasks; i++) {
        if (tasks[i].aborted == FALSE) {
            printTaskDescriptor(tasks[i], roundRobin);
        }
    }
}//printTasks()

/* Calcula a taxa de ocupação da CPU de uma tarefa.
   Parâmetros:
   - taskDesc: Descritor de tarefa (TaskDescriptor).
   - roundRobin: Estrutura RoundRobin contendo informações do sistema.
   Retorno:
   - Taxa de ocupação da CPU (float).
*/
float calculateCPURate(TaskDescriptor taskDesc, RoundRobin roundRobin) {
    return ((float)taskDesc.cpuTime / roundRobin.totalCPUClocks) * 100;
}//calculateCPURate()

/* Calcula a taxa de ocupação do disco de uma tarefa.
   Parâmetros:
   - taskDesc: Descritor de tarefa (TaskDescriptor).
   - roundRobin: Estrutura RoundRobin contendo informações do sistema.
   Retorno:
   - Taxa de ocupação de disco (float).
*/
float calculateDiskRate(TaskDescriptor taskDesc, RoundRobin roundRobin) {
    return ((float)taskDesc.inputOutputTime / roundRobin.totalOutputTime) * 100;
}//calculateDiskRate()

/* Conta o número de tarefas executadas com sucesso.
   Itera sobre as tarefas verificando se foram abortadas. Se uma tarefa não foi abortada,
   incrementa o contador de tarefas executadas com sucesso.
   Parâmetros:
   - tasks: Array de descritores de tarefa.
   - numberOfTasks: Número total de tarefas no array.
   Retorno:
   - Número de tarefas executadas com sucesso.
*/
int numberOfTasksPerformedSuccessfully(TaskDescriptor tasks[], int numberOfTasks) {
    int successfully = 0;
    for (int i = 0; i < numberOfTasks; i++) {
        if (tasks[i].aborted == FALSE) {
            successfully++;
        }
    }
    return successfully;
}//numberOfTasksPerformedSuccessfully()

/* Imprime informações do Round-Robin.
   Esta função imprime métricas do algoritmo Round-Robin, como o tempo médio de execução e o tempo médio de espera.
   Utiliza o número de tarefas executadas com sucesso para calcular esses valores.
   Parâmetros:
   - roundRobin: Estrutura RoundRobin contendo informações do sistema.
   - numberOfTasksPerformedSuccessfully: Número de tarefas executadas com sucesso.
*/
void printRoundRobin(RoundRobin roundRobin, int numberOfTasksPerformedSuccessfully) {
    printf("\n\n- Round-Robin\n");
    printf("\t\tTempo médio de execução = %.2f s\n", (float) roundRobin.totalCPUClocks / numberOfTasksPerformedSuccessfully);
    printf("\t\tTempo médio de espera = %.2f s\n", (float) roundRobin.waitTime / numberOfTasksPerformedSuccessfully);
}//printRoundRobin()

/* Imprime as informações de memória de uma variável da tarefa.
   Parâmetros:
   - var: Variável (Variable) da tarefa.
*/
void printVariableMemoryInfo(Variable var) {
    printf("\t\tEndereço Lógicos = %u a %u ( %u : %u a %u : %u )\n", var.logicalMemory.logicalInitialByte, var.logicalMemory.logicalFinalByte,
        var.logicalMemory.logicalInitialByte / LOGICAL_PHYSICAL_PAGE_SIZE, var.logicalMemory.logicalInitialByte % LOGICAL_PHYSICAL_PAGE_SIZE,
        var.logicalMemory.logicalFinalByte / LOGICAL_PHYSICAL_PAGE_SIZE, var.logicalMemory.logicalFinalByte % LOGICAL_PHYSICAL_PAGE_SIZE);
    printf("\t\tEndereço Físicos = %u a %u ( %u : %u a %u : %u )\n", var.physicalMemory.physicalInitialByte, var.physicalMemory.physicalFinalByte,
        var.physicalMemory.physicalInitialByte / LOGICAL_PHYSICAL_PAGE_SIZE, var.physicalMemory.physicalInitialByte % LOGICAL_PHYSICAL_PAGE_SIZE, 
        var.physicalMemory.physicalFinalByte / LOGICAL_PHYSICAL_PAGE_SIZE, var.physicalMemory.physicalFinalByte % LOGICAL_PHYSICAL_PAGE_SIZE);
}//printVariableMemoryInfo()

/* Calcula e imprime as informações da tabela de páginas de uma tarefa.
   Parâmetros:
   - taskDesc: Descritor de tarefa (TaskDescriptor).
*/
void printPageTableInfo(TaskDescriptor taskDesc) {
    for (int i = 0; i < taskDesc.pagination.finalPage; ++i) {
        printf("\t\tPL %d (%u a %u) --> PF %d (%u a %u)\n", i, i * LOGICAL_PHYSICAL_PAGE_SIZE, (i + 1) * LOGICAL_PHYSICAL_PAGE_SIZE - 1, 
        taskDesc.pagination.initialBytesAllocated / LOGICAL_PHYSICAL_PAGE_SIZE + i,
        ((taskDesc.pagination.initialBytesAllocated / LOGICAL_PHYSICAL_PAGE_SIZE) + i) * LOGICAL_PHYSICAL_PAGE_SIZE, 
        ((taskDesc.pagination.initialBytesAllocated / LOGICAL_PHYSICAL_PAGE_SIZE) + i) * LOGICAL_PHYSICAL_PAGE_SIZE + LOGICAL_PHYSICAL_PAGE_SIZE - 1);
    }
}//printPageTableInfo()

/* Imprime todas as informações de uma tarefa.
   Parâmetros:
   - taskDesc: Descritor de tarefa (TaskDescriptor).
   - roundRobin: Estrutura RoundRobin contendo informações do sistema.
*/
void printTaskDescriptor(TaskDescriptor taskDesc, RoundRobin roundRobin) {
    printf("\n\n- Tarefa: %s\n", taskDesc.task.nameOfTask);
    printf("\t- CPU e Disco\n");
    printf("\t\tTempo de CPU = %hu ut\n", taskDesc.cpuTime);
    printf("\t\tTempo de E/S = %hu ut\n", taskDesc.inputOutputTime);
    printf("\t\tTaxa de ocupação da CPU = %.2f%%\n", calculateCPURate(taskDesc, roundRobin));
    printf("\t\tTaxa de ocupação do disco = %.2f%%\n", calculateDiskRate(taskDesc, roundRobin));
    printf("\t- Memória\n");
    printf("\t\tNúmero de páginas lógicas = %u\n", taskDesc.pagination.finalPage );

    for (int i = 0; i < taskDesc.quantityVariables; ++i) {
        Variable var = taskDesc.variable[i];
        printf("\n\t\t- %s\n", var.name);
        printVariableMemoryInfo(var);
    }

    printTaskMemoryAccesses(taskDesc);

    printf("\n\t\t- Tabela de Páginas\n");
    printPageTableInfo(taskDesc);
}//printTaskDescriptor()

/* Atualiza a memória lógica de uma variável de uma tarefa.
   Esta função atualiza os endereços lógicos de uma variável da tarefa com base no valor alocado.
   Calcula os endereços lógicos inicial e final, bem como atualiza a página final de paginação da tarefa.
   Parâmetros:
   - taskDescriptor: Ponteiro para o descritor de tarefa (TaskDescriptor).
   - index: Índice da variável no array de variáveis da tarefa.
   - value: Valor alocado para a variável.
*/
void updateLogicalMemory(TaskDescriptor* taskDescriptor, int index, int value) {
    taskDescriptor->variable[index].logicalMemory.logicalInitialByte = taskDescriptor->pagination.bytesAllocated + 1;
    taskDescriptor->variable[index].logicalMemory.logicalFinalByte = taskDescriptor->pagination.bytesAllocated + value;
    taskDescriptor->pagination.bytesAllocated += value;
    taskDescriptor->pagination.finalPage = roundingNumber((float) taskDescriptor->pagination.bytesAllocated / LOGICAL_PHYSICAL_PAGE_SIZE);
}//updateLogicalMemory()

/* Atualiza a memória física de uma variável de uma tarefa.
   Esta função atualiza os endereços físicos de uma variável da tarefa com base no valor alocado.
   Calcula os endereços físicos inicial e final da variável.
   Parâmetros:
   - taskDescriptor: Ponteiro para o descritor de tarefa (TaskDescriptor).
   - index: Índice da variável no array de variáveis da tarefa.
   - value: Valor alocado para a variável.
*/
void updatePhysicalMemory(TaskDescriptor* taskDescriptor, int index, int value) {
    taskDescriptor->variable[index].physicalMemory.physicalInitialByte = taskDescriptor->pagination.physicalBytesAllocated + 1;
    taskDescriptor->variable[index].physicalMemory.physicalFinalByte = taskDescriptor->pagination.physicalBytesAllocated + value;
    taskDescriptor->pagination.physicalBytesAllocated += value;
}//updatePhysicalMemory()

/* Executa a instrução 'new' para simular a alocação de variáveis para uma tarefa.
   Esta função analisa a instrução, identifica a variável e seu valor alocado.
   Em seguida, verifica se a variável já foi declarada. Se não, aloca a variável na memória lógica e física da tarefa.
   Parâmetros:
   - instruction: Instrução a ser executada.
   - taskDescriptor: Ponteiro para o descritor de tarefa (TaskDescriptor).
   - roundRobin: Ponteiro para a estrutura RoundRobin contendo informações do sistema.
   Retorno:
   - TRUE se a alocação da variável for bem-sucedida, FALSE caso contrário.
*/
boolean new(String instruction, TaskDescriptor* taskDescriptor, RoundRobin* roundRobin) {
    String identifier;
    int value;
    sscanf(instruction, "%s new %d", identifier, &value);
    for (int i = 0; i < MAXIMUN_NUMBER_OF_VARIABLES; i++) {
        // Verifica se já tem a variável alocada
        if (strcmp(identifier, taskDescriptor->variable[i].name) == 0){
            printf(IDENTIFY_ALREADY_DECLARED_ERROR, taskDescriptor->task.nameOfTask, identifier);
            finishTask(taskDescriptor, TRUE, roundRobin);
            return FALSE;
        }
        
        if (strlen(taskDescriptor->variable[i].name) == 0) {
            taskDescriptor->variable[i].value = value;
            strcpy(taskDescriptor->variable[i].name, identifier);
            updateLogicalMemory(taskDescriptor, i, taskDescriptor->variable[i].value);
            updatePhysicalMemory(taskDescriptor, i, taskDescriptor->variable[i].value);

            if (taskDescriptor->pagination.bytesAllocated > LARGEST_LOGICAL_MEMORY_SIZE) {
                printf(ALLOCATION_SPACE_ERROR, taskDescriptor->task.nameOfTask, LARGEST_LOGICAL_MEMORY_SIZE);
                finishTask(taskDescriptor, TRUE, roundRobin);
                return FALSE;
            }
            taskDescriptor->quantityVariables++;
            return TRUE;
        }
    }
    finishTask(taskDescriptor, TRUE, roundRobin);
    return FALSE;
}//new()

/* Imprime os acessos à memória das variáveis de uma tarefa.
   Esta função percorre os acessos à memória registrados na estrutura da tarefa e imprime os endereços lógicos e físicos
   para cada acesso.
   Parâmetros:
   - taskDesc: Descritor da tarefa (TaskDescriptor) contendo os acessos à memória.
*/
void printTaskMemoryAccesses(TaskDescriptor taskDesc) {
      for (int i = 0; i < taskDesc.quantityAccesses; ++i) {
        int logicalPage = (taskDesc.accesses[i].logicalMemory.logicalInitialByte + taskDesc.accesses[i].value) / LOGICAL_PHYSICAL_PAGE_SIZE; 
        int physicalPage = (taskDesc.accesses[i].physicalMemory.physicalInitialByte + taskDesc.accesses[i].value) / LOGICAL_PHYSICAL_PAGE_SIZE;

        int logicalByte = (taskDesc.accesses[i].logicalMemory.logicalInitialByte + taskDesc.accesses[i].value) % LOGICAL_PHYSICAL_PAGE_SIZE; 
        int physicalByte = (taskDesc.accesses[i].physicalMemory.physicalInitialByte + taskDesc.accesses[i].value) % LOGICAL_PHYSICAL_PAGE_SIZE;

        printf("\n\t\t%s[%d] -> Endereço Lógico = %d : %d\n", taskDesc.accesses[i].name, taskDesc.accesses[i].value, logicalPage, logicalByte);
        printf("\t\t-> Endereço Físico = %d : %d\n", physicalPage, physicalByte);
    }
}//printTaskMemoryAccesses()

/* Simula um acesso à memória de uma variável.
   Esta função analisa a instrução fornecida para acessar a memória de uma variável específica da tarefa.
   Se a variável for encontrada, o acesso à memória é registrado no descritor da tarefa.
   Parâmetros:
   - instruction: Instrução de acesso à memória.
   - taskDescriptor: Ponteiro para o descritor da tarefa (TaskDescriptor).
   - roundRobin: Ponteiro para a estrutura RoundRobin contendo informações do sistema.
   Retorno:
   - TRUE se o acesso à memória foi realizado com sucesso; FALSE caso contrário.
*/
boolean memoryAccess(String instruction, TaskDescriptor* taskDescriptor, RoundRobin* roundRobin) {
    String identifier;
    unsigned int value;
    sscanf(instruction, "%[^[][%d]", identifier, &value);
    for (int i = 0; i < taskDescriptor->quantityVariables; i++) {
        if (strcmp(taskDescriptor->variable[i].name, identifier) == 0) {
            if (value >= taskDescriptor->variable[i].value) {
                printf(MEMORY_ACCESS_ERROR, taskDescriptor->task.nameOfTask, identifier, value);
                finishTask(taskDescriptor, TRUE, roundRobin);
                return FALSE;
            } else {
                taskDescriptor->accesses[taskDescriptor->quantityAccesses] = taskDescriptor->variable[i];
                taskDescriptor->accesses[taskDescriptor->quantityAccesses++].value = value;
                return TRUE;
            }
        }
    }
    printf(UNDECLARED_IDENTIFIER_ERROR, taskDescriptor->task.nameOfTask, identifier);
    finishTask(taskDescriptor, TRUE, roundRobin);
    return FALSE;
}//memoryAccess()

/* Finaliza uma tarefa.
   Esta função atualiza o status de uma tarefa para finalizada e configura a marca de abortada, se aplicável.
   Atualiza também o tempo de término, fecha o arquivo associado à tarefa e ajusta as métricas globais do sistema.
   Parâmetros:
   - taskDescriptor: Ponteiro para o descritor da tarefa (TaskDescriptor).
   - aborted: Indica se a tarefa foi abortada (TRUE) ou concluída com sucesso (FALSE).
   - roundRobin: Ponteiro para a estrutura RoundRobin contendo informações do sistema.
*/
void finishTask(TaskDescriptor* taskDescriptor, boolean aborted, RoundRobin* roundRobin) {
    taskDescriptor->aborted = aborted;
    taskDescriptor->status = FINISHED;
    taskDescriptor->endTime = roundRobin->totalCPUClocks;
    fclose(taskDescriptor->task.taskFile);
    if (aborted) {
        roundRobin->totalCPUClocks -= taskDescriptor->cpuTime;
        roundRobin->totalOutputTime -= taskDescriptor->inputOutputTime;
    }
    int number = taskDescriptor->endTime - taskDescriptor->cpuTime - taskDescriptor->startTime;
    if(number > 0){
        roundRobin->waitTime += taskDescriptor->endTime - taskDescriptor->cpuTime - taskDescriptor->startTime;
    }
}//finishTask()

/* Verifica e atualiza as tarefas suspensas.
   Esta função percorre a lista de tarefas e verifica se alguma está suspensa.
   Se uma tarefa estiver suspensa, ela decrementa o tempo de suspensão pelo número de unidades de tempo especificado.
   Se o tempo de suspensão chegar a zero, a função atualiza o status da tarefa para READY e a coloca na fila de tarefas prontas.
   Parâmetros:
   - queue: Ponteiro para a fila de descritores de tarefas (TaskDescriptorQueue).
   - tasks: Array de descritores de tarefas (TaskDescriptor).
   - numberOfTasks: Número total de tarefas no array.
   - timeUnits: Unidades de tempo a serem subtraídas do tempo de suspensão das tarefas.
*/
void checkAndUpdateSuspendedTasks(TaskDescriptorQueue* queue, TaskDescriptor tasks[], int numberOfTasks, unsigned timeUnits) {
    for (int i = 0; i < numberOfTasks; i++) {
        if (tasks[i].status == SUSPENDED) {
            tasks[i].suspendedTime -= timeUnits;
            if (tasks[i].suspendedTime == 0) {
                tasks[i].status = READY;
                enqueueTaskDescriptor(queue, &tasks[i]);
            }
        }
    }
}//checkAndUpdateSuspendedTasks()

/* Arredonda um número float para o inteiro mais próximo.
   Esta função recebe um número em ponto flutuante e retorna o valor arredondado para o inteiro mais próximo.
   Parâmetros:
   - number: Número em ponto flutuante a ser arredondado.
   Retorno:
   - Valor arredondado para o inteiro mais próximo (int).
*/
int roundingNumber(float number) {
    int integerPart = (int)number; 

    if (number - integerPart > 0) {
        return integerPart + 1; 
    } 
    else {
        return integerPart; 
    }
}//roundingNumber()

/* Atualiza a paginação de uma tarefa com base nos bytes alocados.
   Esta função atualiza os valores de paginação de uma tarefa com base na quantidade de bytes alocados.
   Parâmetros:
   - taskDescriptor: Ponteiro para o descritor de tarefa (TaskDescriptor) a ser atualizado.
   - bytes: Quantidade de bytes alocados.
*/
void updatePagination(TaskDescriptor* taskDescriptor, unsigned int bytes) {
    taskDescriptor->pagination.bytesAllocated = bytes;
    taskDescriptor->pagination.finalPage = roundingNumber((float) taskDescriptor->pagination.bytesAllocated / LOGICAL_PHYSICAL_PAGE_SIZE);
    taskDescriptor->pagination.bytesAllocated = taskDescriptor->pagination.finalPage * LOGICAL_PHYSICAL_PAGE_SIZE - 1;
    taskDescriptor->pagination.physicalBytesAllocated += taskDescriptor->pagination.bytesAllocated;
}//updatePagination()

/* Processa um cabeçalho de instrução para atualizar a paginação de uma tarefa.
   Esta função processa o cabeçalho de uma instrução para atualizar os valores de paginação
   de uma tarefa com base nos bytes especificados no cabeçalho.
   Parâmetros:
   - instruction: String contendo a instrução do cabeçalho.
   - taskDescriptor: Ponteiro para o descritor de tarefa (TaskDescriptor) a ser atualizado.
   Retorno:
   - Retorna TRUE se a operação for bem-sucedida; FALSE, caso contrário.
*/
boolean header(String instruction, TaskDescriptor* taskDescriptor, RoundRobin* roundRobin) {
    unsigned int bytes;
    sscanf(instruction, "#%*[^=]=%u", &bytes);

    if (taskDescriptor->pagination.finalPage > LARGEST_LOGICAL_MEMORY_SIZE) {
        printf(ALLOCATION_SPACE_ERROR, taskDescriptor->task.nameOfTask, LARGEST_LOGICAL_MEMORY_SIZE);
        finishTask(taskDescriptor, TRUE, roundRobin);
        return FALSE;
    }
    updatePagination(taskDescriptor, bytes);
    
    return TRUE;
}//header()

/* Determina o tipo de instrução baseado em padrões regex.
   Esta função verifica o tipo de instrução com base em padrões regex e retorna
   um enum correspondente ao tipo de instrução encontrado.
   Parâmetros:
   - instruction: String contendo a instrução a ser verificada.
   Retorno:
   - Retorna um enum do tipo Instruction correspondente ao tipo de instrução encontrado,
     ou -1 caso não corresponda a nenhum padrão conhecido.
*/
Instruction determineInstructionType(String instruction) {
    if (matchRegex(instruction, INSTRUCTION_HEADER_REGEX)) {
        return HEADER;
    } 
    else if (matchRegex(instruction, INSTRUCTION_NEW_REGEX)) {
        return NEW;
    } 
    else if (matchRegex(instruction, INSTRUCTION_IDEX_REGEX)) {
        return MEMORY_ACCESS;
    } 
    else if (matchRegex(instruction, INSTRUCTION_READ_DISK_REGEX)) {
        return READ_DISK;
    } 
    else {
        return -1; 
    }
}//determineInstructionType()

/* Executa as instruções de uma tarefa dentro do quantum definido.
   Esta função executa as instruções de uma tarefa dentro do quantum definido pelo RoundRobin.
   Parâmetros:
   - queue: Ponteiro para a fila de descritores de tarefa.
   - roundRobin: Ponteiro para a estrutura RoundRobin contendo informações do sistema.
   - taskDescriptor: Ponteiro para o descritor da tarefa em execução.
   - tasks: Array de descritores de tarefas.
   - numberOfTasks: Número total de tarefas.
   Retorno:
   - Retorna o valor do contador de preempção após a execução das instruções.
*/
int executeInstruction(TaskDescriptorQueue* queue, RoundRobin* roundRobin, TaskDescriptor* taskDescriptor, TaskDescriptor tasks[], int numberOfTasks) {
    while (roundRobin->preemptionTimeCounter <= QUANTUM) {
        if (taskDescriptor != NULL && taskDescriptor->status == RUNNING) {
            String instruction;
            if (fgets(instruction, sizeof(instruction), taskDescriptor->task.taskFile) != NULL) {
                roundRobin->totalCPUClocks += UT;
                taskDescriptor->cpuTime += UT;

                Instruction instructionType = determineInstructionType(instruction);

                switch (instructionType) {
                    case HEADER:
                        // Decrementando os contatores, pois o header não deve ser processado no tempo final.
                        roundRobin->preemptionTimeCounter--;
                        roundRobin->totalCPUClocks-= UT;
                        taskDescriptor->cpuTime-= UT;
                        header(instruction, taskDescriptor, roundRobin);
                        break;
                    case NEW:
                        new(instruction, taskDescriptor, roundRobin);
                        break;
                    case MEMORY_ACCESS:
                        memoryAccess(instruction, taskDescriptor, roundRobin);
                        break;
                    case READ_DISK:
                        readDisk(taskDescriptor);
                        roundRobin->totalOutputTime += SUSPENDED_TIME;
                        break;
                    default:
                        finishTask(taskDescriptor, FALSE, roundRobin);
                        break;
                }
            } 
            else {
                finishTask(taskDescriptor, FALSE, roundRobin);
                break;
            }
        }
        roundRobin->preemptionTimeCounter += UT;
    }
    return roundRobin->preemptionTimeCounter;
}//executeInstruction()

/* Inicializa a estrutura RoundRobin com valores iniciais.
   Esta função atribui valores iniciais aos campos da estrutura RoundRobin.
   Parâmetros:
   - roundRobin: Ponteiro para a estrutura RoundRobin a ser inicializada.
*/
void initializeRoundRobin(RoundRobin* roundRobin) {
    roundRobin->totalCPUClocks = 0;
    roundRobin->preemptionTimeCounter = 1;
    roundRobin->waitTime = 0;
}//initializeRoundRobin()

/* Inicializa a fila de descritores de tarefa.
   Esta função inicializa a fila de descritores de tarefa, enfileirando as tarefas prontas e não abortadas.
   Parâmetros:
   - taskDescriptorQueue: Ponteiro para a fila de descritores de tarefa a ser inicializada.
   - tasks: Array de descritores de tarefa.
   - numberOfTasks: Número total de tarefas no array.
*/
void initializeTaskQueue(TaskDescriptorQueue* taskDescriptorQueue, TaskDescriptor tasks[], int numberOfTasks) {
    for (int i = 0; i < numberOfTasks; i++) {
        if (tasks[i].status == READY && tasks[i].aborted == FALSE) {
            tasks[i].startTime = i;
            tasks[i].pagination.initialBytesAllocated = RESERVED_PROGRAM_MEMORY_SIZE + i * LARGEST_LOGICAL_MEMORY_SIZE;
            tasks[i].pagination.physicalBytesAllocated = tasks[i].pagination.initialBytesAllocated;
            enqueueTaskDescriptor(taskDescriptorQueue, &tasks[i]);
        }
    }
}//initializeTaskQueue()

/* Executa o escalonamento e execução das tarefas.
   Esta função executa o escalonamento e a execução das tarefas enquanto houver tarefas a serem processadas.
   Parâmetros:
   - taskDescriptorQueue: Ponteiro para a fila de descritores de tarefa.
   - roundRobin: Estrutura de informações do Round-Robin.
   - tasks: Array de descritores de tarefa.
   - numberOfTasks: Número total de tarefas no array.
*/
void runTasks(TaskDescriptorQueue* taskDescriptorQueue, RoundRobin* roundRobin, TaskDescriptor tasks[], int numberOfTasks) {
    while (TRUE) {
        // O loop so será finalizado quando todas as tarefas estiverem com estado igual a FINISH.
        if (allTasksFinished(tasks, numberOfTasks)) {
            destroyTaskDescriptorQueue(taskDescriptorQueue);
            break;
        }
        
        // A fila contém apenas tarefas com estados READY.
        TaskDescriptor* taskRunningPtr = dequeueTaskDescriptor(taskDescriptorQueue);

        // Alterando o estado para RUNNING, pois a tarefa será executada.
        if (taskRunningPtr != NULL) {
            taskRunningPtr->status = RUNNING;
        }
        
        // Executa a tarefa que foi retirada da fila.
        executeInstruction(taskDescriptorQueue, roundRobin, taskRunningPtr, tasks, numberOfTasks);
        
        if (taskRunningPtr != NULL && taskRunningPtr->status == RUNNING) {
            // Caso a tarefa não sejá suspensa durante sua excecução, seu estado será mudada para READY e ela será adcionada a fila.
            taskRunningPtr->status = READY;
            enqueueTaskDescriptor(taskDescriptorQueue, taskRunningPtr);
        }
        
        //Decrementando o tempo de suspensão das tarefas com estado SUSPENDED.
        checkAndUpdateSuspendedTasks(taskDescriptorQueue, tasks, numberOfTasks, roundRobin->preemptionTimeCounter);
        
        // Resetando o tempo de preempção.
        roundRobin->preemptionTimeCounter = UT;
    }
}//runTasks()

/* Agenda e executa as tarefas usando o algoritmo Round-Robin.
   Esta função cria uma estrutura RoundRobin, uma fila de descritores de tarefa e coordena a execução das tarefas
   usando o algoritmo Round-Robin até que todas as tarefas tenham sido executadas.
   Parâmetros:
   - tasks: Array de descritores de tarefa.
   - numberOfTasks: Número total de tarefas no array.
   Retorno:
   - Estrutura RoundRobin com informações de execução das tarefas.
*/
RoundRobin scheduleTasks(TaskDescriptor tasks[], int numberOfTasks) {
    RoundRobin roundRobin;
    TaskDescriptorQueue* taskDescriptorQueue = createTaskDescriptorQueue(); 

    initializeRoundRobin(&roundRobin);
    initializeTaskQueue(taskDescriptorQueue, tasks, numberOfTasks);
    runTasks(taskDescriptorQueue, &roundRobin, tasks, numberOfTasks);
    return roundRobin;
}//scheduleTasks()

/* Verifica se uma string corresponde a um padrão de expressão regular.
   Esta função utiliza a biblioteca POSIX para compilar e executar uma expressão regular
   e verifica se a string fornecida corresponde ao padrão especificado.
   Parâmetros:
   - string: A string a ser verificada.
   - pattern: O padrão de expressão regular a ser comparado com a string.
   Retorno:
   - TRUE se a string corresponde ao padrão, FALSE caso contrário.
*/
boolean matchRegex(String string, const char *pattern) {
    regex_t regex;
    int result;

    result = regcomp(&regex, pattern, REG_EXTENDED);
    if (result != 0) {
        return FALSE;
    }
    
    result = regexec(&regex, string, 0, NULL, 0);
    regfree(&regex);

    if (result == 0) {
        return TRUE;
    } 
    else {
        return FALSE; 
    }
}//matchRegex()

/* Valida o número de argumentos fornecidos ao programa.
   Esta função verifica se o número de argumentos está dentro do intervalo esperado
   para a execução do programa, com base na quantidade de tarefas previamente definida.
   Parâmetros:
   - numberOfArguments: O número de argumentos fornecidos ao programa.
   Retorno:
   - TRUE se o número de argumentos estiver dentro do intervalo esperado, FALSE caso contrário.
*/
boolean validateNumberOfArguments(int numberOfArguments) {
    if (numberOfArguments < 2 || numberOfArguments > NUMBER_OF_TASKS + 1) {
        return FALSE;
    }
    return TRUE;
}//validateNumberOfArguments()

/* Inicializa um descritor de tarefa.
   Esta função inicializa um descritor de tarefa com valores padrão e abre o arquivo de tarefa associado.
   Parâmetros:
   - descriptor: O descritor de tarefa a ser inicializado.
   - taskName: O nome da tarefa para identificação.
*/
void initializeTaskDescriptor(TaskDescriptor* descriptor, String taskName) {
    // Inicializa todos os atributos com 0
    memset(descriptor, 0, sizeof(TaskDescriptor));

    strcpy(descriptor->task.nameOfTask, taskName);
    FileName fullFileName;
    snprintf(fullFileName, FILE_NAME_SIZE, "%s%s", taskName, FILE_EXTENSION);
    descriptor->task.taskFile = fopen(fullFileName, FILE_OPENING_OPTION);
    descriptor->status = READY;
    descriptor->aborted = FALSE;

    // Inicializa as variáveis da tarefa com 0
    for (int i = 0; i < MAXIMUN_NUMBER_OF_VARIABLES; ++i) {
        memset(descriptor->variable[i].name, 0, STRING_DEFAULT_SIZE);
        descriptor->variable[i].value = 0;
    }
}//initializeTaskDescriptor()

/* Valida um arquivo de tarefa.
   Esta função verifica se um arquivo de tarefa possui um formato válido, verificando suas instruções.
   Parâmetros:
   - fileName: O nome do arquivo de tarefa a ser validado.
   Retorno:
   - TRUE se o arquivo possui um formato válido, FALSE caso contrário.
*/
boolean validateFile(FileName fileName) {
    FileName fullFileName;
    snprintf(fullFileName, FILE_NAME_SIZE, "%s%s", fileName, FILE_EXTENSION);

    FILE *file = fopen(fullFileName, FILE_OPENING_OPTION);

    if (file == NULL) {
        printf(FILE_OPEN_ERROR, fileName);
        return FALSE;
    }
    String line;

    if (fgets(line, sizeof(line), file) != NULL) {
        if (!matchRegex(line, INSTRUCTION_HEADER_REGEX)) {
            fclose(file);
            return FALSE;
        }
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        if (!matchRegex(line, INSTRUCTION_NEW_REGEX) && !matchRegex(line, INSTRUCTION_IDEX_REGEX) && 
            !matchRegex(line, INSTRUCTION_READ_DISK_REGEX)) {
            fclose(file);
            return FALSE;
        }
    }
    fclose(file);
    return TRUE;
}//validateFile()

/* Função principal para execução do escalonamento de tarefas e imprimir os relatórios.
   Parâmetros:
   - numberOfTasks: Número de tarefas a serem escalonadas.
   - tasks: Array de strings contendo os nomes dos arquivos de tarefa.
   Retorno:
   - EXIT_SUCCESS se a execução for bem-sucedida, EXIT_FAILURE caso contrário.
*/
int tsmm(int numberOfTasks, char *tasks[]) {
    setlocale(LC_ALL, LOCALE);
    if (!validateNumberOfArguments(numberOfTasks)) {
        perror(INVALID_ARGUMENTS_ERROR);
        exit(EXIT_FAILURE);
    }
    RoundRobin roundRobin;
    initializeRoundRobin(&roundRobin);
    TaskDescriptor tasksDescriptions[numberOfTasks -1];
    for (int i = 1; i < numberOfTasks; i++) {
        if (!validateFile(tasks[i])) {
            printf(DIVERGET_INSTRUCTION_ERROR, tasks[i]);
            tasksDescriptions[i - 1].aborted = TRUE;
            tasksDescriptions[i - 1].status = FINISHED;
        }
        else {
            initializeTaskDescriptor(&tasksDescriptions[i - 1], tasks[i]);
        }
    }
    roundRobin = scheduleTasks(tasksDescriptions, numberOfTasks - 1);
    printRoundRobin(roundRobin, numberOfTasksPerformedSuccessfully(tasksDescriptions, numberOfTasks -1));
    printTasks(tasksDescriptions, numberOfTasks -1, roundRobin);
    return EXIT_SUCCESS;
}//tsmm()

int main(int argc, char *argv[]) {
    return tsmm(argc, argv);
}//main()
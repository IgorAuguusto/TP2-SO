#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <regex.h>
#include "tsmm.h"


TaskDescriptorQueue* createTaskDescriptorQueue() {
    TaskDescriptorQueue* queue = (TaskDescriptorQueue*)malloc(sizeof(TaskDescriptorQueue));
    queue->front = queue->rear = NULL;
    return queue;
}//createTaskDescriptorQueue()


int isTaskDescriptorQueueEmpty(TaskDescriptorQueue* queue) {
    return (queue->front == NULL);
}//isTaskDescriptorQueueEmpty()


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



void destroyTaskDescriptorQueue(TaskDescriptorQueue* queue) {
    while (!isTaskDescriptorQueueEmpty(queue)) {
        dequeueTaskDescriptor(queue);
    }
    free(queue);
}//destroyTaskDescriptorQueue()


boolean allTasksFinished(TaskDescriptor tasks[], int numberOfTasks) {
    for (int i = 0; i < numberOfTasks; i++) {
        if (tasks[i].status !=  FINISHED) {
            return FALSE;
        }
    } 
    return TRUE;
}

void readDisk(TaskDescriptor* taskDescriptor) {
    taskDescriptor->status = SUSPENDED;
    taskDescriptor->suspendedTime += SUSPENDED_TIME;
    taskDescriptor->inputOutputTime += SUSPENDED_TIME;
}

void printTasks(TaskDescriptor tasks[], int numberOfTasks, RoundRobin roundRobin) {
    for (int i = 0; i < numberOfTasks; i++) {
        if (tasks[i].aborted == FALSE) {
            printTaskDescriptor(tasks[i], roundRobin);
        }
    }
}

/* Calcula a taxa de ocupação da CPU de uma tarefa.
   Parâmetros:
   - taskDesc: Descritor de tarefa (TaskDescriptor).
   - roundRobin: Estrutura RoundRobin contendo informações do sistema.
   Retorno:
   - Taxa de ocupação da CPU (float).
*/
float calculateCPURate(TaskDescriptor taskDesc, RoundRobin roundRobin) {
    return ((float)taskDesc.cpuTime / roundRobin.totalCPUClocks) * 100;
}

/* Calcula a taxa de ocupação do disco de uma tarefa.
   Parâmetros:
   - taskDesc: Descritor de tarefa (TaskDescriptor).
   - roundRobin: Estrutura RoundRobin contendo informações do sistema.
*/
float calculateDiskRate(TaskDescriptor taskDesc, RoundRobin roundRobin) {
    return ((float)taskDesc.inputOutputTime / roundRobin.totalOutputTime) * 100;
}

int numberOfTasksPerformedSuccessfully(TaskDescriptor tasks[], int numberOfTasks) {
    int successfully = 0;
    for (int i = 0; i < numberOfTasks; i++) {
        if (tasks[i].aborted == FALSE) {
            successfully++;
        }
    }
    return successfully;
}

void printRoundRobin(RoundRobin roundRobin, int numberOfTasksPerformedSuccessfully) {
    printf("\n\n- Round-Robin\n");
    printf("\t\tTempo médio de execução = %.2f s\n", (float) roundRobin.totalCPUClocks / numberOfTasksPerformedSuccessfully);
    printf("\t\tTempo médio de espera = %.2f s\n", (float) roundRobin.waitTime / numberOfTasksPerformedSuccessfully);
}

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
}

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
}

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
}


void updateLogicalMemory(TaskDescriptor* taskDescriptor, int index, int value) {
    taskDescriptor->variable[index].logicalMemory.logicalInitialByte = taskDescriptor->pagination.bytesAllocated + 1;
    taskDescriptor->variable[index].logicalMemory.logicalFinalByte = taskDescriptor->pagination.bytesAllocated + value;
    taskDescriptor->pagination.bytesAllocated += value;
    taskDescriptor->pagination.finalPage = roundingNumber((float) taskDescriptor->pagination.bytesAllocated / LOGICAL_PHYSICAL_PAGE_SIZE);
}

void updatePhysicalMemory(TaskDescriptor* taskDescriptor, int index, int value) {
    taskDescriptor->variable[index].physicalMemory.physicalInitialByte = taskDescriptor->pagination.physicalBytesAllocated + 1;
    taskDescriptor->variable[index].physicalMemory.physicalFinalByte = taskDescriptor->pagination.physicalBytesAllocated + value;
    taskDescriptor->pagination.physicalBytesAllocated += value;
}

boolean new(String instruction, TaskDescriptor* taskDescriptor, RoundRobin* roundRobin) {
    String identifier;
    int value;
    sscanf(instruction, "%s new %d", identifier, &value);
    for (int i = 0; i < MAXIMUN_NUMBER_OF_VARIABLES; i++) {
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
}

void printTaskMemoryAccesses(TaskDescriptor taskDesc) {
      for (int i = 0; i < taskDesc.quantityAccesses; ++i) {
        int logicalPage = (taskDesc.accesses[i].logicalMemory.logicalInitialByte + taskDesc.accesses[i].value) / LOGICAL_PHYSICAL_PAGE_SIZE; 
        int physicalPage = (taskDesc.accesses[i].physicalMemory.physicalInitialByte + taskDesc.accesses[i].value) / LOGICAL_PHYSICAL_PAGE_SIZE;

        int logicalByte = (taskDesc.accesses[i].logicalMemory.logicalInitialByte + taskDesc.accesses[i].value) % LOGICAL_PHYSICAL_PAGE_SIZE; 
        int physicalByte = (taskDesc.accesses[i].physicalMemory.physicalInitialByte + taskDesc.accesses[i].value) % LOGICAL_PHYSICAL_PAGE_SIZE;

        printf("\n\t\t%s[%d] -> Endereço Lógico = %d : %d\n", taskDesc.accesses[i].name, taskDesc.accesses[i].value, logicalPage, logicalByte);
        printf("\t\t-> Endereço Físico = %d : %d\n", physicalPage, physicalByte);
    }
}



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
}

void finishTask(TaskDescriptor* taskDescriptor, boolean aborted, RoundRobin* roundRobin) {
    taskDescriptor->aborted = aborted;
    taskDescriptor->status = FINISHED;
    taskDescriptor->endTime = roundRobin->totalCPUClocks;
    fclose(taskDescriptor->task.taskFile);
    if (aborted) {
        roundRobin->totalCPUClocks -= taskDescriptor->cpuTime;
        roundRobin->totalOutputTime -= taskDescriptor->inputOutputTime;
    }
    else {
        roundRobin->waitTime += taskDescriptor->endTime - taskDescriptor->cpuTime - taskDescriptor->startTime;
    }
}

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
}

int roundingNumber(float number) {
    int integerPart = (int)number; 

    if (number - integerPart > 0) {
        return integerPart + 1; 
    } 
    else {
        return integerPart; 
    }
}

void updatePagination(TaskDescriptor* taskDescriptor, unsigned int bytes) {
    taskDescriptor->pagination.bytesAllocated = bytes;
    taskDescriptor->pagination.finalPage = roundingNumber((float) taskDescriptor->pagination.bytesAllocated / LOGICAL_PHYSICAL_PAGE_SIZE);
    taskDescriptor->pagination.bytesAllocated = taskDescriptor->pagination.finalPage * LOGICAL_PHYSICAL_PAGE_SIZE - 1;
    taskDescriptor->pagination.physicalBytesAllocated += taskDescriptor->pagination.bytesAllocated;
}

boolean header(String instruction, TaskDescriptor* taskDescriptor) {
    unsigned int bytes;
    sscanf(instruction, "#%*[^=]=%u", &bytes);
    
    updatePagination(taskDescriptor, bytes);

    if (taskDescriptor->pagination.finalPage > LARGEST_LOGICAL_MEMORY_SIZE) {
        printf(ALLOCATION_SPACE_ERROR, taskDescriptor->task.nameOfTask, LARGEST_LOGICAL_MEMORY_SIZE);
        finishTask(taskDescriptor, TRUE, FALSE);
        return FALSE;
    }
    return TRUE;
}

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
}


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
                        roundRobin->preemptionTimeCounter--;
                        roundRobin->totalCPUClocks-= UT;
                        taskDescriptor->cpuTime-= UT;
                        header(instruction, taskDescriptor);
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
}

void initializeRoundRobin(RoundRobin* roundRobin) {
    roundRobin->totalCPUClocks = 0;
    roundRobin->preemptionTimeCounter = 1;
    roundRobin->waitTime = 0;
}

void initializeTaskQueue(TaskDescriptorQueue* taskDescriptorQueue, TaskDescriptor tasks[], int numberOfTasks) {
    for (int i = 0; i < numberOfTasks; i++) {
        if (tasks[i].status == READY && tasks[i].aborted == FALSE) {
            tasks[i].startTime = i;
            tasks[i].pagination.initialBytesAllocated = RESERVED_PROGRAM_MEMORY_SIZE + i * LARGEST_LOGICAL_MEMORY_SIZE;
            tasks[i].pagination.physicalBytesAllocated = tasks[i].pagination.initialBytesAllocated;
            enqueueTaskDescriptor(taskDescriptorQueue, &tasks[i]);
        }
    }
}

void runTasks(TaskDescriptorQueue* taskDescriptorQueue, RoundRobin* roundRobin, TaskDescriptor tasks[], int numberOfTasks) {
    while (TRUE) {
        if (allTasksFinished(tasks, numberOfTasks)) {
            destroyTaskDescriptorQueue(taskDescriptorQueue);
            break;
        }
        
        TaskDescriptor* taskRunningPtr = dequeueTaskDescriptor(taskDescriptorQueue);

        if (taskRunningPtr != NULL) {
            taskRunningPtr->status = RUNNING;
        }
        
        executeInstruction(taskDescriptorQueue, roundRobin, taskRunningPtr, tasks, numberOfTasks);
        
        if (taskRunningPtr != NULL && taskRunningPtr->status == RUNNING) {
            taskRunningPtr->status = READY;
            enqueueTaskDescriptor(taskDescriptorQueue, taskRunningPtr);
        }
        
        checkAndUpdateSuspendedTasks(taskDescriptorQueue, tasks, numberOfTasks, roundRobin->preemptionTimeCounter);
        
        roundRobin->preemptionTimeCounter = 1;
    }
}

RoundRobin scheduleTasks(TaskDescriptor tasks[], int numberOfTasks) {
    RoundRobin roundRobin;
    TaskDescriptorQueue* taskDescriptorQueue = createTaskDescriptorQueue(); 

    initializeRoundRobin(&roundRobin);
    initializeTaskQueue(taskDescriptorQueue, tasks, numberOfTasks);
    runTasks(taskDescriptorQueue, &roundRobin, tasks, numberOfTasks);
    return roundRobin;
}


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

boolean validateNumberOfArguments(int numberOfArguments) {
    if (numberOfArguments < 2 || numberOfArguments > NUMBER_OF_TASKS + 1) {
        return FALSE;
    }
    return TRUE;
}//validateNumberOfArguments()

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
            return FALSE;
        }
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        if (!matchRegex(line, INSTRUCTION_NEW_REGEX) && !matchRegex(line, INSTRUCTION_IDEX_REGEX) && 
            !matchRegex(line, INSTRUCTION_READ_DISK_REGEX)) {
            return FALSE;
        }
    }
    fclose(file);
    return TRUE;
}//validateFile()

int tsmm(int numberOfTasks, char *tasks[]) {
    setlocale(LC_ALL, LOCALE);
    if (!validateNumberOfArguments(numberOfTasks)) {
        perror(INVALID_ARGUMENTS_ERROR);
        exit(EXIT_FAILURE);
    }

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
    RoundRobin roundRobin = scheduleTasks(tasksDescriptions, numberOfTasks - 1);
    printRoundRobin(roundRobin, numberOfTasksPerformedSuccessfully(tasksDescriptions, numberOfTasks));
    printTasks(tasksDescriptions, numberOfTasks, roundRobin);
    return EXIT_SUCCESS;
}//tsmm()

int main(int argc, char *argv[]) {
    return tsmm(argc, argv);
}//main()
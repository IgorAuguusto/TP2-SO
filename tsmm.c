#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "tsmm.h"

// Função para criar uma fila vazia
TaskDescriptorQueue* createTaskDescriptorQueue() {
    TaskDescriptorQueue* queue = (TaskDescriptorQueue*)malloc(sizeof(TaskDescriptorQueue));
    queue->front = queue->rear = NULL;
    return queue;
}//createTaskDescriptorQueue()

// Função para verificar se a fila está vazia
int isTaskDescriptorQueueEmpty(TaskDescriptorQueue* queue) {
    return (queue->front == NULL);
}//isTaskDescriptorQueueEmpty()

// Função para enfileirar um elemento na fila
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

// Função para desenfileirar um elemento da fila
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


// Função para liberar a memória alocada para a fila
void destroyTaskDescriptorQueue(TaskDescriptorQueue* queue) {
    while (!isTaskDescriptorQueueEmpty(queue)) {
        dequeueTaskDescriptor(queue);
    }
    free(queue);
}//destroyTaskDescriptorQueue()

// Função para verificar se todas as tarefas têm o status FINISHED
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

void printTaskDescriptor(TaskDescriptor taskDesc) {
    printf("- Tarefa: %s\n", taskDesc.task.nameOfTask);
    printf("- CPU e Disco\n");
    printf("Tempo de CPU = %hu ut\n", taskDesc.cpuTime);
    printf("Tempo de E/S = %hu ut\n", taskDesc.inputOutputTime);
    printf("Taxa de ocupação da CPU = %.2f%%\n", (float)taskDesc.cpuTime / (taskDesc.cpuTime + taskDesc.inputOutputTime) * 100);
    printf("Taxa de ocupação do disco = %.2f%%\n", taskDesc.pagination.physicalBytesAllocated == 0 ? 0 : ((float)taskDesc.pagination.bytesAllocated / taskDesc.pagination.physicalBytesAllocated) * 100);
    printf("- Memória\n");
    printf("Número de páginas lógicas = %u\n", taskDesc.pagination.finalPage );

    for (int i = 0; i < taskDesc.quantityVariables; ++i) {
        Variable var = taskDesc.variable[i];
        printf("- %s\n", var.name);
        printf("Endereço Lógicos = %u a %u ( %u : %u a %u : %u )\n", var.logicalInitialByte, var.logicalFinalByte,
               var.logicalInitialByte / 512, var.logicalInitialByte % 512, var.logicalFinalByte / 512, var.logicalFinalByte % 512);
        printf("Endereço Físicos = %u a %u ( %u : %u a %u : %u )\n", var.physicalInitialByte, var.physicalFinalByte,
               var.physicalInitialByte / 512, var.physicalInitialByte % 512, var.physicalFinalByte / 512, var.physicalFinalByte % 512);
    }

    printf("- Tabela de Páginas\n");
    for (int i = 0; i < taskDesc.pagination.finalPage; ++i) {
        printf("PL %d (%u a %u) --> PF %d (%u a %u)\n", i, i * 512, (i + 1) * 512 - 1, taskDesc.pagination.initialBytesAllocated / 512 + i, 
        ((taskDesc.pagination.initialBytesAllocated / 512) + i) * 512,((taskDesc.pagination.initialBytesAllocated / 512) + i) * 512 + 511);
    }
}

boolean new(String instruction, TaskDescriptor* taskDescriptor) {
    for (int i = 0; i < MAXIMUN_NUMBER_OF_VARIABLES; i++) {
        if (strlen(taskDescriptor->variable[i].name) == 0) {
            sscanf(instruction, "%s new %d", taskDescriptor->variable[i].name, &taskDescriptor->variable[i].value);
            taskDescriptor->variable->logicalInitialByte += taskDescriptor->pagination.bytesAllocated + 1;
            taskDescriptor->variable->logicalFinalByte += taskDescriptor->pagination.bytesAllocated + taskDescriptor->variable[i].value;
            
            taskDescriptor->pagination.bytesAllocated += taskDescriptor->variable[i].value;
            
            taskDescriptor->variable->physicalInitialByte += taskDescriptor->pagination.physicalBytesAllocated + 1;
            taskDescriptor->variable->physicalFinalByte += taskDescriptor->pagination.physicalBytesAllocated + taskDescriptor->variable[i].value;

            taskDescriptor->pagination.physicalBytesAllocated += taskDescriptor->variable[i].value;

            taskDescriptor->pagination.finalPage = roundingNumber(taskDescriptor->pagination.bytesAllocated / 512.0);

            if (taskDescriptor->pagination.bytesAllocated > 4096) {
                finishTask(taskDescriptor, TRUE);
                return FALSE;
            }
            return TRUE;
        }
    }
    return FALSE;
}

boolean memoryAccess(String instruction, TaskDescriptor* taskDescriptor) {
    String identifier;
    unsigned int value;
    sscanf(instruction, "%[^[][%d]", identifier, &value);
    for (int i = 0; i < MAXIMUN_NUMBER_OF_VARIABLES; i++) {
        if (strcmp(taskDescriptor->variable[i].name, identifier) == 0) {
            if (value >= taskDescriptor->variable[i].value) {
                printf("Error in instruction: %s\n", instruction);
                finishTask(taskDescriptor, TRUE);
                return FALSE;
            } else {
                return TRUE;
            }
        }
    }
    finishTask(taskDescriptor, TRUE);
    return FALSE;
}

void finishTask(TaskDescriptor* taskDescriptor, boolean aborted) {
    fclose(taskDescriptor->task.taskFile);
    taskDescriptor->status = FINISHED;
    if (aborted) {
        taskDescriptor->aborted = TRUE;
    }
}

void checkAndUpdateSuspendedTasks(TaskDescriptorQueue* queue, TaskDescriptor tasks[], int numberOfTasks) {
    for (int i = 0; i < numberOfTasks; i++) {
        if (tasks[i].status == SUSPENDED) {
            tasks[i].suspendedTime -= UT;
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

boolean header(String instruction, TaskDescriptor* taskDescriptor) {
    sscanf(instruction, "#%*[^=]=%u", &taskDescriptor->pagination.bytesAllocated);
    taskDescriptor->pagination.finalPage = roundingNumber(taskDescriptor->pagination.bytesAllocated / 512.0);
    taskDescriptor->pagination.bytesAllocated = taskDescriptor->pagination.finalPage * 512 - 1;
    taskDescriptor->pagination.physicalBytesAllocated += taskDescriptor->pagination.bytesAllocated;
    if (taskDescriptor->pagination.finalPage > 4096) {
        finishTask(taskDescriptor, TRUE);
        return FALSE;
    }
    return FALSE;
}

boolean executeTask(TaskDescriptorQueue* queue, RoundRobin* roundRobin, TaskDescriptor* taskDescriptor, TaskDescriptor tasks[], int numberOfTasks) {
    while (roundRobin->preemptionTimeCounter < QUANTUM) {
        checkAndUpdateSuspendedTasks(queue, tasks, numberOfTasks);
        if (taskDescriptor != NULL && taskDescriptor->status == RUNNING) {
            String instruction;
            if (fgets(instruction, sizeof(instruction), taskDescriptor->task.taskFile) != NULL) {
                roundRobin->totalCPUClocks++;
                taskDescriptor->cpuTime++;

                if (matchRegex(instruction, INSTRUCTION_HEADER_REGEX)) {
                    roundRobin->totalCPUClocks--;
                    taskDescriptor->cpuTime--;
                    header(instruction, taskDescriptor);
                }
                else if (matchRegex(instruction, INSTRUCTION_NEW_REGEX)) {
                    new(instruction, taskDescriptor);
                } else if (matchRegex(instruction, INSTRUCTION_IDEX_REGEX)) {
                    memoryAccess(instruction, taskDescriptor);
                } else if (matchRegex(instruction, INSTRUCTION_READ_DISK_REGEX)) {
                    readDisk(taskDescriptor);
                    break;
                }
            } else {
                finishTask(taskDescriptor, FALSE);
                break;
            }
        }
        else {
            roundRobin->waitingTime++;
        }
        roundRobin->preemptionTimeCounter += UT;
    }
    return TRUE;
}

void scheduleTasks(TaskDescriptor tasks[], int numberOfTasks) {
    RoundRobin roudRobin;
    roudRobin.totalCPUClocks = 0;
    roudRobin.waitingTime = 0;
    roudRobin.preemptionTimeCounter = 0;
    TaskDescriptorQueue* taskDescriptorQueue = createTaskDescriptorQueue(); 

    float numberOfTasksRunning = 0;
    for (int i = 0; i < numberOfTasks; i ++) {
        if (tasks[i].status == READY && tasks[i].aborted == FALSE) {
            numberOfTasksRunning++;
            tasks[i].pagination.initialBytesAllocated = 20480 + i * 4096;
            tasks[i].pagination.physicalBytesAllocated = tasks[i].pagination.initialBytesAllocated;
            printf("\nFISICA QUANTICA: %u\n",  tasks[i].pagination.physicalBytesAllocated);
            enqueueTaskDescriptor(taskDescriptorQueue, &tasks[i]);
        }
    }

    while (TRUE) {

        if (allTasksFinished(tasks, numberOfTasks)) {
            destroyTaskDescriptorQueue(taskDescriptorQueue);
            break;
        }
        
        TaskDescriptor* taskRunningPtr = dequeueTaskDescriptor(taskDescriptorQueue);

        if (taskRunningPtr != NULL) {
            taskRunningPtr->status = RUNNING;
        }
        
        executeTask(taskDescriptorQueue, &roudRobin, taskRunningPtr, tasks, numberOfTasks);
        
        if (taskRunningPtr != NULL && taskRunningPtr->status == RUNNING) {
            taskRunningPtr->status = READY;
            enqueueTaskDescriptor(taskDescriptorQueue, taskRunningPtr);
        }

        roudRobin.preemptionTimeCounter = 0;
    }

    for (int i = 0; i < numberOfTasks; i ++) {
        printTaskDescriptor(tasks[i]);
        printf("\n\n");
    }
}

boolean matchRegex(String string, const char *pattern) {
    regex_t regex;
    int result;

    result = regcomp(&regex, pattern, REG_EXTENDED);
    if (result != 0) {
        char error_message[100];
        regerror(result, &regex, error_message, sizeof(error_message));
        fprintf(stderr, "Erro ao compilar a expressão regular: %s\n", error_message);
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
    descriptor->task.taskFile = fopen(fullFileName, "r");
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

    FILE *file = fopen(fullFileName, "r");

    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
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

int tsmm(int numberOfArguments, char *arguments[]) {
    if (!validateNumberOfArguments(numberOfArguments)) {
        perror("Programa finalizado, argumentos inválidos");
        exit(EXIT_FAILURE);
    }

    TaskDescriptor tasksDescriptions[numberOfArguments -1];
    for (int i = 1; i < numberOfArguments; i++) {
        if (!validateFile(arguments[i])) {
            printf("\nError Arquivo %s\n", arguments[i]);
            tasksDescriptions[i - 1].aborted = TRUE;
            tasksDescriptions[i - 1].status = FINISHED;
        }
        else {
            initializeTaskDescriptor(&tasksDescriptions[i - 1], arguments[i]);
        }
    }
    scheduleTasks(tasksDescriptions, numberOfArguments - 1);
    return EXIT_SUCCESS;
}//tsmm()

int main(int argc, char *argv[]) {
    return tsmm(argc, argv);
}//main()
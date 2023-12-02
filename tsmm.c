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
        if (tasks[i].status ==  FINISHED) {
            return FALSE;
        }
    } 
    return TRUE;
}//allTasksFinished()

void readDisk(TaskDescriptor* taskDescriptor) {
    taskDescriptor->status = SUSPENDED;
    taskDescriptor->suspendedTime += SUSPENDED_TIME;
    taskDescriptor->inputOutputTime += SUSPENDED_TIME; 
}//readDisk()

boolean new(String instruction, TaskDescriptor* taskDescriptor) {
    for (int i = 0; i < MAXIMUN_NUMBER_OF_VARIABLES; i++) {
        if (strlen(taskDescriptor->variable[i].name) == 0) {
            sscanf(instruction, "%s new %d", taskDescriptor->variable[i].name, &taskDescriptor->variable[i].value);
            return TRUE;
        }
    }
    return FALSE;
}//new()

boolean memoryAccess(String instruction, TaskDescriptor* taskDescriptor) {
    String identifier;
    unsigned int value;
    sscanf(instruction, "%[^[][%d]", identifier, &value);
    for (int i = 0; i < MAXIMUN_NUMBER_OF_VARIABLES; i++) {
        if (strcmp(taskDescriptor->variable[i].name, identifier) == 0) {
            if (value >= taskDescriptor->variable[i].value) {
                taskDescriptor->aborted = TRUE;
                return FALSE;
            }
            else {
                return TRUE;
            }
        }
    }
    taskDescriptor->aborted = TRUE;
    return FALSE;
}//memoryAccess()

boolean executeTask(RoundRobin* roundRobin, TaskDescriptor* taskDescriptor) {
    
    while (roundRobin->preemptionTimeCounter < QUANTUM) {
        String instruction;
        if (fgets(instruction, sizeof(instruction), taskDescriptor->task.taskFile) != NULL) {
            if (matchRegex(instruction, INSTRUCTION_NEW_REGEX)) {
                new(instruction, taskDescriptor);
            }   
            else if (matchRegex(instruction, INSTRUCTION_IDEX_REGEX)) {
                memoryAccess(instruction, taskDescriptor);
            }
            else {
                readDisk(taskDescriptor);
                break;
            }
        }
        else {
            taskDescriptor->status = FINISHED;
            fclose(taskDescriptor->task.taskFile);
            break;
        }
    }
    
    return TRUE;
}//executeTask()

void scheduleTasks(TaskDescriptor tasks[], int numberOfTasks) {
    RoundRobin roudRobin;
    roudRobin.totalCPUClocks = 0;
    roudRobin.preemptionTimeCounter = 0;

    TaskDescriptorQueue* taskDescriptorQueue = createTaskDescriptorQueue();
    for (int i = 0; i < numberOfTasks; i++) {
        enqueueTaskDescriptor(taskDescriptorQueue, &tasks[i]);
    }   

    while (TRUE) {
        // Verifica se há tarefas aptas à serem escalonadas (estado diferente de TERMINADA); 
        if (allTasksFinished(tasks, numberOfTasks)) {
            destroyTaskDescriptorQueue(taskDescriptorQueue);
            break;
        }

        TaskDescriptor* taskRunningPtr = dequeueTaskDescriptor(taskDescriptorQueue);

        if (taskRunningPtr != NULL) {
            executeTask(&roudRobin, taskRunningPtr);
        }
    }
}//scheduleTasks()

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

    // Inicializa as variáveis da tarefa com 0
    for (int i = 0; i < MAXIMUN_NUMBER_OF_VARIABLES; ++i) {
        memset(descriptor->variable[i].name, 0, STRING_DEFAULT_SIZE);
        descriptor->variable[i].value = 0;
    }
}//initializeTaskDescriptor()


int tsmm(int numberOfArguments, char *arguments[]) {
    if (!validateNumberOfArguments(numberOfArguments)) {
        perror("Programa finalizado, argumentos inválidos");
        exit(0);
    }

    TaskDescriptor tasksDescriptions[numberOfArguments -1];
    for (int i = 1; i < numberOfArguments; i++) {
        if (!validateFile(arguments[i])) {
            printf("\nError Arquivo %s\n", arguments[i]);
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
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
void enqueueTaskDescriptor(TaskDescriptorQueue* queue, TaskDescriptor taskDesc) {
    TaskDescriptorNode* newNode = (TaskDescriptorNode*)malloc(sizeof(TaskDescriptorNode));
    newNode->taskDescriptor = taskDesc;
    newNode->next = NULL;

    if (isTaskDescriptorQueueEmpty(queue)) {
        queue->front = queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}//enqueueTaskDescriptor()

// Função para desenfileirar um elemento da fila
TaskDescriptor dequeueTaskDescriptor(TaskDescriptorQueue* queue) {
    if (isTaskDescriptorQueueEmpty(queue)) {
        printf("A fila está vazia.\n");
        TaskDescriptor emptyTaskDesc = {0}; 
        return emptyTaskDesc;
    }

    TaskDescriptor taskDesc = queue->front->taskDescriptor;
    TaskDescriptorNode* temp = queue->front;

    queue->front = queue->front->next;
    free(temp);

    return taskDesc;
}//dequeueTaskDescriptor()

// Função para liberar a memória alocada para a fila
void destroyTaskDescriptorQueue(TaskDescriptorQueue* queue) {
    while (!isTaskDescriptorQueueEmpty(queue)) {
        dequeueTaskDescriptor(queue);
    }
    free(queue);
}//destroyTaskDescriptorQueue()

// Função para verificar se todos os nodos na fila têm o status FINISHED
int allTasksFinished(TaskDescriptorQueue* queue) {
    TaskDescriptorNode* current = queue->front;

    while (current != NULL) {
        if (current->taskDescriptor.status != FINISHED) {
            return FALSE;
        }
        current = current->next;
    }
    return TRUE;
}//allTasksFinished()


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
}//matchRegex

boolean validateFile(FileName fileName) {
    FileName fullFileName;
    snprintf(fullFileName, FILE_NAME_SIZE, "%s%s", fileName, FILE_EXTENSION);

    FILE *file = fopen(fullFileName, "r");

    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return FALSE;
    }
    String line;
    int lineNumber = 0;

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

void scheduleTasks(TaskDescriptor tasks[], int numberOfTasks) {
    RoundRobin roudRobin;
    roudRobin.totalCPUClocks = 0;
    roudRobin.preemptionTimeCounter = 0;

    TaskDescriptorQueue* taskDescriptorQueue = createTaskDescriptorQueue();
    for (int i = 0; i < numberOfTasks; i++) {
        enqueueTaskDescriptor(taskDescriptorQueue, tasks[0]);
    }   

    while(TRUE) {
        
        if(allTasksFinished(taskDescriptorQueue)) break;     
        

    }
}//scheduleTasks

int tsmm(int numberOfArguments, char *arguments[]) {
    if (!validateNumberOfArguments(numberOfArguments)) {
        perror("Programa finalizado, argumendos inválidos");
        exit(0);
    }

    TaskDescriptor tasksDescriptions[numberOfArguments -1];
    for (int i = 1; i < numberOfArguments; i++) {
        if (!validateFile(arguments[i])) {
            printf("\nError Arquivo %s\n", arguments[i]);
        }
        else {
            FileName fullFileName;
            snprintf(fullFileName, FILE_NAME_SIZE, "%s%s", arguments[i], FILE_EXTENSION);
            tasksDescriptions[--i].task.taskFile = fopen(fullFileName, "r");
            tasksDescriptions[--i].status = READY;
            strcpy(tasksDescriptions[--i].task.nameOfTask, arguments[0]);
        }
    }
    scheduleTasks(tasksDescriptions, numberOfArguments - 1);
    return EXIT_SUCCESS;
}//tsmm()


int main(int argc, char *argv[]) {
    return tsmm(argc, argv);
}//main
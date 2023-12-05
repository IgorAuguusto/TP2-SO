#pragma once

// Constantes lógicas.
#define TRUE 1
#define FALSE 0

//Formatações
#define NEW_LINE '\n'
#define EMPTY_LINE "\n\n"
#define TAB '\t'

// Unidade de tempo: equivale à 1 instrução da tarefa.
#define UT 1

// Fatia de tempo (quantum) = 2ut (unidades de tempo).
#define QUANTUM (2 * UT)

// Define o número de tarefas realizadas pelo programa.
#define NUMBER_OF_TASKS 4

// Tempo de suspensão da tarefa
#define SUSPENDED_TIME 5

// Número máximo de instruções da tarefa.
#define MAXIMUM_NUMBER_OF_INSTRUCTIONS 64

// Número máximo de variaveis por tarefa
#define MAXIMUN_NUMBER_OF_VARIABLES 10

// Tamanho padrão de string.
#define STRING_DEFAULT_SIZE 128

// Tamanho do nome do arquivo.
#define FILE_NAME_SIZE 64

// Extensão do arquivo
#define FILE_EXTENSION ".tsk"

// Padrões das instruções do arquivo
#define INSTRUCTION_HEADER_REGEX "^#T=[0-9]+\\s*$"
#define INSTRUCTION_NEW_REGEX "^[a-zA-Z_][a-zA-Z0-9_]*\\s+new\\s+[0-9]+\\s*$"
#define INSTRUCTION_READ_DISK_REGEX "^read disk\\s*$"
#define INSTRUCTION_IDEX_REGEX "^[a-zA-Z_][a-zA-Z0-9_]*\\s*\\[[0-9]+\\]\\s*$"

// Define uma string de tamanho padrão T, onde T é igual STRING_DEFAULT_SIZE.
typedef char String[STRING_DEFAULT_SIZE];

// Nome do arquivo.
typedef char FileName[FILE_NAME_SIZE];

// Tipo lógico
typedef int boolean;

// Indica o estado atual da tarefa. 
typedef enum {
    READY, 
    RUNNING, 
    SUSPENDED, 
    FINISHED
} TaskStatus;

// Instrução 
typedef enum {
    HEADER, 
    NEW, 
    MEMORY_ACESS, 
    READER_DISk
} Instruction;

// Representa a strutura da tarefa
typedef struct  {
   	// Número de instruções da tarefa.
	unsigned short numberOfTaskInstructions;

	// Nome da tarefa.
	String nameOfTask;

    // Ponteiro para a tarefa
	FILE *taskFile;
} Task;

// Estrutura para representar uma variavel da instrução new
typedef struct {
    String name;
    unsigned int value;
    unsigned int logicalInitialByte;
    unsigned int logicalFinalByte;
    unsigned int physicalInitialByte;
    unsigned int physicalFinalByte;
} Variable;

// Estrutura que representa a páginação das páginas.
typedef struct  {
   unsigned int bytesAllocated;
   unsigned int physicalBytesAllocated; 
   unsigned int initialBytesAllocated; 
   unsigned int finalPage;
}Pagination;



// Representa o descritor da tarefa
typedef struct {
    // Identificação da tarefa deste bloco de controle da tarefa.
    Task task;

    // Indica o estado atual da tarefa. O estado da tarefa deve ser atualizado segundo o seu ciclo de vida durante sua execução. 
    TaskStatus status;

    // Tempo de Cpu.
    unsigned short cpuTime;

    // Variavel para controlar o tempo em que a terefa fica suspensa.
    unsigned short suspendedTime;

    // Quantidade de variáveis.
    unsigned short quantityVariables;

    // Variavel de controle para saber se a terafa foi abortada.
    boolean aborted;

    // Tempo de Entrada e Saida.
    unsigned short inputOutputTime;

    Pagination pagination;

    // Variaveis da tarefa
    Variable variable[MAXIMUN_NUMBER_OF_VARIABLES];
} TaskDescriptor;

// Representa uma unidade de tempo (UT), renomeada para dar mais sentido às variáveis da struct RoundRobin.
typedef unsigned short int TimeUnit;

// Representa os dados coletados pelo escalonador Round-Robin, para calcular tempos médios após a execução dos programas.
typedef struct {
    // Representa os 'clocks' totais do CPU (UT)
    TimeUnit totalCPUClocks;

    // Representa o contador de preempção por tempo (Quantum)
    TimeUnit preemptionTimeCounter;

    // Representa o tempo de médio de espera
    TimeUnit waitingTime;
} RoundRobin;

// Definição da estrutura do nó da fila
typedef struct TaskDescriptorNode {
    TaskDescriptor* taskDescriptorPtr;
    struct TaskDescriptorNode* next;
} TaskDescriptorNode;


// Definição da estrutura da fila
typedef struct {
    TaskDescriptorNode* front; 
    TaskDescriptorNode* rear; 
} TaskDescriptorQueue;

// Função para criar uma fila vazia de descritores de tarefas
TaskDescriptorQueue* createTaskDescriptorQueue();

// Função para verificar se a fila de descritores de tarefas está vazia
int isTaskDescriptorQueueEmpty(TaskDescriptorQueue* queue);

// Função para enfileirar um elemento na fila de descritores de tarefas
void enqueueTaskDescriptor(TaskDescriptorQueue* queue, TaskDescriptor* taskDescPtr);

// Função para desenfileirar um elemento da fila de descritores de tarefas
TaskDescriptor* dequeueTaskDescriptor(TaskDescriptorQueue* queue);

// Função para liberar a memória alocada para a fila de descritores de tarefas
void destroyTaskDescriptorQueue(TaskDescriptorQueue* queue);

// Função para verificar se todas as tarefas têm o status FINISHED
boolean allTasksFinished(TaskDescriptor tasks[], int numberOfTasks);

// Função para realizar a leitura de disco para uma tarefa
void readDisk(TaskDescriptor* taskDescriptor);

// Função para criar uma nova variável para uma tarefa
boolean new(String instruction, TaskDescriptor* taskDescriptor);

// Função para acessar a memória de uma tarefa
boolean memoryAccess(String instruction, TaskDescriptor* taskDescriptor);

// Função para finalizar uma tarefa
void finishTask(TaskDescriptor* taskDescriptor, boolean aborted);

// Função para verificar e atualizar as tarefas suspensas
void checkAndUpdateSuspendedTasks(TaskDescriptorQueue* queue, TaskDescriptor tasks[], int numberOfTasks);

// Função para executar a lógica da tarefa
boolean executeTask(TaskDescriptorQueue* queue, RoundRobin* roundRobin, TaskDescriptor* taskDescriptor, TaskDescriptor tasks[], int numberOfTasks);

// Função para agendar e executar as tarefas
void scheduleTasks(TaskDescriptor tasks[], int numberOfTasks);

// Função para verificar se uma string corresponde a um padrão regex
boolean matchRegex(String string, const char *pattern);

// Função para validar o número de argumentos passados para o programa
boolean validateNumberOfArguments(int numberOfArguments);

// Função para inicializar o descritor de tarefa
void initializeTaskDescriptor(TaskDescriptor* descriptor, String taskName);

// Função para validar um arquivo de tarefa
boolean validateFile(FileName fileName);

// Função principal do programa
int tsmm(int numberOfArguments, char *arguments[]);

// Função principal que chama a função tsmm
int main(int argc, char *argv[]);

int roundingNumber(float number);
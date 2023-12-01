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
#define QUANTUM 2

// Define o número de tarefas realizadas pelo programa.
#define NUMBER_OF_TASKS 4

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
} Variable;


// Representa o descritor da tarefa
typedef struct {
    // Identificação da tarefa deste bloco de controle da tarefa.
    Task task;

    // Indica o estado atual da tarefa. O estado da tarefa deve ser atualizado segundo o seu ciclo de vida durante sua execução. 
    TaskStatus status;

    // Tempo de Cpu
    unsigned short cpuTime;

    // Tempo de Entrada e Saida
    unsigned short inputOutputTime;

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

    // Matriz que representa o 'clock' (UT) em que cada programa entrou (PRONTA) e saiu (TERMINADA) da fila do processador.
    TimeUnit entryAndExitTimesQueue[NUMBER_OF_TASKS][2];
} RoundRobin;

// Definição da estrutura do nó da fila
// Alterando a estrutura TaskDescriptorNode para armazenar um ponteiro para TaskDescriptor
typedef struct TaskDescriptorNode {
    TaskDescriptor* taskDescriptorPtr;
    struct TaskDescriptorNode* next;
} TaskDescriptorNode;


// Definição da estrutura da fila
typedef struct {
    TaskDescriptorNode* front; 
    TaskDescriptorNode* rear; 
} TaskDescriptorQueue;

// Protótipos das funções para manipulação da fila
TaskDescriptorQueue* createTaskDescriptorQueue();
int isTaskDescriptorQueueEmpty(TaskDescriptorQueue* queue);
void enqueueTaskDescriptor(TaskDescriptorQueue* queue, TaskDescriptor* taskDescPtr);
TaskDescriptor* dequeueTaskDescriptor(TaskDescriptorQueue* queue);
void destroyTaskDescriptorQueue(TaskDescriptorQueue* queue);

// Verifica se todas as tarefas na lista têm o status FINISHED
boolean allTasksFinished(TaskDescriptor tasks[], int numberOfTasks);

// Escalona as tarefas para execução
void scheduleTasks(TaskDescriptor tasks[], int numberOfTasks);

// Verifica se uma string corresponde a um padrão de expressão regular
boolean matchRegex(String string, const char *pattern);

// Valida se um arquivo segue o padrão de instruções
boolean validateFile(FileName fileName);

// Valida o número de argumentos passados para o programa
boolean validateNumberOfArguments(int numberOfArguments);

// Inicializa os atributos de um descritor de tarefa
void initializeTaskDescriptor(TaskDescriptor* descriptor, String taskName);

// Função principal que inicia o processamento das tarefas
int tsmm(int numberOfArguments, char *arguments[]);
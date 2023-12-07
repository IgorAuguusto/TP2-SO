#pragma once

// Constantes lógicas.
#define TRUE 1
#define FALSE 0

//Formatações
#define NEW_LINE "\n"
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

// Constantes para memória física e lógica
// Total de memória física em bytes (64 KB)
#define PHYSICAL_MEMORY_TOTAL 65536  
// Maior tamanho de memória lógica em bytes (4 KB)
#define LARGEST_LOGICAL_MEMORY_SIZE 4096  


// Tamanho de cada página lógica/física em bytes
#define LOGICAL_PHYSICAL_PAGE_SIZE 512  

#define RESERVED_PROGRAM_MEMORY_SIZE 20480 

// Padrões das instruções do arquivo
#define INSTRUCTION_HEADER_REGEX "^#T=[0-9]+\\s*$"
#define INSTRUCTION_NEW_REGEX "^[a-zA-Z_][a-zA-Z0-9_]*\\s+new\\s+[0-9]+\\s*$"
#define INSTRUCTION_READ_DISK_REGEX "^read disk\\s*$"
#define INSTRUCTION_IDEX_REGEX "^[a-zA-Z_][a-zA-Z0-9_]*\\s*\\[[0-9]+\\]\\s*$"

// Mensagens de erros
#define DIVERGET_INSTRUCTION_ERROR "A tarefa %s não será executada, pois tem instruções diferentes do tipo 1, 2 e 3."
#define INVALID_ARGUMENTS_ERROR "O Programa foi abortado, número de argumentos inválidos"
#define MEMORY_ACCESS_ERROR "A tarefa %s foi abortada, pois tentou realizar um acesso inválido à memória: %s[%d]"
#define UNDECLARED_IDENTIFIER_ERROR "A tarefa %s foi abortada, pois tentou acessar o identificador (%s) que não foi declarado."
#define FILE_OPEN_ERROR "Falha em abrir o arquivo: (%s)"
#define IDENTIFY_ALREADY_DECLARED "A tarefa %s foi abortada, pois tentou criar um identificador (%s) já declarado."


// Extensão do arquivo
#define FILE_EXTENSION ".tsk"

// Região 
#define LOCALE "pt_BR.utf8"

// Opção de abertura do arquivo.
#define FILE_OPENING_OPTION "r"

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
    MEMORY_ACCESS, 
    READ_DISK
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

// Representa as memórias logugicas.
typedef struct {
    unsigned int logicalInitialByte;
    unsigned int logicalFinalByte;
} LogicalMemory;

// Representa as memórias fisícas.
typedef struct {
    unsigned int physicalInitialByte;
    unsigned int physicalFinalByte;
} PhysicalMemory;

// Estrutura para representar uma variavel da instrução new
typedef struct {
    String name;
    unsigned int value;

    LogicalMemory logicalMemory;
    PhysicalMemory physicalMemory;
} Variable;

// Estrutura que representa a páginação das páginas.
typedef struct  {
   unsigned int bytesAllocated;
   unsigned int physicalBytesAllocated; 
   unsigned int initialBytesAllocated; 
   unsigned int finalPage;
} Pagination;



// Representa o descritor da tarefa
typedef struct {
    // Identificação da tarefa deste bloco de controle da tarefa.
    Task task;

    // Indica o estado atual da tarefa. O estado da tarefa deve ser atualizado segundo o seu ciclo de vida durante sua execução. 
    TaskStatus status;

    // Instante de tempo no qual a tarefa entrou na fila a primeira vez.
    unsigned short startTime;

    // Instante de tempo no qual a tarefa foi finalizada.
    unsigned short endTime;

    // Tempo de Cpu.
    unsigned short cpuTime;

    // Variavel para controlar o tempo em que a terefa fica suspensa.
    unsigned short suspendedTime;

    // Variavel de controle para saber se a terafa foi abortada.
    boolean aborted;

    // Tempo de Entrada e Saida.
    unsigned short inputOutputTime;

    // Quantidade de váriaveis
    unsigned short quantityVariables;

    // Campo que representa a páginação
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
    
    // Representa o total de entrada e saida
    TimeUnit totalOutputTime;

    //Representa o tempo de espera
    TimeUnit waitTime;

    // Representa o contador de preempção por tempo (Quantum)
    TimeUnit preemptionTimeCounter;

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

int isTaskDescriptorQueueEmpty(TaskDescriptorQueue* queue);
void enqueueTaskDescriptor(TaskDescriptorQueue* queue, TaskDescriptor* taskDescPtr);
TaskDescriptor* dequeueTaskDescriptor(TaskDescriptorQueue* queue);
void destroyTaskDescriptorQueue(TaskDescriptorQueue* queue);
boolean allTasksFinished(TaskDescriptor tasks[], int numberOfTasks);
void readDisk(TaskDescriptor* taskDescriptor);
void printTasks(TaskDescriptor tasks[], int numberOfTasks, RoundRobin roundRobin);
float calculateCPURate(TaskDescriptor taskDesc, RoundRobin roundRobin);
float calculateDiskRate(TaskDescriptor taskDesc, RoundRobin roundRobin);
void printVariableMemoryInfo(Variable var);
void printPageTableInfo(TaskDescriptor taskDesc);
void printTaskDescriptor(TaskDescriptor taskDesc, RoundRobin roundRobin);
void updateLogicalMemory(TaskDescriptor* taskDescriptor, int index, int value);
void updatePhysicalMemory(TaskDescriptor* taskDescriptor, int index, int value);
boolean new(String instruction, TaskDescriptor* taskDescriptor, RoundRobin* roundRobin);
boolean memoryAccess(String instruction, TaskDescriptor* taskDescriptor, RoundRobin* roundRobin);
void finishTask(TaskDescriptor* taskDescriptor, boolean aborted, RoundRobin* roundRobin);
void checkAndUpdateSuspendedTasks(TaskDescriptorQueue* queue, TaskDescriptor tasks[], int numberOfTasks, unsigned timeUnits);
int roundingNumber(float number);
void updatePagination(TaskDescriptor* taskDescriptor, unsigned int bytes);
boolean header(String instruction, TaskDescriptor* taskDescriptor);
Instruction determineInstructionType(String instruction);
boolean executeInstruction(TaskDescriptorQueue* queue, RoundRobin* roundRobin, TaskDescriptor* taskDescriptor, TaskDescriptor tasks[], int numberOfTasks);
void initializeRoundRobin(RoundRobin* roundRobin);
void initializeTaskQueue(TaskDescriptorQueue* taskDescriptorQueue, TaskDescriptor tasks[], int numberOfTasks);
void runTasks(TaskDescriptorQueue* taskDescriptorQueue, RoundRobin* roundRobin, TaskDescriptor tasks[], int numberOfTasks);
RoundRobin scheduleTasks(TaskDescriptor tasks[], int numberOfTasks);
boolean matchRegex(String string, const char *pattern);
boolean validateNumberOfArguments(int numberOfArguments);
void initializeTaskDescriptor(TaskDescriptor* descriptor, String taskName);
boolean validateFile(FileName fileName);
int tsmm(int numberOfTasks, char *tasks[]);
int main(int argc, char *argv[]);
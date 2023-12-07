#pragma once

// Constantes lógicas.
#define TRUE 1
#define FALSE 0

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
#define DIVERGET_INSTRUCTION_ERROR "\nA tarefa %s não será executada, pois tem instruções diferentes do tipo 1, 2 e 3."
#define INVALID_ARGUMENTS_ERROR "\nO Programa foi abortado, número de argumentos inválidos"
#define MEMORY_ACCESS_ERROR "\nA tarefa %s foi abortada, pois tentou realizar um acesso inválido à memória: %s[%d]"
#define UNDECLARED_IDENTIFIER_ERROR "\nA tarefa %s foi abortada, pois tentou acessar o identificador (%s) que não foi declarado."
#define FILE_OPEN_ERROR "\nFalha em abrir o arquivo: (%s)"
#define IDENTIFY_ALREADY_DECLARED_ERROR "\nA tarefa %s foi abortada, pois tentou criar um identificador (%s) já declarado."
#define ALLOCATION_SPACE_ERROR "\nA tarefa %s foi abortada, pois excedeu o tamanho maximo de alocação da memória reservada (%d) bytes"

// Representa um caractere nulo.
#define NULL_CHAR '\0'

//Formatações
#define DOT '.'
#define COMMA ','

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

    // Quantidade de acessos
    unsigned short quantityAccesses;

    // Campo que representa a páginação
    Pagination pagination;

    // Acessos a memoria
    Variable accesses[MAXIMUN_NUMBER_OF_VARIABLES];

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

/* Cria uma nova fila de descritores de tarefa.
   Aloca memória para a estrutura TaskDescriptorQueue e inicializa seus ponteiros front e rear como NULL,
   indicando que a fila está vazia.
   Retorna um ponteiro para a nova fila criada.
*/
TaskDescriptorQueue* createTaskDescriptorQueue();

/* Verifica se a fila de descritores de tarefa está vazia.
   Verifica se o ponteiro 'front' da fila passada como parâmetro é NULL,
   indicando que não há nenhum elemento na fila.
   Retorna TRUE se a fila estiver vazia, caso contrário retorna FALSE.
*/
boolean isTaskDescriptorQueueEmpty(TaskDescriptorQueue* queue);

/* Insere um descritor de tarefa na fila de descritores de tarefa.
   Adiciona um novo nó contendo o descritor de tarefa fornecido à fila.
   Se a fila estiver vazia, o novo nó é inserido como o primeiro e último elemento da fila.
   Caso contrário, o novo nó é adicionado ao final da fila, atualizando o ponteiro 'rear'.
   Parâmetros:
   - queue: Ponteiro para a fila de descritores de tarefa.
   - taskDescPtr: Ponteiro para o descritor de tarefa a ser inserido na fila.
*/
void enqueueTaskDescriptor(TaskDescriptorQueue* queue, TaskDescriptor* taskDescPtr);

/* Remove e retorna o descritor de tarefa no início da fila de descritores de tarefa.
   Se a fila estiver vazia, retorna NULL, indicando que a fila está sem elementos.
   Caso contrário, remove o primeiro elemento da fila, atualizando o ponteiro 'front'.
   Parâmetros:
   - queue: Ponteiro para a fila de descritores de tarefa.
   Retorno:
   - Ponteiro para o descritor de tarefa removido, ou NULL se a fila estiver vazia.
*/
TaskDescriptor* dequeueTaskDescriptor(TaskDescriptorQueue* queue);

/* Destroi a fila de descritores de tarefa liberando toda a memória alocada.
   Remove cada descritor de tarefa da fila usando a função 'dequeueTaskDescriptor()'
   até que a fila esteja vazia e, em seguida, libera a memória da própria fila.
   Parâmetros:
   - queue: Ponteiro para a fila de descritores de tarefa a ser destruída.
*/
void destroyTaskDescriptorQueue(TaskDescriptorQueue* queue);

/* Verifica se todas as tarefas foram concluídas.
   Percorre o vetor de descritores de tarefa verificando o status de cada uma.
   Parâmetros:
   - tasks: Array de descritores de tarefa.
   - numberOfTasks: Número total de tarefas no array.
   Retorno:
   - TRUE se todas as tarefas estiverem marcadas como 'FINISHED', FALSE caso contrário.
*/
boolean allTasksFinished(TaskDescriptor tasks[], int numberOfTasks);

/* Simula a leitura do disco por uma tarefa.
   Atualiza o status da tarefa para 'SUSPENDED' e adiciona o tempo de E/S ao tempo total de entrada/saída da tarefa.
   Parâmetros:
   - taskDescriptor: Ponteiro para o descritor de tarefa a ser modificado.
*/
void readDisk(TaskDescriptor* taskDescriptor);

/* Imprime os descritores das tarefas ativas.
   Itera sobre as tarefas verificando se foram abortadas. Se não foram abortadas,
   imprime o descritor da tarefa usando a função 'printTaskDescriptor'.
   Parâmetros:
   - tasks: Array de descritores de tarefa.
   - numberOfTasks: Número total de tarefas no array.
   - roundRobin: Estrutura RoundRobin contendo informações do sistema.
*/
void printTasks(TaskDescriptor tasks[], int numberOfTasks, RoundRobin roundRobin);

/* Calcula a taxa de ocupação da CPU de uma tarefa.
   Parâmetros:
   - taskDesc: Descritor de tarefa (TaskDescriptor).
   - roundRobin: Estrutura RoundRobin contendo informações do sistema.
   Retorno:
   - Taxa de ocupação da CPU (float).
*/
float calculateCPURate(TaskDescriptor taskDesc, RoundRobin roundRobin);

/* Calcula a taxa de ocupação do disco de uma tarefa.
   Parâmetros:
   - taskDesc: Descritor de tarefa (TaskDescriptor).
   - roundRobin: Estrutura RoundRobin contendo informações do sistema.
   Retorno:
   - Taxa de ocupação de disco (float).
*/
float calculateDiskRate(TaskDescriptor taskDesc, RoundRobin roundRobin) ;

/* Conta o número de tarefas executadas com sucesso.
   Itera sobre as tarefas verificando se foram abortadas. Se uma tarefa não foi abortada,
   incrementa o contador de tarefas executadas com sucesso.
   Parâmetros:
   - tasks: Array de descritores de tarefa.
   - numberOfTasks: Número total de tarefas no array.
   Retorno:
   - Número de tarefas executadas com sucesso.
*/
int numberOfTasksPerformedSuccessfully(TaskDescriptor tasks[], int numberOfTasks);

/* Imprime informações do Round-Robin.
   Esta função imprime métricas do algoritmo Round-Robin, como o tempo médio de execução e o tempo médio de espera.
   Utiliza o número de tarefas executadas com sucesso para calcular esses valores.
   Parâmetros:
   - roundRobin: Estrutura RoundRobin contendo informações do sistema.
   - numberOfTasksPerformedSuccessfully: Número de tarefas executadas com sucesso.
*/
void printRoundRobin(RoundRobin roundRobin, int numberOfTasksPerformedSuccessfully);

/* Imprime as informações de memória de uma variável da tarefa.
   Parâmetros:
   - var: Variável (Variable) da tarefa.
*/
void printVariableMemoryInfo(Variable var) ;

/* Calcula e imprime as informações da tabela de páginas de uma tarefa.
   Parâmetros:
   - taskDesc: Descritor de tarefa (TaskDescriptor).
*/
void printPageTableInfo(TaskDescriptor taskDesc);

/* Imprime todas as informações de uma tarefa.
   Parâmetros:
   - taskDesc: Descritor de tarefa (TaskDescriptor).
   - roundRobin: Estrutura RoundRobin contendo informações do sistema.
*/
void printTaskDescriptor(TaskDescriptor taskDesc, RoundRobin roundRobin);

/* Atualiza a memória lógica de uma variável de uma tarefa.
   Esta função atualiza os endereços lógicos de uma variável da tarefa com base no valor alocado.
   Calcula os endereços lógicos inicial e final, bem como atualiza a página final de paginação da tarefa.
   Parâmetros:
   - taskDescriptor: Ponteiro para o descritor de tarefa (TaskDescriptor).
   - index: Índice da variável no array de variáveis da tarefa.
   - value: Valor alocado para a variável.
*/
void updateLogicalMemory(TaskDescriptor* taskDescriptor, int index, int value);

/* Atualiza a memória física de uma variável de uma tarefa.
   Esta função atualiza os endereços físicos de uma variável da tarefa com base no valor alocado.
   Calcula os endereços físicos inicial e final da variável.
   Parâmetros:
   - taskDescriptor: Ponteiro para o descritor de tarefa (TaskDescriptor).
   - index: Índice da variável no array de variáveis da tarefa.
   - value: Valor alocado para a variável.
*/
void updatePhysicalMemory(TaskDescriptor* taskDescriptor, int index, int value);

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
boolean new(String instruction, TaskDescriptor* taskDescriptor, RoundRobin* roundRobin);

/* Imprime os acessos à memória das variáveis de uma tarefa.
   Esta função percorre os acessos à memória registrados na estrutura da tarefa e imprime os endereços lógicos e físicos
   para cada acesso.
   Parâmetros:
   - taskDesc: Descritor da tarefa (TaskDescriptor) contendo os acessos à memória.
*/
void printTaskMemoryAccesses(TaskDescriptor taskDesc);

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
boolean memoryAccess(String instruction, TaskDescriptor* taskDescriptor, RoundRobin* roundRobin);

/* Finaliza uma tarefa.
   Esta função atualiza o status de uma tarefa para finalizada e configura a marca de abortada, se aplicável.
   Atualiza também o tempo de término, fecha o arquivo associado à tarefa e ajusta as métricas globais do sistema.
   Parâmetros:
   - taskDescriptor: Ponteiro para o descritor da tarefa (TaskDescriptor).
   - aborted: Indica se a tarefa foi abortada (TRUE) ou concluída com sucesso (FALSE).
   - roundRobin: Ponteiro para a estrutura RoundRobin contendo informações do sistema.
*/
void finishTask(TaskDescriptor* taskDescriptor, boolean aborted, RoundRobin* roundRobin);

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
void checkAndUpdateSuspendedTasks(TaskDescriptorQueue* queue, TaskDescriptor tasks[], int numberOfTasks, unsigned timeUnits);

/* Arredonda um número float para o inteiro mais próximo.
   Esta função recebe um número em ponto flutuante e retorna o valor arredondado para o inteiro mais próximo.
   Parâmetros:
   - number: Número em ponto flutuante a ser arredondado.
   Retorno:
   - Valor arredondado para o inteiro mais próximo (int).
*/
int roundingNumber(float number);

/* Atualiza a paginação de uma tarefa com base nos bytes alocados.
   Esta função atualiza os valores de paginação de uma tarefa com base na quantidade de bytes alocados.
   Parâmetros:
   - taskDescriptor: Ponteiro para o descritor de tarefa (TaskDescriptor) a ser atualizado.
   - bytes: Quantidade de bytes alocados.
*/
void updatePagination(TaskDescriptor* taskDescriptor, unsigned int bytes);

/* Processa um cabeçalho de instrução para atualizar a paginação de uma tarefa.
   Esta função processa o cabeçalho de uma instrução para atualizar os valores de paginação
   de uma tarefa com base nos bytes especificados no cabeçalho.
   Parâmetros:
   - instruction: String contendo a instrução do cabeçalho.
   - taskDescriptor: Ponteiro para o descritor de tarefa (TaskDescriptor) a ser atualizado.
   - roundRobin: Ponteiro para a estrutura RoundRobin contendo informações do sistema.
   Retorno:
   - Retorna TRUE se a operação for bem-sucedida; FALSE, caso contrário.
*/
boolean header(String instruction, TaskDescriptor* taskDescriptor, RoundRobin* roundRobin);

/* Determina o tipo de instrução baseado em padrões regex.
   Esta função verifica o tipo de instrução com base em padrões regex e retorna
   um enum correspondente ao tipo de instrução encontrado.
   Parâmetros:
   - instruction: String contendo a instrução a ser verificada.
   Retorno:
   - Retorna um enum do tipo Instruction correspondente ao tipo de instrução encontrado,
     ou -1 caso não corresponda a nenhum padrão conhecido.
*/
Instruction determineInstructionType(String instruction);

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
int executeInstruction(TaskDescriptorQueue* queue, RoundRobin* roundRobin, TaskDescriptor* taskDescriptor, TaskDescriptor tasks[], int numberOfTasks);

/* Inicializa a estrutura RoundRobin com valores iniciais.
   Esta função atribui valores iniciais aos campos da estrutura RoundRobin.
   Parâmetros:
   - roundRobin: Ponteiro para a estrutura RoundRobin a ser inicializada.
*/
void initializeRoundRobin(RoundRobin* roundRobin);

/* Inicializa a fila de descritores de tarefa.
   Esta função inicializa a fila de descritores de tarefa, enfileirando as tarefas prontas e não abortadas.
   Parâmetros:
   - taskDescriptorQueue: Ponteiro para a fila de descritores de tarefa a ser inicializada.
   - tasks: Array de descritores de tarefa.
   - numberOfTasks: Número total de tarefas no array.
*/
void initializeTaskQueue(TaskDescriptorQueue* taskDescriptorQueue, TaskDescriptor tasks[], int numberOfTasks);

/* Executa o escalonamento e execução das tarefas.
   Esta função executa o escalonamento e a execução das tarefas enquanto houver tarefas a serem processadas.
   Parâmetros:
   - taskDescriptorQueue: Ponteiro para a fila de descritores de tarefa.
   - roundRobin: Estrutura de informações do Round-Robin.
   - tasks: Array de descritores de tarefa.
   - numberOfTasks: Número total de tarefas no array.
*/
void runTasks(TaskDescriptorQueue* taskDescriptorQueue, RoundRobin* roundRobin, TaskDescriptor tasks[], int numberOfTasks);

/* Agenda e executa as tarefas usando o algoritmo Round-Robin.
   Esta função cria uma estrutura RoundRobin, uma fila de descritores de tarefa e coordena a execução das tarefas
   usando o algoritmo Round-Robin até que todas as tarefas tenham sido executadas.
   Parâmetros:
   - tasks: Array de descritores de tarefa.
   - numberOfTasks: Número total de tarefas no array.
   Retorno:
   - Estrutura RoundRobin com informações de execução das tarefas.
*/
RoundRobin scheduleTasks(TaskDescriptor tasks[], int numberOfTasks);

/* Verifica se uma string corresponde a um padrão de expressão regular.
   Esta função utiliza a biblioteca POSIX para compilar e executar uma expressão regular
   e verifica se a string fornecida corresponde ao padrão especificado.
   Parâmetros:
   - string: A string a ser verificada.
   - pattern: O padrão de expressão regular a ser comparado com a string.
   Retorno:
   - TRUE se a string corresponde ao padrão, FALSE caso contrário.
*/
boolean matchRegex(String string, const char *pattern);

/* Valida o número de argumentos fornecidos ao programa.
   Esta função verifica se o número de argumentos está dentro do intervalo esperado
   para a execução do programa, com base na quantidade de tarefas previamente definida.
   Parâmetros:
   - numberOfArguments: O número de argumentos fornecidos ao programa.
   Retorno:
   - TRUE se o número de argumentos estiver dentro do intervalo esperado, FALSE caso contrário.
*/
boolean validateNumberOfArguments(int numberOfArguments);

/* Inicializa um descritor de tarefa.
   Esta função inicializa um descritor de tarefa com valores padrão e abre o arquivo de tarefa associado.
   Parâmetros:
   - descriptor: O descritor de tarefa a ser inicializado.
   - taskName: O nome da tarefa para identificação.
*/
void initializeTaskDescriptor(TaskDescriptor* descriptor, String taskName);

/* Valida um arquivo de tarefa.
   Esta função verifica se um arquivo de tarefa possui um formato válido, verificando suas instruções.
   Parâmetros:
   - fileName: O nome do arquivo de tarefa a ser validado.
   Retorno:
   - TRUE se o arquivo possui um formato válido, FALSE caso contrário.
*/
boolean validateFile(FileName fileName);

/* Função principal para execução do escalonamento de tarefas e imprimir os relatórios.
   Parâmetros:
   - numberOfTasks: Número de tarefas a serem escalonadas.
   - tasks: Array de strings contendo os nomes dos arquivos de tarefa.
   Retorno:
   - EXIT_SUCCESS se a execução for bem-sucedida, EXIT_FAILURE caso contrário.
*/
int tsmm(int numberOfTasks, char *tasks[]);

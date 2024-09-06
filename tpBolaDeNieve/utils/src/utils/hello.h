#ifndef UTILS_HELLO_H_
#define UTILS_HELLO_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h> 
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include <assert.h>
#include <commons/config.h>
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>

typedef struct {
    uint32_t size;      // Tamaño del payload
    uint32_t offset;    // Desplazamiento dentro del payload
    void* stream;       // Payload
} t_buffer;

typedef enum { // Las voy separando por su módulo
    MENSAJE,
    PAQUETE,
    /* op_codes de KERNEL */
    PETICION_INTERFAZ,
    //Interrupt asi(?)
    INTERRUPT,
    CONTEXT_EXEC_PROCESO,
    SOLICITUD_PROCESO,
    PROCESO,
    INTERFAZ,
    /* op_codes de CPU*/
    PATH_PRUEBA,
    IO_SOLICITADA_GEN_SLEEP,
    FIN_PROCESO,
    FIN_QUANTUM,
    PROGRAM_COUNTER,
    LINEA,
    IO_CPU,
    /* op_codes de IO*/
    INTERFACES_CONECTADAS_IO 
} op_code;

typedef struct { // Struct para enviar un path en la función 1 del kernel
    char* path;
    uint32_t path_length;
} pathPrueba;


typedef struct { // Struct para escribir en la consola y tome la función y lo demás por separado
    char*     nombreFuncion;
    char*     path_or_number;
    uint32_t  path_or_numberLength;
} inputText;

typedef struct { // Struct de los paquetes para enviar cosas a los demás módulos
    op_code codigo_operacion;
    t_buffer* buffer;
} t_paquete;
typedef enum{
    NEW,
    READY,
    BLOCKED,
    EXECUTE,
    EXIT
} estadosProceso;
typedef struct { // Estructura de los registros del CP
    uint8_t AX;
    uint8_t BX;
    uint8_t CX;
    uint8_t DX;
    uint32_t EAX;
    uint32_t EBX;
    uint32_t ECX;
    uint32_t EDX;
    uint32_t SI;
    uint32_t DI;
} t_CPUregisters;

typedef struct {
    char* nombreUnico;
    char* archivoConfig;
    uint32_t interfaz_length;
} interfazIO;

typedef struct {
    char* nombreUnico;
    uint32_t nombre_length;
    char* nombreOp;
    uint32_t nombreOp_length;
    uint32_t unidadesTrabajo; // Esto habría que cambiarlo, ya que habrán otras interfaces que no utilizan esto :P

} peticionIO;
typedef struct { // Estructura del PCB
    char* path;
    uint32_t path_length;
    uint32_t PID;
    uint32_t quantum;
    uint32_t programCounter;
    t_CPUregisters registros;
    estadosProceso estado;
} PCB;

typedef struct {
    char* path;
    uint32_t path_length;
    uint32_t programCounter;
    t_CPUregisters registros;
}contextoEjecución;

void terminar_programa(t_log* logger, t_config* config);

uint32_t* recibirPC(int unSocket);

// CONEXION: Lado Cliente
int crear_conexion(t_log* logger,const char* server_name,char *ip, char* puerto);

t_paquete* crear_paquete(void);

void* serializar_paquete(t_paquete* paquete, int bytes);
void crear_buffer(t_paquete* paquete);
void enviar_mensaje(char* mensaje, int socket_cliente);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);

// CONEXION: Lado Servidor
extern t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_servidor(t_log*, const char* name, char* puerto);
int esperar_cliente(t_log* logger, int socket_servidor);
t_list* recibir_paquete(int);
void recibir_mensaje(t_log* logger, int socket_cliente);
int recibir_operacion(int);
t_config* iniciar_config(void);

void decir_hola(char* quien);

char* recibir_paq(int socket_cliente);

// ==================================================[SEPARADOR DE TEMA]===============================================
/*  [TITULO]: Cabecera de funciones que nos abstraen la de/serialización de las estructuras que usemos.
              Si no entienden cómo usarlas, fijense en la guía de serialización del tp0.
*/

t_buffer *buffer_create(uint32_t size);
void buffer_destroy(t_buffer *buffer);
// Serialización de procesos
void buffer_add_unit32(t_buffer *buffer, uint32_t data);
void buffer_add_unit8(t_buffer *buffer, uint8_t data);
void buffer_add_cpuRegisters(t_buffer *buffer, t_CPUregisters data);
void buffer_add_string(t_buffer *buffer, uint32_t length, char *string);
void buffer_add_int(t_buffer *buffer, int data);

// Serialización de interfaz
void buffer_add_bool(t_buffer *buffer, bool data);

// Por implementarlas bien:
uint32_t buffer_read_unit32(void* stream);
uint8_t buffer_read_unit8(t_buffer *buffer);
t_CPUregisters buffer_read_cpuRegisters(t_buffer *buffer);
char* buffer_read_string(t_buffer *buffer, uint32_t *length);

#endif

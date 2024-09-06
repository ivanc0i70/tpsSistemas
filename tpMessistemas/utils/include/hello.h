#ifndef HELLO_H_
#define HELLO_H_

#include <stdlib.h>
#include <stdio.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>

#include <readline/readline.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <semaphore.h>


/**
* @fn    decir_hola
* @brief Imprime un saludo al nombre que se pase por parámetro por consola.
*/


//Estructuras del cliente del TP0
typedef enum
{
	MENSAJE,
	PAQUETE,
    //IREMOS COLOCANDO MAS VARIABLES SEGUN NECESITEMOS
    //PODEMOS SEPARAR SEGUN MENSAJES ENTRE DISTINTOS MODULOS O
    //CREAR ENUMS DISTINTOS PARA CADA MODULOS

	//OP_CODES KERNEL -> MODULO DESEADO
	CREAR_PROCESO_KERNEL_MEMORIA
	//

}op_code;

typedef enum{
	NEW,
	READY,
	RUNNING,
	BLOCKED,
	SALIDA
}t_estado;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


typedef enum{
	SET,
	MOV_IN,
	MOV_OUT,
	SUM,
	SUB,
	JNZ,
	RESIZE,
	COPY_STRING,
	WAIT,
	SIGNAL,
	IO_GEN_SLEEP,
	IO_STDIN_READ,
	IO_STDOUT_WRITE,
	IO_FS_CREATE,
	IO_FS_DELETE,
	IO_FS_TRUNCATE,
	IO_FS_WRITE,
	IO_FS_READ,
	EXIT,
}t_identificador;


typedef struct {
	uint8_t AX, BX, CX, DX;
	uint32_t PC, EAX, EBX, ECX, EDX, SI, DI;
}registros_cpu;

typedef struct{
	int pid;
	t_estado estado;
	int programCounter;
	int quantum;
	int sleep;
	registros_cpu* registros_cpu;
}t_pcb;

typedef struct {
	t_identificador identificador;
	char* parametro_1;
	char* parametro_2;
	char* parametro_3;
}t_instruccion;


typedef enum{
	INICIAR_PROCESO_CPU,
	INICIAR_PROCESO_KERNEL,
	FINALIZAR_PROCESO,
	ACCESO_A_TABLA,
}opcion_funciones;

typedef struct{
	int pid;
	int programCounter;
	char* parametro_1;
	char* parametro_2;
	char* parametro_3;
	int direccion;
	uint32_t registro_valor;

	opcion_funciones flag;
}memoria_cpu_data;

typedef enum
{
	EXECUTE_CPU,
	INTERRUPT_CPU_EXITO,
	INTERRUPT_CPU_FALLO,
	DESALOJAR_PCB,
	i_SET,
	i_SUM,
	i_SUB,
	i_JNZ,
	i_IO_GEN_SLEEP,
	i_EXIT,
}kernel_cpu_dato;

typedef struct{
	char* nombre;
	int size;
	int pid; //el pid asociado a espacio de memoria
	int pagina;
	opcion_funciones flag;
}kernel_memoria_data;


//Funciones del cliente del TP0
int crear_conexion(char *ip, char* puerto);

//Funciones del servidor del TP0
int iniciar_servidor(char* puerto, t_log* un_log, char* msj_servidor);
int esperar_cliente(int socket_servidor, t_log* un_log, char* msj_servidor);
int recibir_operacion(int socket_cliente);

//Funciones de manejo de buffer

t_buffer* recibir_todo_buffer(int conexion);
void* extraer_choclo_del_buffer(t_buffer* un_buffer);
int extraer_int_del_buffer(t_buffer* un_buffer);
char* extraer_string_del_buffer(t_buffer* un_buffer);
uint32_t extraer_uint32_del_buffer(t_buffer* un_buffer);

//Funciones de carga de uint32, int y strings al buffer


t_buffer* crear_buffer();
void destruir_buffer(t_buffer* un_buffer);
void agregar_choclo_al_buffer(t_buffer* un_buffer, void* choclo, int size_choclo);
void cargar_int_al_buffer(t_buffer* un_buffer, int int_value);
void cargar_uint32_al_buffer(t_buffer* un_buffer, uint32_t un_valor);
void cargar_string_al_buffer(t_buffer* un_buffer, char* un_string);


t_paquete* crear_super_paquete(op_code cod_op, t_buffer* un_buffer);
void destruir_paquete(t_paquete* un_paquete);
void* serializar_paquete(t_paquete* paquete);
void enviar_paquete(t_paquete* paquete, int conexion);


#endif  /* HELLO_H_ */

#ifndef ES_GESTOR_
#define ES_GESTOR_
#include <stdlib.h>
#include <stdio.h>
#include <../../utils/include/hello.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <pthread.h>
#include <readline/readline.h>
#include "inicializar_entsal.h"
#include "entsal_memoria.h"
#include "entsal_kernel.h"


// varieables GLOBALES

extern t_log* entradaSalida_logger; 
extern t_config* entradaSalida_config;
extern pthread_t hilo_entsal_memoria;
extern pthread_t hilo_entsal_kernel;

extern int fd_memoria;
extern int fd_kernel;

extern char* TIPO_INTERFAZ;
extern int TIEMPO_UNIDAD_TRABAJO;
extern char* IP_KERNEL;
extern char* PUERTO_KERNEL;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* PATH_BASE_DIALFS;
extern int BLOCK_SIZE;
extern int BLOCK_COUNT;
extern int RETRASO_COMPACTACION;


#endif  /* ES_GESTOR_ */
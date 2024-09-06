#ifndef M_GESTOR_
#define M_GESTOR_
#include <stdlib.h>
#include <stdio.h>
#include <../../utils/include/hello.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <pthread.h>
#include <dirent.h>
#include <readline/readline.h>
#include "inicializar_memoria.h"
#include "memoria_cpu.h"
#include "memoria_entsal.h"
#include "memoria_kernel.h"
#include "mem_instrucciones.h"

// VARIABLES GLOBALES

extern t_log* memoria_logger; 
extern t_config* memoria_config;
extern pthread_t hilo_memoria_cpu;
extern pthread_t hilo_memoria_kernel;
extern pthread_t hilo_memoria_entsal;

extern int fd_memoria;
extern int fd_entradaSalida;
extern int fd_cpu;
extern int fd_kernel;

extern char* PUERTO_ESCUCHA;
extern int TAM_MEMORIA;
extern int TAM_PAGINA;
extern char* PATH_INSTRUCCIONES;
extern int RETARDO_RESPUESTA;

extern char* PATH_INSTRUCCIONES;

#endif  /* M_GESTOR_ */
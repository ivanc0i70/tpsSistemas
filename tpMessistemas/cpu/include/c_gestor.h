#ifndef C_GESTOR_
#define C_GESTOR_

#include <stdlib.h>
#include <stdio.h>
#include <../../utils/include/hello.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>

#include <readline/readline.h>
#include "inicializar_cpu.h"
#include <pthread.h>
#include "cpu_kerneld.h"
#include "cpu_kerneli.h"
#include "cpu_memoria.h"
#include "ciclo_de_instruccion.h"

extern t_log* cpu_logger;
extern t_config* cpu_t_config;
extern t_log* cpu_debug;
extern pthread_t hilo_cpu_kernel_dispatch;
extern pthread_t hilo_cpu_kernel_interrupt;
extern pthread_t hilo_cpu_memoria;
extern pthread_mutex_t mutex_interrupt;

extern int fd_cpu_interrupt;
extern int fd_cpu_dispatch;
extern int fd_kernel_dispatch;
extern int fd_memoria;
extern int fd_kernel_interrupt;

extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA; 
extern char* PUERTO_ESCUCHA_DISPATCH;
extern char* PUERTO_ESCUCHA_INTERRUPT;
extern int CANTIDAD_ENTRADAS_TLB;
extern char* ALGORITMO_TLB;

extern bool existe_interrupcion;
extern bool enviar_interrupt;



#endif  /* C_GESTOR_ */
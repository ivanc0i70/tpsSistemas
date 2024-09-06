#ifndef K_GESTOR_
#define K_GESTOR_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <readline/readline.h>

#include <../../utils/include/hello.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>


extern t_log* kernel_logger;
extern t_log* kernel_debug; 
extern t_config* kernel_config;

extern pthread_t hilo_kernel_es;
extern pthread_t hilo_kernel_memoria;
extern pthread_t hilo_kernel_cpu_dispatch;
extern pthread_t hilo_kernel_cpu_interrupt;
extern pthread_t hilo_consola;

extern int fd_kernel;
extern int fd_cpu_dispatch;
extern int fd_cpu_interrupt;
extern int fd_memoria;
extern int fd_entsal;

extern char* PUERTO_ESCUCHA;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* IP_CPU;
extern char* PUERTO_CPU_DISPATCH;
extern char* PUERTO_CPU_INTERRUPT;
extern char* ALGORITMO_PLANIFICACION;
extern int QUANTUM;
extern char** RECURSOS;
extern char** INSTANCIAS_RECURSOS;
extern int GRADO_MULTIPROGRAMACION;

extern int identificador_PID;
extern int contador_pcbs;

extern pthread_mutex_t mutex_pid, mlog;


extern bool deadlock;
extern bool ejecucion_pausada;
extern t_pcb* proceso_ejecutando;
extern sem_t new_disponible, ready_disponible, procesos_en_ready,
      nivel_multiprogramacion, procesos_en_exit, bloqueado;
extern t_queue* cola_new;
extern t_queue* cola_ready;
extern t_queue* cola_blocked;
extern t_queue* cola_exit;

#endif  /* K_GESTOR_ */
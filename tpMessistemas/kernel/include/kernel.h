#ifndef KERNEL_H_
#define KERNEL_H_

#include "k_gestor.h"
#include "inicializar_kernel.h"
#include "kernel_cpuDisp.h"
#include "kernel_cpuInt.h"
#include "kernel_es.h"
#include "kernel_memoria.h"
#include "consola.h"
#include "servicios_kernel.h"

t_log* kernel_logger;
t_log* kernel_debug; 
t_config* kernel_config;


// VARIABLES GLOBALES
int fd_kernel;
int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_memoria;
int fd_entsal;

pthread_t hilo_kernel_es;
pthread_t hilo_kernel_memoria;
pthread_t hilo_kernel_cpu_dispatch;
pthread_t hilo_kernel_cpu_interrupt;
pthread_t hilo_consola;

char* PUERTO_ESCUCHA;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* IP_CPU;
char* PUERTO_CPU_DISPATCH;
char* PUERTO_CPU_INTERRUPT;
char* ALGORITMO_PLANIFICACION;
int QUANTUM;
char** RECURSOS;
char** INSTANCIAS_RECURSOS;
int GRADO_MULTIPROGRAMACION;

int identificador_PID=1;
int contador_pcbs=1;

pthread_mutex_t mutex_pid, mlog;
bool deadlock;
bool ejecucion_pausada;
t_pcb* proceso_ejecutando;
sem_t new_disponible, ready_disponible, procesos_en_ready,
      nivel_multiprogramacion, procesos_en_exit, bloqueado;
t_queue* cola_new;
t_queue* cola_ready;
t_queue* cola_blocked;
t_queue* cola_exit;
#endif  /* KERNEL_H_ */
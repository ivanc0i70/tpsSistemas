#ifndef CPU_H_
#define CPU_H_


#include "c_gestor.h"


// variables GLOBALES

t_log* cpu_logger; 
t_log* cpu_debug;
t_config* cpu_t_config;
pthread_t hilo_cpu_kernel_dispatch;
pthread_t hilo_cpu_kernel_interrupt;
pthread_t hilo_cpu_memoria;
pthread_mutex_t mutex_interrupt;

//typedef struct {
int fd_kernel_dispatch;
int fd_kernel_interrupt;
int fd_memoria;
int fd_cpu_dispatch;
int fd_cpu_interrupt;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;
int CANTIDAD_ENTRADAS_TLB;
char* ALGORITMO_TLB;
//}cpu_config;

bool existe_interrupcion = false;
bool enviar_interrupt = false;

#endif  /* CPU_H_ */
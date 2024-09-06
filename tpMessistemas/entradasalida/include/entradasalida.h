#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

#include "es_gestor.h"

// VARIABLES GLOBALES

t_log* entradaSalida_logger; 
t_config* entradaSalida_config;
pthread_t hilo_entsal_kernel;
pthread_t hilo_entsal_memoria;

int fd_memoria;
int fd_kernel;

char* TIPO_INTERFAZ;
int TIEMPO_UNIDAD_TRABAJO;
char* IP_KERNEL;
char* PUERTO_KERNEL;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PATH_BASE_DIALFS;
int BLOCK_SIZE;
int BLOCK_COUNT;
int RETRASO_COMPACTACION;


#endif  /* ENTRADASALIDA_H_ */
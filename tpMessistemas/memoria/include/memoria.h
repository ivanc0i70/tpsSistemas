#ifndef MEMORIA_H_
#define MEMORIA_H_


#include "m_gestor.h"


// variables GLOBALES

t_log* memoria_logger; 
t_config* memoria_t_config;

int fd_memoria;
int fd_entradaSalida;
int fd_cpu;
int fd_kernel;

char* PUERTO_ESCUCHA;
int TAM_MEMORIA;
int TAM_PAGINA;
char* PATH_INSTRUCCIONES;
int RETARDO_RESPUESTA;

char* PATH_INSTRUCCIONES = "../Memoria/Directorio/";

typedef struct
{
    char *puerto_escucha;
    int tam_memoria;
    int tam_pagina;
    char *path_instrucciones;
    int retardo_respuesta;
    int socket_io;
    int socket_cpu;
    int socket_kernel;

}memoria_config;

#endif  /* MEMORIA_H_ */
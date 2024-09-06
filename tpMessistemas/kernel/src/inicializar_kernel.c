#include <../include/inicializar_kernel.h>

void inicializar_kernel(){
    inicializar_logs();
    inicializar_configs();
    imprimir_configs();
}

void inicializar_logs(){
     
    kernel_logger = log_create("kernel.log", "KERNEL_LOG", 1, LOG_LEVEL_INFO);
    if(kernel_logger == NULL) {    
        perror("Algo salio mal con el kernel_logger, no se pudo crear o escuchar el archivo");
        exit(EXIT_FAILURE);
    }

    kernel_debug = log_create("kernel.log", "KERNEL_LOG", 1, LOG_LEVEL_INFO);
    if(kernel_debug == NULL) {    
        perror("Algo salio mal con el kernel_debug, no se pudo crear o escuchar el archivo");
        exit(EXIT_FAILURE);
    }

}

void inicializar_configs(){

    kernel_config = config_create("/home/utnso/tp-2024-1c-Los-Messistemas/kernel/kernel.config");


    if(kernel_config == NULL) {    
        perror("Error al cargar kernel_config");
        exit(EXIT_FAILURE);
    }

    PUERTO_ESCUCHA = config_get_string_value(kernel_config, "PUERTO_ESCUCHA");
    IP_MEMORIA = config_get_string_value(kernel_config, "IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(kernel_config, "PUERTO_MEMORIA");
    IP_CPU = config_get_string_value(kernel_config, "IP_CPU");
    PUERTO_CPU_DISPATCH = config_get_string_value(kernel_config, "PUERTO_CPU_DISPATCH");
    PUERTO_CPU_INTERRUPT = config_get_string_value(kernel_config, "PUERTO_CPU_INTERRUPT");
    ALGORITMO_PLANIFICACION = config_get_string_value(kernel_config, "ALGORITMO_PLANIFICACION");
    QUANTUM = config_get_int_value(kernel_config, "QUANTUM");
    RECURSOS = config_get_array_value(kernel_config, "RECURSOS");
    INSTANCIAS_RECURSOS = config_get_array_value(kernel_config, "INSTANCIAS_RECURSOS");
    GRADO_MULTIPROGRAMACION = config_get_int_value(kernel_config, "GRADO_MULTIPROGRAMACION");

}

void imprimir_configs(){
    log_info(kernel_logger, "PUERTO_ESCUCHA: %s", PUERTO_ESCUCHA);
    log_info(kernel_logger, "IP_MEMORIA: %s", IP_MEMORIA);
    log_info(kernel_logger, "PUERTO_MEMORIA: %s", PUERTO_MEMORIA);
    log_info(kernel_logger, "IP_CPU: %s", IP_CPU);
    log_info(kernel_logger, "PUERTO_CPU_DISPATCH: %s", PUERTO_CPU_DISPATCH);
    log_info(kernel_logger, "PUERTO_CPU_INTERRUPT: %s", PUERTO_CPU_INTERRUPT);
    log_info(kernel_logger, "ALGORITMO_PLANIFICACION: %s", ALGORITMO_PLANIFICACION);
    log_info(kernel_logger, "QUANTUM: %d", QUANTUM);
    log_info(kernel_logger, "RECURSOS: [%s|%s|%s]", RECURSOS[0], RECURSOS[1], RECURSOS[2]);
    log_info(kernel_logger, "INSTANCIAS_RECURSOS: [%s|%s|%s]", INSTANCIAS_RECURSOS[0], INSTANCIAS_RECURSOS[1], INSTANCIAS_RECURSOS[2]);
    log_info(kernel_logger, "GRADO_MULTIPROGRAMACION: %d", GRADO_MULTIPROGRAMACION);
}

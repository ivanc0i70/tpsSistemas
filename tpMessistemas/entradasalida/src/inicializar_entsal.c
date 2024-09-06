#include <../include/inicializar_entsal.h>

void inicializar_entsal(){
    inicializar_logs();
    inicializar_configs();
    imprimir_configs();
}

void inicializar_logs(){

    entradaSalida_logger = log_create("entradaSalida.log", "ENTRADASALIDA_LOG", 1, LOG_LEVEL_INFO);
    if(entradaSalida_logger == NULL) {    
        perror("Algo salio mal con el entraSalida_log, no se pudo crear o escuchar el archivo");
        exit(EXIT_FAILURE);
    }

}

void inicializar_configs(){


    entradaSalida_config = config_create("/home/utnso/tp-2024-1c-Los-Messistemas/entradasalida/entradasalida.config");
    
    if(entradaSalida_config == NULL) {    
        perror("Error al cargar entradaSalida_config");
        exit(EXIT_FAILURE);
    }

    TIPO_INTERFAZ = config_get_string_value(entradaSalida_config, "TIPO_INTERFAZ");
    TIEMPO_UNIDAD_TRABAJO = config_get_int_value(entradaSalida_config, "TIEMPO_UNIDAD_TRABAJO");
    IP_KERNEL = config_get_string_value(entradaSalida_config, "IP_KERNEL");
    PUERTO_KERNEL = config_get_string_value(entradaSalida_config, "PUERTO_KERNEL");
    IP_MEMORIA = config_get_string_value(entradaSalida_config, "IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(entradaSalida_config, "PUERTO_MEMORIA");
    PATH_BASE_DIALFS = config_get_string_value(entradaSalida_config, "PATH_BASE_DIALFS");
    BLOCK_SIZE = config_get_int_value(entradaSalida_config, "BLOCK_SIZE");
    BLOCK_COUNT = config_get_int_value(entradaSalida_config, "BLOCK_COUNT");
    RETRASO_COMPACTACION = config_get_int_value(entradaSalida_config, "RETRASO_COMPACTACION");

}

void imprimir_configs(){

    log_info(entradaSalida_logger, "TIPO_INTERFAZ: %s", TIPO_INTERFAZ);
    log_info(entradaSalida_logger, "TIEMPO_UNIDAD_TRABAJO: %d", TIEMPO_UNIDAD_TRABAJO);
    log_info(entradaSalida_logger, "IP_KERNEL: %s", IP_KERNEL);
    log_info(entradaSalida_logger, "PUERTO_KERNEL: %s", PUERTO_KERNEL);
    log_info(entradaSalida_logger, "IP_MEMORIA: %s", IP_MEMORIA);
    log_info(entradaSalida_logger, "PUERTO_MEMORIA: %s", PUERTO_MEMORIA);
    log_info(entradaSalida_logger, "PATH_BASE_DIALFS: %s", PATH_BASE_DIALFS);
    log_info(entradaSalida_logger, "BLOCK_SIZE: %d", BLOCK_SIZE);
    log_info(entradaSalida_logger, "BLOCK_COUNT: %d", BLOCK_COUNT);
    log_info(entradaSalida_logger, "RETRASO_COMPACTACION: %d", RETRASO_COMPACTACION);

}
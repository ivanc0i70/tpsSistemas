#include <../include/inicializar_memoria.h>   

void inicializar_memoria(){
    inicializar_logs();
    inicializar_configs();
    imprimir_configs();
}

void inicializar_logs(){

    memoria_logger = log_create("memoria.log", "MEMORIA_LOG", 1, LOG_LEVEL_INFO);
    if(memoria_logger == NULL) {    
        perror("Algo salio mal con el memoria_log, no se pudo crear o escuchar el archivo");
        exit(EXIT_FAILURE);
    }

}

void inicializar_configs(){

    memoria_config = config_create("/home/utnso/tp-2024-1c-Los-Messistemas/memoria/memoria.config");
    
    if(memoria_config == NULL) {    
        perror("Error al cargar memoria_config");
        exit(EXIT_FAILURE);
    }


    PUERTO_ESCUCHA = config_get_string_value(memoria_config, "PUERTO_ESCUCHA");
    TAM_MEMORIA = config_get_int_value(memoria_config, "TAM_MEMORIA");
    TAM_PAGINA = config_get_int_value(memoria_config, "TAM_PAGINA");
    PATH_INSTRUCCIONES = config_get_string_value(memoria_config, "PATH_INSTRUCCIONES");
    RETARDO_RESPUESTA = config_get_int_value(memoria_config, "RETARDO_RESPUESTA");

}

void imprimir_configs(){

    log_info(memoria_logger, "PUERTO_ESCUCHA: %s", PUERTO_ESCUCHA);
    log_info(memoria_logger, "TAM_MEMORIA: %d", TAM_MEMORIA);
    log_info(memoria_logger, "TAM_PAGINA: %d", TAM_PAGINA);
    log_info(memoria_logger, "PATH_INSTRUCCIONES: %s", PATH_INSTRUCCIONES);
    log_info(memoria_logger, "RETARDO_RESPUESTA: %d", RETARDO_RESPUESTA);

}

void enviar_tamanio_pagina(){
	
	memoria_cpu_data* data_cpu = malloc(sizeof(memoria_cpu_data));

	data_cpu->programCounter = TAM_PAGINA;
	data_cpu->parametro_1 = "";
	data_cpu->paramatro_2 = "";
	data_cpu->parametro_3 = "";
	data_cpu->pid = -1;
	data_cpu->registro_valor =0;
	data_cpu->direccion = -1;
	log_info(memoria_logger, "enviando tamanio pagina %d", TAM_PAGINA);
	enviar_memoria_cpu(data_cpu,fd_cpu,INICIAR_PROCESO_CPU);

}
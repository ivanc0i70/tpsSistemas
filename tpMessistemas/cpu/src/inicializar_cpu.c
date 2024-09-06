#include <../include/inicializar_cpu.h>

void inicializar_cpu(){
    inicializar_logs();
    inicializar_configs();
    imprimir_configs();
}

void inicializar_logs(){

    cpu_logger = log_create("cpu.log", "CPU_LOG", 1, LOG_LEVEL_INFO);
    if(cpu_logger == NULL) {    
        perror("Algo salio mal con el cpu_log, no se pudo crear o escuchar el archivo");
        exit(EXIT_FAILURE);
    }

}

void inicializar_configs(){

     cpu_t_config = config_create("/home/utnso/tp-2024-1c-Los-Messistemas/cpu/cpu.config");
    
    if(cpu_t_config == NULL) {    
        perror("Error al cargar cpu_config");
        exit(EXIT_FAILURE);
    }

    IP_MEMORIA = config_get_string_value(cpu_t_config, "IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(cpu_t_config, "PUERTO_MEMORIA");
    PUERTO_ESCUCHA_DISPATCH = config_get_string_value(cpu_t_config, "PUERTO_ESCUCHA_DISPATCH");
    PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(cpu_t_config, "PUERTO_ESCUCHA_INTERRUPT");
    CANTIDAD_ENTRADAS_TLB = config_get_int_value(cpu_t_config, "CANTIDAD_ENTRADAS_TLB");
    ALGORITMO_TLB = config_get_string_value(cpu_t_config, "ALGORITMO_TLB");

}

void imprimir_configs(){

    log_info(cpu_logger, "IP_MEMORIA: %s", IP_MEMORIA);
    log_info(cpu_logger, "PUERTO_MEMORIA: %s", PUERTO_MEMORIA);
    log_info(cpu_logger, "PUERTO_ESCUCHA_DISPATCH: %s", PUERTO_ESCUCHA_DISPATCH);
    log_info(cpu_logger, "PUERTO_ESCUCHA_INTERRUP: %s", PUERTO_ESCUCHA_INTERRUPT);
    log_info(cpu_logger, "CANTIDAD_ENTRADAS_TLB: %d", CANTIDAD_ENTRADAS_TLB);
    log_info(cpu_logger, "ALGORITMO_TLB: %s", ALGORITMO_TLB);


}
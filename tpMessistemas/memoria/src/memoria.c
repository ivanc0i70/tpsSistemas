#include <../include/memoria.h>

int main(int argc, char* argv[]) {
    
    inicializar_memoria();

    //iniciar el servidor de memoria 
    fd_memoria = iniciar_servidor(PUERTO_ESCUCHA, memoria_logger, "MEMORIA INICIADA");

    //esperar conexion de cpu
    log_info(memoria_logger, "Esperando a cpu...");
    fd_cpu= esperar_cliente(fd_memoria, memoria_logger, "CPU");

    //esperar conexion de kernel
    log_info(memoria_logger, "Esperando a Kernel...");
    fd_kernel= esperar_cliente(fd_memoria, memoria_logger, "KERNEL");
    enviar_tamanio_pagina();
    
    //esperar conexion de entradasalida
    log_info(memoria_logger, "Esperando a entradasalida...");
    fd_entradaSalida = esperar_cliente(fd_memoria, memoria_logger, "ENTRADASALIDA");

    
	
    //Atender mensajes del KERNEL
    pthread_t hilo_memoria_kernel;
    pthread_create(&hilo_memoria_kernel, NULL, (void*)atender_memoria_kernel, NULL);

    pthread_detach(hilo_memoria_kernel);

    //Atender mensajes de Entrada/Salida  -
    pthread_t hilo_memoria_entsal;
    pthread_create(&hilo_memoria_entsal, NULL, (void*)atender_memoria_entsal, NULL);
    pthread_detach(hilo_memoria_entsal);


	//Atender mensajes del CPU
	pthread_t hilo_memoria_cpu;
    pthread_create(&hilo_memoria_cpu, NULL, (void*)atender_memoria_cpu, NULL);
    pthread_join(hilo_memoria_cpu, NULL);


    //Finaliza Memoria

    return EXIT_SUCCESS;
}

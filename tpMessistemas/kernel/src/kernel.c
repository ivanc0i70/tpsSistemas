#include <../include/kernel.h>

int main(int argc, char* argv[]) {
    
    //iniciar kernel
    inicializar_kernel();
    //iniciar servidor kernel
    fd_kernel = iniciar_servidor(PUERTO_ESCUCHA, kernel_logger, "KERNEL LEVANTADO");
    //conectarse a memoria
    log_info(kernel_logger, "Conectando a memoria...");
    fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
    //conectarse a cpu dispatch
    log_info(kernel_logger, "Conectando a CPU dispatch...");
    fd_cpu_dispatch = crear_conexion(IP_CPU, PUERTO_CPU_DISPATCH);
    //conectarse a cpu interrupt
    log_info(kernel_logger, "Conectando CPU interrupt...");
    fd_cpu_interrupt = crear_conexion(IP_CPU, PUERTO_CPU_INTERRUPT);
    //esperar conexion entrada/salida
    log_info(kernel_logger, "Esperando conexion E/S...");
    fd_entsal = esperar_cliente(fd_kernel, kernel_logger, "ENTRADA/SALIDA");

    //Atender mensajes de Entrada/salida
    pthread_t hilo_kernel_es;
    pthread_create(&hilo_kernel_es, NULL, (void*)atender_kernel_es, NULL);
    pthread_detach(hilo_kernel_es);
   
    //Atender mensajes del CPU disp
    pthread_t hilo_kernel_cpu_dispatch;
    pthread_create(&hilo_kernel_cpu_dispatch, NULL, (void*)atender_kernel_cpuD, NULL);
    pthread_detach(hilo_kernel_cpu_dispatch);

    //Atender mensajes del CPU int
    pthread_t hilo_kernel_cpu_interrupt;
    pthread_create(&hilo_kernel_cpu_interrupt, NULL, (void*)atender_kernel_cpuI, NULL);
    pthread_detach(hilo_kernel_cpu_interrupt);

    //Atender mensajes de la MEMORIA
    pthread_t hilo_kernel_memoria;
    pthread_create(&hilo_kernel_memoria, NULL, (void*)atender_kernel_memoria, NULL);
    pthread_detach(hilo_kernel_memoria);

    //Inicio de la Consola para Ejecucion de Funciones

    iniciar_consola_funciones();
    //pthread_t hilo_consola;
    //pthread_create(&hilo_consola, NULL, (void*)iniciar_consola_funciones, NULL);
    //pthread_join(hilo_consola, NULL);





    log_debug(kernel_debug,"Se ha desconectado de kernell");

    return EXIT_SUCCESS;
}

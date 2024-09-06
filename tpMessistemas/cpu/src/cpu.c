#include <../include/cpu.h>

int main(int argc, char* argv[]) {
    
    inicializar_cpu();
    //levantar cpu dispatch
    fd_cpu_dispatch = iniciar_servidor(PUERTO_ESCUCHA_DISPATCH, cpu_logger, "CPU-DISPATCH LEVANTADO");
    //levantar cpu interrupt
    fd_cpu_interrupt = iniciar_servidor(PUERTO_ESCUCHA_INTERRUPT, cpu_logger, "CPU-INTERRUPT LEVANTADO");
    //conectarse a memoria
    log_info(cpu_logger, "Conectando a memoria...");
    fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
    //esperar conexion kernel al dispatch
    log_info(cpu_logger, "Esperando conexion de KERNEL a DISPATCH...");
    fd_kernel_dispatch = esperar_cliente(fd_cpu_dispatch, cpu_logger, "KERNEL_DISPATCH");
    //esperar conexion kernel al interrupt
    log_info(cpu_logger, "Esperando conexion KERNEL a INTERRUPT...");
    fd_kernel_interrupt = esperar_cliente(fd_cpu_interrupt, cpu_logger, "KERNEL_INTERRUPT");
    
    //Atender mensajes de la MEMORIA
    pthread_t hilo_cpu_memoria;
    pthread_create(&hilo_cpu_memoria, NULL, (void*)atender_cpu_memoria, NULL);
    pthread_detach(hilo_cpu_memoria);
   
    //Atender mensajes del KERNEL-DISPATCH   ---- HECHO
    pthread_t hilo_cpu_kernel_dispatch;
    pthread_create(&hilo_cpu_kernel_dispatch, NULL, (void*)atender_cpu_kernel_dispatch, NULL);
    pthread_detach(hilo_cpu_kernel_dispatch);
    
    //Atender mensajes del KERNEL-INTERRUPT
    pthread_t hilo_cpu_kernel_interrupt;
    pthread_create(&hilo_cpu_kernel_interrupt, NULL, (void*)atender_cpu_kernel_interrupt, NULL);
    pthread_join(hilo_cpu_kernel_interrupt, NULL);

    

    return EXIT_SUCCESS; 

    
}

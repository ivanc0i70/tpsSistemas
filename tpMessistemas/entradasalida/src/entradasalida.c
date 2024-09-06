#include <../include/entradasalida.h>

int main(int argc, char* argv[]) {
    
    //Incializar estructuras de entradasalida
    inicializar_entsal();

    //Conectar como cliente a MEMORIA
    fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);

    //Conectar como cliente a KERNEL
    fd_kernel = crear_conexion(IP_KERNEL, PUERTO_KERNEL);


    //Atender mensajes de la MEMORIA
    pthread_t hilo_entsal_memoria;
    pthread_create(&hilo_entsal_memoria, NULL, (void*)atender_entsal_memoria, NULL);
    pthread_detach(hilo_entsal_memoria);
  

    //Atender mensajes del KERNEL
    pthread_t hilo_entsal_kernel;
    pthread_create(&hilo_entsal_kernel, NULL, (void*)atender_entsal_kernel, NULL);
    pthread_join(hilo_entsal_kernel, NULL);
    
    return EXIT_SUCCESS;
}

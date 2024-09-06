#include <../include/cpu_kerneld.h>

void atender_cpu_kernel_dispatch(){
    bool control_key=1;

    while (control_key) {
		int cod_op = recibir_operacion(fd_kernel_dispatch);
		switch (cod_op) {
            case MENSAJE:
                // recibir_mensaje(cliente_fd);
                break;
            case PAQUETE:
                // lista = recibir_paquete(fd_kernel_dispatch);
                // log_info(cpu_logger, "Me llegaron los siguientes valores:\n");
                // list_iterate(lista, (void*) iterator);
                break;
            case -1:
                log_error(cpu_logger, "El KERNEL-DISPATCH se desconecto. Terminando servidor");
                control_key=0;
                // return EXIT_FAILURE;
                break;
            default:
                log_warning(cpu_logger,"Operacion desconocida de KERNEL-DISPATCH");
                break;
            }
	}
}
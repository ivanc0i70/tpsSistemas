#include <../include/cpu_kerneli.h>

void atender_cpu_kernel_interrupt(){
    bool control_key=1;

    while (control_key) {
		int cod_op = recibir_operacion(fd_kernel_interrupt);
		switch (cod_op) {
            case MENSAJE:
                // recibir_mensaje(cliente_fd);
                break;
            case PAQUETE:
                // lista = recibir_paquete(fd_kernel_INTERRUPT);
                // log_info(cpu_logger, "Me llegaron los siguientes valores:\n");
                // list_iterate(lista, (void*) iterator);
                break;
            case -1:
                log_error(cpu_logger, "El KERNEL-INTERRUPT se desconecto. Terminando servidor");
                control_key=0;
                // return EXIT_FAILURE;
                break;
            default:
                log_warning(cpu_logger,"Operacion desconocida de KERNEL-INTERRUPT");
                break;
            }
	}
}
#ifndef SERVICIOS_KERNEL_H_
#define SERVICIOS_KERNEL_H_

#include "k_gestor.h"

int asignar_pid();
t_pcb* inicializar_pcb();
void inicializar_registros_cpu(t_pcb* pcb);
t_pcb* crear_pcb();
void log_protegido(char* mensaje);
void inicializar_estructuras();
const char* traducir_estado(t_estado estado);



#endif /* SERVICIOS_KERNEL_H_ */
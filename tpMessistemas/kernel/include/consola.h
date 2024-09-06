#ifndef CONSOLA_H_
#define CONSOLA_H_

#include "k_gestor.h"
#include "servicios_kernel.h"


void iniciar_consola_funciones();
bool validacion_de_instruccion_de_consola(char* leido);
void atender_instruccion_validada(char* leido);
void f_iniciar_proceso(t_buffer* un_buffer);

#endif /* CONSOLA_H_ */
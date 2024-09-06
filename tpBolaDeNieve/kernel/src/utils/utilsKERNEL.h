#ifndef FUNCIONES_KERNEL_
#define FUNCIONES_KERNEL_
// ===========================================[SEPARADOR DE TEMA]=============================================
// [TITULO]: Acá definimos al estado del PCB y la cabecera de las funciones creadas por nosotros para el TP.


typedef struct {
    PCB* proceso;
    struct nodoProceso* next;
} NodoProceso;

typedef struct {
    int cantidad;
    NodoProceso* inicio;
    NodoProceso* fin;
} ColaProcesos;

inputText textoIngresado(); // Función de consola (main)
int buscarFuncion(char* text); // Función de consola (main)
bool verificarOperacion(interfazIO* interfazPedida, t_log* logger); // si
interfazIO* obtenerInterfaz(char* stringCompleto);

bool interfazBuscadaPorNombre(t_list* lista_interfaces, char* nombre);


PCB* buscarYEliminarProcesoEnCola(ColaProcesos *cola, int PID);
ColaProcesos* crearCola();
NodoProceso* crearNodoNuevo(PCB* proceso);
void destruirNodo(NodoProceso* cola);
void encolarProceso(ColaProcesos* cola, PCB* proceso);
PCB* desencolarProceso(ColaProcesos *cola);
char* leerCola(ColaProcesos* cola);

#endif
#include <utils/hello.h>

/* . En este archivo vamos a dejar las funciones relacionadas a la comunicación 'estandar' entre los módulos
   . Todo esto hecho junto la lógica del TP0 (ily fran).
*/

void decir_hola(char* quien) {
    printf("Hola desde %s!!\n", quien);
}

void* serializar_paquete(t_paquete* paquete, int bytes){
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int)); desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int)); desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
}

void enviar_mensaje(char* mensaje, int socket_cliente) {
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2 * sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void crear_buffer(t_paquete* paquete) {
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->offset = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void){
	t_paquete* paquete = malloc(sizeof(t_paquete));
	// paquete->codigo_operacion = PAQUETE; Lo dejo comentado; cuando usen esta función, agreguenle donde la usen el op_code
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio){
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);
	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente) {
	int bytes = paquete->buffer->size + 2 * sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

t_config* iniciar_config(void) {
    t_config* nuevo_config = NULL; //supongo para que pueda saltar el error
    nuevo_config = config_create("kernel.config");
    if (nuevo_config == NULL) {
        printf("No se pudo crear el config!");
    close(-1);
}
    return nuevo_config;
}

void eliminar_paquete(t_paquete* paquete){
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

int crear_conexion(t_log* logger, const char* server_name, char* ip, char* puerto) {
    struct addrinfo hints, *servinfo;

    // Init de hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Recibe addrinfo
    getaddrinfo(ip, puerto, &hints, &servinfo);

    // Crea un socket con la informacion recibida (del primero, suficiente)
    int socket_cliente = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

    // Fallo en crear el socket
    if(socket_cliente == -1) {
        log_error(logger, "Error creando el socket para %s:%s", ip, puerto);
        return 0;
    } 
	
    // Error conectando
    if(connect(socket_cliente, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        log_error(logger, "Error al conectar (a %s)", server_name);
        freeaddrinfo(servinfo);
        return 0;
    } 
    freeaddrinfo(servinfo);

    return socket_cliente;
}

void liberar_conexion(int socket_cliente) {
	close(socket_cliente);
}

t_log* logger;
int iniciar_servidor(t_log* logger, const char* name, char* puerto){
    int socket_servidor;
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, puerto, &hints, &servinfo);
	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

	if (socket_servidor == -1) {
		log_error(logger, "Error al crear el socket para: [%s]", name);
		exit(-1);
	} else {
		log_info(logger, "Se creo el socket con éxito en: [%s]!", name);
	}

	// Asociamos el socket a un puerto
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);
	return socket_servidor;
}

int esperar_cliente(t_log* logger, int socket_servidor) {
	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	if(socket_cliente == -1) {
		log_error(logger, "Error al aceptar el cliente");
		exit(-1);
	} else {
		log_info(logger, "Se conecto un cliente!");
	}
	return socket_cliente;
}

int recibir_operacion(int socket_cliente){
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0) {
		return cod_op;
	} else {
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente) {
	void * buffer;
	recv(socket_cliente, size, sizeof(int), MSG_WAITALL); buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(t_log* logger, int socket_cliente) {
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente) {
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size) {
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int)); desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio); desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}
char* recibir_paq(int socket_cliente) {
	int size;
	int desplazamiento = 0;
	void * buffer;
	int tamanio;
	char* valor;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size) {
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		valor = malloc(tamanio);
		memcpy(valor, buffer + desplazamiento, tamanio);
		desplazamiento += tamanio;
		//log_info(logger, "Valor recibido: %s", valor);
	}
	free(buffer);
	return valor;
}

void terminar_programa(t_log* logger, t_config* config){
	log_destroy(logger);
	config_destroy(config);
}

// ==================================================[SEPARADOR DE TEMA]===============================================
/*  [TITULO]: Funciones que nos abstraen la de/serialización de las estructuras que usemos.
              Si no entienden cómo usarlas, fijense en la guía de serialización del tp0.
*/

t_buffer *buffer_create(uint32_t size) {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = size; 
    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);
	return buffer;
}

void buffer_add_unit32(t_buffer *buffer, uint32_t data) {
	memcpy(buffer->stream + buffer->offset, &data, sizeof(uint32_t)); buffer->offset += sizeof(uint32_t);
}

void buffer_add_int(t_buffer *buffer, int data) {
	memcpy(buffer->stream + buffer->offset, &data, sizeof(int)); buffer->offset += sizeof(int);
}

void buffer_add_unit8(t_buffer *buffer, uint8_t data) {
	memcpy(buffer->stream + buffer->offset, &data, sizeof(uint8_t)); buffer->offset += sizeof(uint8_t);
}

void buffer_add_cpuRegisters(t_buffer *buffer, t_CPUregisters data) {
	memcpy(buffer->stream + buffer->offset, &data, sizeof(t_CPUregisters)); buffer->offset += sizeof(t_CPUregisters);
}

void buffer_add_string(t_buffer *buffer, uint32_t length, char* string) {
	memcpy(buffer->stream + buffer->offset, &length, sizeof(uint32_t)); buffer->offset += sizeof(uint32_t);
    memcpy(buffer->stream + buffer->offset, string, length);
}

// Interfaz

void buffer_add_bool(t_buffer *buffer, bool data) {
	memcpy(buffer->stream + buffer->offset, &data, sizeof(bool)); buffer->offset += sizeof(bool);
}

// ==================================================[SEPARADOR DE TEMA]===============================================
/*  [TITULO]: Funciones que no se relacionan con la serialización ni comunicación entre módulos, 
			  y que serán útiles para todos los módulos
*/

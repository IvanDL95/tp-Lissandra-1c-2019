/*
 * MemoryPool.c
 *
 *  Created on: 6 abril 2019
 *      Authors: 	Iván De Luca
 *      			Lucas Cerliani
 *      			Lucas Rosende
 *      			Christian Digiorno
 *      			Alejandro González
 */

// This code doesn't have a license. Feel free to copy.

#include "../src/MemoryPool.h"

#include <stdlib.h>
#include <stdio.h>
#include <API.h>
#include <pthread.h>
#include <bits/time.h>
#include <sys/time.h>

static void administrar_conexion(t_paquete* paquete_recibido, un_socket nuevo_socket);
void inicializar_memoria();
void iniciar_gossiping();
int conectarse_con_FS();
void iniciar_servidor_select();
void crear_nuevo_segmento(char*);
char* buscar_key(tabla_paginas, int key);
static void solicitar_pagina(const char* value);
int hacer_select(un_socket maxfd, fd_set* temp_set, struct timeval* tv);

int main(int argc, char** argv){
	char* pathMemoriaConfig = argv[1];
	logger = log_create("memoria.log", "MemoryPool", 1, LOG_LEVEL_DEBUG);
	log_info(logger, "Iniciando Memoria\n");

	get_configuracion(pathMemoriaConfig);

	switch(conectarse_con_FS()){
		case cop_ok:
			break;
		case -1:
			log_error(logger,"Handshake fallido");
			exit(EXIT_FAILURE);
			break;
		case 1:
			log_error(logger, "Tamaño del Value no recibido");
			enviar(socket_FS,codigo_error,0,NULL);
			exit(EXIT_FAILURE);
			break;
	}

	inicializar_memoria();
	log_info(logger, "Memoria Principal reservada\n");

	crear_nuevo_segmento("Tabla_prueba");
	log_info(logger, "Segmento unico checkpoint 2 creado\n");


	pthread_create(&hilo_consola, NULL, (void*) iniciar_consola, logger);

	pthread_create(&hilo_server, NULL, (void*) iniciar_servidor_select, NULL);

	pthread_create(&hilo_gossiping, NULL, (void*) iniciar_gossiping, NULL);
	log_debug(logger, "Gossiping inicializado\n");

	pthread_join(hilo_consola, NULL);
	pthread_join(hilo_server, NULL);

	terminar_programa(logger, (int*)-1);
	/*
	t_gossip* memory[2];

	memory[0] = (t_gossip*)list_get(tabla_gossiping,0);
	memory[1] = (t_gossip*)list_get(tabla_gossiping,1);

	log_debug(logger, "Puerto memoria %d : %s , IP memoria 0: %s\n", memory[0]->numero_memoria,memory[0]->Puerto,memory[0]->dir_IP);
	log_debug(logger, "Puerto memoria %d : %s , IP memoria 1: %s\n", memory[1]->numero_memoria,memory[1]->Puerto,memory[1]->dir_IP);
	*/
}

void get_configuracion(char* ruta){
	log_info(logger, "Leyendo archivo de configuracion del proceso Memoria\n");

	t_config* archivo_configuracion = config_create(ruta);

	if (archivo_configuracion == NULL) {
		log_error(logger, "Error al abrir Archivo de Configuracion\n");
		return;
	}

	config_MP.PUERTO_ESCUCHA = copy_string(get_campo_config_string(archivo_configuracion,"PUERTO_ESCUCHA"));
	config_MP.IP_FS = copy_string(get_campo_config_string(archivo_configuracion, "IP_FS"));
	config_MP.PUERTO_FS = copy_string(get_campo_config_string(archivo_configuracion,"PUERTO_FS"));
	config_MP.IP_SEEDS = get_campo_config_array(archivo_configuracion, "IP_SEEDS");
	config_MP.PUERTO_SEEDS = get_campo_config_array(archivo_configuracion, "PUERTO_SEEDS");
	config_MP.RETARDO_MEM = get_campo_config_int(archivo_configuracion,"RETARDO_MEM");
	config_MP.RETARDO_FS = get_campo_config_int(archivo_configuracion,"RETARDO_FS");
	config_MP.TAM_MEM = get_campo_config_int(archivo_configuracion,"TAM_MEM");
	config_MP.RETARDO_JOURNAL = get_campo_config_int(archivo_configuracion,"RETARDO_JOURNAL");
	config_MP.RETARDO_GOSSIPING = get_campo_config_int(archivo_configuracion,"RETARDO_GOSSIPING");
	config_MP.MEMORY_NUMBER = get_campo_config_int(archivo_configuracion,"MEMORY_NUMBER");

	puts("\n");
	config_destroy(archivo_configuracion);
}

char* ejecutar_API(command_api operacion, char** argumento){
	log_debug(logger, "Ejecutando la API\n");
	// K&R = D&D
	char* nombre_tabla;
	char* value = malloc(tamanio_value);
	int key;

	switch(operacion){
		case SELECT:
			log_debug(logger, "SELECT %s %s\n", argumento[0], argumento[1]);
			nombre_tabla = malloc(size_of_string(argumento[0]));
			strcpy(nombre_tabla,argumento[0]);
			key = atoi(argumento[1]);

			int _is_equal_segmento(t_segmento* segmento){
				// la lógica del list_find está al revés
				return !strcmp(nombre_tabla,segmento->nombre_tabla);
			}

			t_segmento* segmento_buscado = (t_segmento*)list_find(tabla_segmentos,(void*)_is_equal_segmento);

			//Si el segmento existe ejecuta el if
			if(segmento_buscado != NULL){
				char* new_value = malloc(tamanio_value);
				new_value = buscar_key(segmento_buscado->tabla, key);
				if(new_value != NULL)
					//Si encuentra la key devuelve su value y la retorna, sino sale del if y continua
					return new_value;
			}
			t_list* lista_strings = list_create();
			// for(int i=0; argumento[i] != NULL;i++)
				list_add(lista_strings,argumento[0]);
				list_add(lista_strings,argumento[1]);
			enviar_listado_de_strings(socket_FS,lista_strings,SELECT);
			t_paquete* paquete_recibido = recibir(socket_FS);

			int desplazamiento = 0;
			char* new_value = deserializar_string(paquete_recibido->data,&desplazamiento /* 0 */ );
			solicitar_pagina(new_value);
			return new_value;

			break;
		case INSERT:
			log_debug(logger, "INSERT %s %s\n", argumento[0], argumento[1], argumento[2]);
			nombre_tabla = malloc(size_of_string(argumento[0]));
			strcpy(nombre_tabla,argumento[0]);
			strcpy(value,argumento[2]);
			key = atoi(argumento[1]);

			break;
		case CREATE:
			printf("hacer CREATE\n");
			break;
		case DESCRIBE:
			printf("hacer DESCRIBE\n");
			break;
		case DROP:
			printf("hacer DROP\n");
			break;
		case JOURNAL:
			printf("hacer JOURNAL\n");
			break;
		default:
			log_info(logger,"Paquete no reconocido\n");
	}
	return NULL;
}


/****************** FUNCIONES DE MEMORIA **************************/

void inicializar_memoria(){
	int tamanio_pagina = tamanio_base_pagina + tamanio_value;
	unsigned int cantindad_frames = config_MP.TAM_MEM/tamanio_pagina;
	memoria_principal = calloc(cantindad_frames,tamanio_pagina);
	log_debug(logger, "Malloc memoria exitoso\n");

	tabla_segmentos = list_create();
}

void crear_nuevo_segmento(char* nombre_tabla){
	t_segmento* segmento_nuevo = malloc(sizeof(t_segmento));

	segmento_nuevo->nombre_tabla = malloc(size_of_string(nombre_tabla));
	strcpy(segmento_nuevo->nombre_tabla, nombre_tabla);
	//segmento_nuevo.nombre_tabla = nombre_tabla;
	log_debug(logger, "Tabla: %s\n",segmento_nuevo->nombre_tabla);
	segmento_nuevo->tabla = list_create();


	list_add(tabla_segmentos,segmento_nuevo);
	log_debug(logger, "Nuevo segmento creado\n");
}

char* buscar_key(tabla_paginas tabla, int key){
	log_info(logger,"buscando key\n");
	return "0";
}

static void solicitar_pagina(const char* valor){
	log_info(logger,"Solcitando página para value: %s\n", valor);
}

/****************** CONEXIONES **************************/

int conectarse_con_FS(){
	un_socket socket_recibido;
	do socket_recibido = conectar_a(config_MP.IP_FS, config_MP.PUERTO_FS);
	while(socket_recibido==-1);

	socket_FS = socket_recibido;
	log_info(logger, "Me conecté con Lissandra\n");
	if(!realizar_handshake(socket_FS))
		return -1;

	log_info(logger, "Handshake exitoso!\n");
	t_paquete* paquete_recibido = recibir(socket_FS);

	if(paquete_recibido->codigo_operacion == cop_ok){
		int *p_data = paquete_recibido->data;
		log_info(logger, "Tamaño del Value = %d\n", *p_data);
		enviar(socket_FS,cop_ok,0,NULL);
	}else
		return 1; /* FS aun no devuelve el tamaño del Value */
	liberar_paquete(paquete_recibido);
	return cop_ok;
}


static void administrar_conexion(t_paquete* paquete_recibido, un_socket nuevo_socket){
		if(paquete_recibido->codigo_operacion == cop_handshake){
			log_info(logger, "Realizando handshake con Kernel\n");
			esperar_handshake(nuevo_socket, paquete_recibido);
			/* TODO meter estructura de control dentro de "esperar_handshake(un_socket, paquete)"
			while(paquete_recibido->codigo_operacion != cop_ok){
				log_error(logger,"No se recibió un valor correcto\n");
				paquete_recibido = NULL;
				paquete_recibido = recibir(nuevo_socket);
			}*/
			//liberar_paquete(paquete_recibido);
			//return;
		}else if(paquete_recibido->codigo_operacion < 0)
			return;
		else{
			log_info(logger, "Recibiendo datos del Kernel\n");
			command_api comando = paquete_recibido->codigo_operacion;
			/* envio argumentos como lista de strings */
			int desplazamiento = 0;
			t_list* lista_argumentos = deserializar_lista_strings(paquete_recibido->data, &desplazamiento);
			int tamanio_lista = list_size(lista_argumentos);
			char* argumentos[tamanio_lista];
			for(int i=0;i<tamanio_lista;i++){
				argumentos[i] = list_get(lista_argumentos,i);
			}
			list_destroy(lista_argumentos);
			ejecutar_API(comando, argumentos);
			//free(argumentos);
		}
		liberar_paquete(paquete_recibido);
}

// Cambiar a otro archivo 'global' ???

/*

void iniciar_servidor2(){
	un_socket socket_listener = socket_escucha(IP,config_MP.PUERTO_ESCUCHA);
	log_debug(logger, "Estoy escuchando\n");
    while(1) {  // main accept() loop
    	int new_fd;
        new_fd = aceptar_conexion(socket_listener);
        //if (!fork()) { // Este es el proceso hijo
        //    close(*socket_listener); // El hijo no necesita este descriptor
        log_debug(logger, "Me llego una nueva conexión\n");
        administrar_conexion(new_fd);
        close(new_fd);
        //    exit(0);
        //}
        //close(new_fd);  // El proceso padre no lo necesita
    }
    close(socket_listener);
    pthread_exit(NULL);
}

*/

void iniciar_servidor_select(){
	fd_set readset, tempset;
	struct timeval tv;
	un_socket socket_listener, maxfd;


	socket_listener = socket_escucha(IP,config_MP.PUERTO_ESCUCHA);
	log_debug(logger, "Estoy escuchando\n");

	FD_ZERO(&readset);
	FD_SET(socket_listener, &readset);
	maxfd = socket_listener;

	tv.tv_sec = 30;
	tv.tv_usec = 0;

    while(1) {  // main accept() loop
		memcpy(&tempset, &readset, sizeof(tempset));

		int resultado = hacer_select(maxfd,&tempset,&tv);
		if(resultado == -1)
			continue;

		if (FD_ISSET(socket_listener, &tempset)) {
			un_socket socket_cliente = aceptar_conexion(socket_listener);
			log_debug(logger, "Me llego una nueva conexión\n");
			FD_CLR(socket_listener, &tempset);
			if (socket_cliente < 0)
				continue;
			FD_SET(socket_cliente, &readset);
			maxfd = (maxfd < socket_cliente)?socket_cliente:maxfd;
		}

		for (int socket_select=0; socket_select<maxfd+1; socket_select++) {
			if(FD_ISSET(socket_select, &tempset)){

			t_paquete* paquete_recibido = recibir(socket_select);
			/*
			do {
               result = recv(j, buffer, MAX_BUFFER_SIZE, 0);
            } while (result == -1 && errno == EINTR);
			*/

            if (paquete_recibido->codigo_operacion > 0) {
               log_debug(logger,"Recibí datos desde un socket\n");
               administrar_conexion(paquete_recibido, socket_select);
			}
            else if (paquete_recibido->codigo_operacion == codigo_error) {
				log_debug(logger, "Se cerró un socket y tuve que darle 18 balazos\n");
				close(socket_select);
				FD_CLR(socket_select, &readset);
            }
            else
               log_debug(logger,"Error en recv(): %s\n", strerror(errno));
		}
		}      // end for (j=0;...)
	// end else if (result > 0)
    } // end main while(1)
    close(socket_listener);
    pthread_exit(NULL);
}

void iniciar_gossiping(){
	t_list* tabla_gossiping = list_create();
	int cantidad_seeds;
	for(cantidad_seeds=0;config_MP.PUERTO_SEEDS[cantidad_seeds] != NULL && config_MP.IP_SEEDS[cantidad_seeds] != NULL;cantidad_seeds++);
	log_debug(logger,"Cantidad de seeds: %d\n", cantidad_seeds);
	if(config_MP.IP_SEEDS[cantidad_seeds] != NULL){
		log_error(logger, "Menor cantidad de Puertos que de IP semilla, revisar .cfg\n");
		exit(EXIT_FAILURE);
	}
	if(config_MP.PUERTO_SEEDS[cantidad_seeds] != NULL){
		log_error(logger, "Menor cantidad de IPs que de Puertos semilla, revisar .cfg\n");
		exit(EXIT_FAILURE);
	}

	t_gossip memoria[cantidad_seeds];

	for(int i=0;i<cantidad_seeds;i++){
		memoria[i].numero_memoria = i+1;
		memoria[i].Puerto= copy_string((config_MP.PUERTO_SEEDS)[i]);
		memoria[i].dir_IP = copy_string((config_MP.IP_SEEDS)[i]);
		memoria[i].conexion = -1;

		//if(string_equals_ignore_case(memoria[i]->Puerto,"0") || string_equals_ignore_case(memoria[i].dir_IP,"0"))
		//	log_error(logger, "Distinta cantidad de Puertos e IPs, revisar config\n");
		log_debug(logger,"Memoria: %d, Puerto: %s, IP: %s \n", memoria[i].numero_memoria, memoria[i].Puerto, memoria[i].dir_IP);
		list_add_in_index(tabla_gossiping,i,&(memoria[i]));
	}
	log_debug(logger, "Tabla de gossiping inicializada\n");


	/*
	t_gossip memoria_actual;
	while(1){
		for(int i=0; ;i++){
			list_get(tabla_gossiping,i);
			//conectar_a();
		}
		sleep(config_MP.RETARDO_GOSSIPING);
	}
	*/
	pthread_exit(NULL);
}


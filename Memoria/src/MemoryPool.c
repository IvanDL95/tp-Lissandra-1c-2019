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
#include <sys/time.h>
#include <bits/time.h>
#define TAMANIO_PAGINA (sizeof(int)+sizeof(time_t)+tamanio_value)
#define CANTIDAD_FRAMES config_MP.TAM_MEM/TAMANIO_PAGINA

static void administrar_conexion(t_paquete* paquete_recibido, un_socket nuevo_socket);
void iniciar_gossiping();
void iniciar_servidor_select();

int conectarse_con_FS();
static void inicializar_memoria();
t_segmento* crear_nuevo_segmento(char*);
t_registro* buscar_key(tabla_paginas, int key);
t_pagina* solicitar_pagina(tabla_paginas, const char* value, int key, flag);
static void actualizar_pagina(t_registro*,const char* new_value);
//static int asignar_key(tabla_paginas page_table);
static void algoritmo_reemplazo();

int main(int argc, char** argv){
	logger = log_create("memoria.log", "MemoryPool", 1, LOG_LEVEL_TRACE);
	pthread_mutex_init(&mutex_logger, NULL);
	log_info(logger, "Iniciando Memoria\n");


	get_configuracion(argv[1]); //pathMemoriaConfig

	switch(conectarse_con_FS()){
		case cop_ok:
			break;
		case -1:
			log_error(logger,"Handshake fallido\n");
			exit(EXIT_FAILURE);
			break;
		case -2:
			log_error(logger, "Tamaño del Value no recibido\n");
			enviar(socket_FS,codigo_error,0,NULL);
			exit(EXIT_FAILURE);
			break;
		default:
			log_error(logger, "No sé ni que pasó\n");
			exit(EXIT_FAILURE);
	}

	inicializar_memoria();
	log_info(logger, "Memoria Principal reservada\n");

	/*
	crear_nuevo_segmento("TABLA7");
	log_info(logger, "Segmento unico checkpoint 2 creado\n");
	*/

	pthread_t hilo_server,hilo_consola//,hilo_gossiping
	;
	pthread_create(&hilo_consola, NULL, (void*) iniciar_consola, logger);

	pthread_create(&hilo_server, NULL, (void*) iniciar_servidor_select, NULL);

	//pthread_create(&hilo_gossiping, NULL, (void*) iniciar_gossiping, NULL);
	//log_debug(logger, "Gossiping inicializado\n");

	pthread_join(hilo_consola, NULL);
	pthread_kill(hilo_server, SIGQUIT);
	//pthread_kill(hilo_gossiping, SIGQUIT);


	log_info(logger, "Finaliza el programa\n");

	memoria_principal = NULL;
	free(memoria_principal);
	log_destroy(logger);

	exit(EXIT_SUCCESS);

	//terminar_programa(logger, (int*)-1);
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

	switch(operacion){
	char* nombre_tabla;
	int _is_equal_segmento(t_segmento* segmento){
		// la lógica del list_find está al revés
		log_trace(logger,"Compara %s con %s\n", nombre_tabla, segmento->nombre_tabla);
		return !strcmp(nombre_tabla,segmento->nombre_tabla);
	}
		case SELECT:
		{
			if(argumento[0] == NULL || argumento[1] == NULL)
				return "Campos invalidos";
			//if(!isdigit(argumento[1]))
				//return "Key no numerica";

			nombre_tabla = malloc(size_of_string(argumento[0]));
			string_to_upper(argumento[0]);
			strcpy(nombre_tabla,argumento[0]);
			int key = atoi(argumento[1]);
			// liberar_argumentos(char** argumento);
			pthread_mutex_lock(&mutex_logger);
			log_debug(logger, "SELECT %s %d\n", nombre_tabla, key);
			pthread_mutex_unlock(&mutex_logger);

			if(size_of_string(argumento[1]) > tamanio_value)
				return "Longitud del value muy grande, 'out of bounds'";

			pthread_mutex_lock(&mutex_logger);
			t_segmento* segmento_buscado = (t_segmento*)list_find(tabla_segmentos,(void*)_is_equal_segmento);
			pthread_mutex_unlock(&mutex_logger);
			free(nombre_tabla);

			//Si el segmento NO existe ejecuta el if
			if(segmento_buscado == NULL)
				return "La tabla no existe";

			t_registro* pagina_buscada = buscar_key(segmento_buscado->tabla, key);
			if(pagina_buscada != NULL){
				//Si encuentra la key devuelve su value y la retorna, sino sale del if y continua
				return string_from_format("El value es: %s", pagina_buscada->pagina->value);
			}

			t_list* lista_strings = list_create();
			// for(int i=0; argumento[i] != NULL;i++)
				list_add(lista_strings,argumento[0]);
				list_add(lista_strings,argumento[1]);
			enviar_listado_de_strings(socket_FS,lista_strings,SELECT);
			t_paquete* paquete_recibido = recibir(socket_FS);

			if(paquete_recibido->codigo_operacion == cop_ok){
				int desplazamiento = 0;
				char* new_value = deserializar_string(paquete_recibido->data,&desplazamiento /* 0 */ );
				solicitar_pagina(segmento_buscado->tabla,new_value,key,NO_MODIFICADO);
				return string_from_format("El value es: %s", new_value);
			}
			else if(paquete_recibido->codigo_operacion == codigo_error)
				return "La key solicitada no existe en la tabla";

			return "Recibí cualquier cosa";
		}
		break;

		case INSERT:
		{
			if(argumento[0]== NULL || argumento[1]== NULL || argumento[2] == NULL)
				return "Campos invalidos";
			/*
			if(!isdigit(argumento[1]))
				return "Key no numerica";
			*/
			nombre_tabla = malloc(size_of_string(argumento[0]));
			string_to_upper(argumento[0]);
			strcpy(nombre_tabla,argumento[0]);
			int key = atoi(argumento[1]);
			const char* value = argumento[2];
			// liberar_argumentos(char** argumento);
			if(size_of_string(argumento[1]) > tamanio_value)
				return "Longitud del value muy grande, 'out of bounds'";

			pthread_mutex_lock(&mutex_logger);
			log_debug(logger, "INSERT %s %d %s\n", nombre_tabla, key, value);
			pthread_mutex_unlock(&mutex_logger);

			t_segmento* segmento_buscado = (t_segmento*)list_find(tabla_segmentos,(void*)_is_equal_segmento);

			if(segmento_buscado == NULL)
				segmento_buscado = crear_nuevo_segmento(nombre_tabla);
			else
				log_debug(logger,"Ya existe la tabla\n");

			free(nombre_tabla);

			t_registro* pagina_buscada = buscar_key(segmento_buscado->tabla, key);
			if(pagina_buscada != NULL){
				pthread_mutex_lock(&mutex_logger);
				log_debug(logger,"Key encontrada. Actualizando value y Timestamp\n");
				pthread_mutex_unlock(&mutex_logger);
				actualizar_pagina(pagina_buscada, value);
				return string_from_format("Value Actualizado: %s\n Timestamp: %d", pagina_buscada->pagina->value, pagina_buscada->pagina->timestamp);
			}
			else{
				pthread_mutex_lock(&mutex_logger);
				log_debug(logger,"Key no encontrada\n");
				pthread_mutex_unlock(&mutex_logger);
				t_pagina* nueva_pagina = solicitar_pagina(segmento_buscado->tabla, value, key, MODIFICADO);
				return string_from_format("Nueva key creada: %d\n Value: %s\n Timestamp: %d", nueva_pagina->key, nueva_pagina->value, nueva_pagina->timestamp);
			}
		}
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

static void inicializar_memoria(){
	memoria_principal = calloc(CANTIDAD_FRAMES,TAMANIO_PAGINA);
	for(int i=0;i<CANTIDAD_FRAMES;i++)
		memoria_principal[i].value = malloc(TAMANIO_PAGINA);

	log_debug(logger, "Malloc memoria exitoso\n");

	tabla_segmentos = list_create();
}

t_segmento* crear_nuevo_segmento(char* nombre_tabla){
	t_segmento* segmento_nuevo = malloc(sizeof(t_segmento));

	segmento_nuevo->nombre_tabla = malloc(size_of_string(nombre_tabla));
	strcpy(segmento_nuevo->nombre_tabla, nombre_tabla);
	//segmento_nuevo.nombre_tabla = nombre_tabla;
	log_debug(logger, "Tabla: %s\n",segmento_nuevo->nombre_tabla);
	segmento_nuevo->tabla = list_create();


	list_add(tabla_segmentos,segmento_nuevo);
	log_debug(logger, "Nuevo segmento creado\n");
	return segmento_nuevo;
}

t_registro* buscar_key(tabla_paginas page_table, int key){
	log_debug(logger,"Buscando key\n");

	int _is_equal_key(t_registro* registro){
		// la lógica del list_find está al revés
		log_trace(logger,"Compara %d con %d\n", key, registro->pagina->key);
		return (key == registro->pagina->key);
	}

	if(!list_is_empty(page_table))
		return ((t_registro*)list_find(page_table,(void*)_is_equal_key));
	else
		return NULL;
}

t_pagina* solicitar_pagina(tabla_paginas page_table, const char* valor, int key, flag flag_state){
	log_debug(logger,"Solcitando página para value: %s\n", valor);


	for(int i=0;i<CANTIDAD_FRAMES;i++){
		if(&(memoria_principal[i]) != NULL){
			log_trace(logger,"Frame libre!: %d\n",i);

			memoria_principal[i].key= key;
			strcpy(memoria_principal[i].value, valor);
			memoria_principal[i].timestamp = time(NULL);

			t_registro* nuevo_registro = malloc(sizeof(t_registro));
			nuevo_registro->pagina = &(memoria_principal[i]);
			nuevo_registro->modificado = flag_state;

			list_add(page_table,nuevo_registro);

			return &memoria_principal[i];
		}
		log_trace(logger,"Frame ocupado: %d\n",i);
	}

	log_info(logger,"Todas las páginas están ocupadas. Ejecutando algoritmo de reemplazo\n");
	algoritmo_reemplazo();
	return solicitar_pagina(page_table, valor, key, flag_state);
}

static void actualizar_pagina(t_registro* pagina_encontrada, const char* new_value){
	strcpy(pagina_encontrada->pagina->value, new_value);
	pagina_encontrada->pagina->timestamp = time(NULL);
	pagina_encontrada->modificado = MODIFICADO;
}
/*
static int asignar_key(tabla_paginas page_table){
	if(list_is_empty(page_table))
		return 1;

	int _is_bigger_key(t_registro* x, t_registro* y){
		return x->pagina->key > y->pagina->key;
	}

	t_list* tabla_ordenada_segun_key = list_sorted(page_table,(void*)_is_bigger_key);
	int key_anterior = (((t_registro*)list_get(tabla_ordenada_segun_key, 0))->pagina->key);

	return key_anterior+1;
}
*/


static void algoritmo_reemplazo(){

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
		tamanio_value = *(int *)(paquete_recibido->data);
		log_info(logger, "Tamaño del Value = %d\n", tamanio_value);
		enviar(socket_FS,cop_ok,0,NULL);
	}else
		return -2;
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


void iniciar_servidor_select(){
	fd_set readset, tempset;
	un_socket socket_listener, maxfd;

	socket_listener = socket_escucha(IP,config_MP.PUERTO_ESCUCHA);
	log_debug(logger, "Estoy escuchando\n");

	FD_ZERO(&readset);
	FD_SET(socket_listener, &readset);
	maxfd = socket_listener;


    while(1) {  // main accept() loop

		int resultado = hacer_select(maxfd,&tempset,&readset);
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

            if (paquete_recibido->codigo_operacion >= 0) {
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
    log_info(logger,"Cerrando servidor\n");
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

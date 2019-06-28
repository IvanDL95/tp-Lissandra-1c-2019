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

static void administrar_conexion(t_paquete* paquete_recibido, un_socket nuevo_socket);
//void iniciar_gossiping();
void iniciar_servidor_select();

int conectarse_con_FS();
static void inicializar_memoria();
t_segmento* crear_nuevo_segmento(char*);
static void destruir_segmento(t_segmento*);
t_pagina* buscar_key(t_segmento*, int key);
t_frame* solicitar_pagina(t_segmento*, const char* value, int key, flag);
static void actualizar_pagina(char* tabla,t_pagina*,const char* new_value);
static void actualizar_cola(int key, char* tabla);
//static int asignar_key(tabla_paginas page_table);
static void algoritmo_reemplazo();
static void hacer_journal(flag_full);
static void page_destroyer(t_pagina*);

int main(int argc, char** argv){
	logger = log_create("memoria.log", "MemoryPool", 1, LOG_LEVEL_TRACE);
	pthread_mutex_init(&mutex_logger, NULL);
	log_info(logger, "Iniciando Memoria\n");
	cola_LRU = queue_create();

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

			t_pagina* pagina_buscada = buscar_key(segmento_buscado, key);
			if(pagina_buscada != NULL){
				//Si encuentra la key devuelve su value y la retorna, sino sale del if y continua
				char* selected_value = malloc(tamanio_value);
				memcpy(pagina_buscada->pagina->value,selected_value,tamanio_value);
				return string_from_format("El value es: %s", selected_value);
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
				solicitar_pagina(segmento_buscado,new_value,key,NO_MODIFICADO);
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



			t_pagina* pagina_buscada = buscar_key(segmento_buscado, key);
			if(pagina_buscada != NULL){
				pthread_mutex_lock(&mutex_logger);
				log_debug(logger,"Key encontrada. Actualizando value y Timestamp\n");
				pthread_mutex_unlock(&mutex_logger);
				actualizar_pagina(nombre_tabla,pagina_buscada, value);
				free(nombre_tabla);
				return string_from_format("Value Actualizado: %s\n Timestamp: %d", pagina_buscada->pagina->value, pagina_buscada->pagina->timestamp);
			}
			else{
				pthread_mutex_lock(&mutex_logger);
				log_debug(logger,"Key no encontrada\n");
				pthread_mutex_unlock(&mutex_logger);
				t_frame* nueva_pagina = solicitar_pagina(segmento_buscado, value, key, MODIFICADO);
				free(nombre_tabla);
				return string_from_format("Nueva key creada: %d\n Value: %s\n Timestamp: %d", nueva_pagina->key, nueva_pagina->value, nueva_pagina->timestamp);
			}
		}
		break;

		case CREATE:
		{
			pthread_mutex_lock(&mutex_logger);
			log_debug(logger, "INSERT %s %s %s %s\n", argumento[0], argumento[1], argumento[2], argumento[3]);
			pthread_mutex_unlock(&mutex_logger);

			t_list* lista_argumentos = list_create();
			for(int i=0;i < 4;i++)
				list_add(lista_argumentos, argumento[i]);

			enviar_listado_de_strings(socket_FS,lista_argumentos,SELECT);
			t_paquete* paquete_recibido = recibir(socket_FS);

			if(paquete_recibido->codigo_operacion == cop_ok)
				return "Tabla creada exitosamente";
			else if(paquete_recibido->codigo_operacion == codigo_error)
				return "La tabla ya existe!!";

			return "Recibí cualquier cosa";
		}
		break;

		case DESCRIBE:
			log_debug(logger, "DESCRIBE\n");
			break;
		case DROP:
		{
			log_debug(logger, "DROP\n %s", argumento[0]);
			list_remove_and_destroy_by_condition(tabla_segmentos,(void*)_is_equal_segmento,(void*)destruir_segmento);
			return "Drop exitoso\n";
		}
		break;
		case JOURNAL:
			log_debug(logger, "JOURNAL\n");
			hacer_journal(NOT_FULL);
			return "Journal completado";
			break;
		default:
			log_info(logger,"Paquete no reconocido\n");
	}
	return NULL;
}


/****************** FUNCIONES DE MEMORIA **************************/

static void inicializar_memoria(){
	memoria_principal = calloc(CANTIDAD_FRAMES,TAMANIO_PAGINA);
	for(int i=0;i<CANTIDAD_FRAMES;i++){
		memoria_principal[i].value = malloc(tamanio_value);
	}
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

static void destruir_segmento(t_segmento* segmento_a_destruir){
	t_pagina* pagina_a_destruir = malloc(sizeof(t_pagina));

	int is_seeked_key(t_cola_LRU* cola_element){
		return cola_element->nro_pagina == pagina_a_destruir->pagina->key;
	}

	void cola_destroyer(t_cola_LRU* cola_element){
		free(cola_element->nombre_tabla);
		//free(cola_element);
	}

	for(int i=0;!list_is_empty(segmento_a_destruir->tabla);i++){
		pagina_a_destruir = list_remove(segmento_a_destruir->tabla, i);
		if(pagina_a_destruir == NULL)
			continue;
		list_remove_and_destroy_by_condition(cola_LRU->elements, (void*)is_seeked_key, (void*)cola_destroyer);
	}
	free(pagina_a_destruir);
	list_destroy_and_destroy_elements(segmento_a_destruir->tabla,(void*)page_destroyer);
	return;
}

static void page_destroyer(t_pagina* pagina_a_destruir){
	int x = 0;
	time_t y = 0;
	memcpy(&pagina_a_destruir->pagina->key, &x,sizeof(int));
	memcpy(&pagina_a_destruir->pagina->timestamp, &y,sizeof(time_t));
	//memcpy(cola_element->registro->pagina->value,NULL,0);

	free(pagina_a_destruir->pagina);
	//free(pagina_a_destruir);?
}

t_pagina* buscar_key(t_segmento* segmento_encontrado, int key){
	log_debug(logger,"Buscando key\n");
	/*
	 Ahora busca por índice en la tabla de páginas ya que key == índice lista

	int _is_equal_key(t_pagina* registro){
		// la lógica del list_find está al revés
		log_trace(logger,"Compara %d con %d\n", key, registro->pagina->key);
		return (key == registro->pagina->key);
	}*/
	t_cola_LRU* last_element = (t_cola_LRU*)queue_peek(cola_LRU);
	if(last_element->nro_pagina == key && strcmp(last_element->nombre_tabla, segmento_encontrado->nombre_tabla))
		return list_get(segmento_encontrado->tabla,key);

	if(!list_is_empty(segmento_encontrado->tabla)){
		t_pagina* current_page = list_get(segmento_encontrado->tabla, key);

		//((t_pagina*)list_find(segmento_encontrado->tabla,(void*)_is_equal_key));
		actualizar_cola(current_page->pagina->key,segmento_encontrado->nombre_tabla);
		queue_push(cola_LRU,current_page);
		return current_page;
	}
	else
		return NULL;
}

t_frame* solicitar_pagina(t_segmento* current_segmento, const char* valor, int key, flag flag_state){
	log_debug(logger,"Solcitando página para value: %s\n", valor);


	for(int i=0;i<CANTIDAD_FRAMES;i++){
		if((memoria_principal[i].timestamp) == 0){
			log_trace(logger,"Frame libre!: %d\n",i);
			char* new_valor = malloc(tamanio_value);
			strcpy(new_valor,valor);

			memoria_principal[i].key= key;
			memcpy(memoria_principal[i].value, new_valor, tamanio_value);
			memoria_principal[i].timestamp = time(NULL);

			t_pagina* nuevo_registro = malloc(sizeof(t_pagina));
			nuevo_registro->pagina = &(memoria_principal[i]);
			nuevo_registro->modificado = flag_state;

			list_add_in_index(current_segmento->tabla,key,nuevo_registro);
			actualizar_cola(key,current_segmento->nombre_tabla);

			return &memoria_principal[i];
		}
		log_trace(logger,"Frame ocupado: %d\n",i);
	}

	log_info(logger,"Todas las páginas están ocupadas. Ejecutando algoritmo de reemplazo\n");
	algoritmo_reemplazo();
	return solicitar_pagina(current_segmento, valor, key, flag_state);
}

static void actualizar_pagina(char* tabla,t_pagina* pagina_encontrada, const char* new_value){
	strcpy(pagina_encontrada->pagina->value, new_value);
	pagina_encontrada->pagina->timestamp = time(NULL);
	pagina_encontrada->modificado = MODIFICADO;
	actualizar_cola(pagina_encontrada->pagina->key,tabla);
}

static void actualizar_cola(int key, char* tabla){

	int is_seeked_queue_element(t_cola_LRU* cola_element){
		return (cola_element->nro_pagina == key && strcmp(tabla,cola_element->nombre_tabla));
	}

	t_cola_LRU* elemento = (t_cola_LRU*)list_remove_by_condition(cola_LRU->elements,(void*)is_seeked_queue_element);

	if(elemento != NULL)
		queue_push(cola_LRU,elemento);
	else{
		t_cola_LRU* new_element = malloc(sizeof(t_cola_LRU));
		new_element->nro_pagina = key;
		new_element->nombre_tabla = malloc(size_of_string(tabla));
		strcpy(new_element->nombre_tabla,tabla);

		queue_push(cola_LRU,new_element);
	}

}
/*
static int asignar_key(tabla_paginas page_table){
	if(list_is_empty(page_table))
		return 1;

	int _is_bigger_key(t_pagina* x, t_pagina* y){
		return x->pagina->key > y->pagina->key;
	}

	t_list* tabla_ordenada_segun_key = list_sorted(page_table,(void*)_is_bigger_key);
	int key_anterior = (((t_pagina*)list_get(tabla_ordenada_segun_key, 0))->pagina->key);

	return key_anterior+1;
}
*/


static void algoritmo_reemplazo(){

	t_pagina* ultima_pagina = malloc(sizeof(t_pagina));
	while(!queue_is_empty(cola_LRU)){
		ultima_pagina = queue_pop(cola_LRU);
		if(ultima_pagina->modificado == MODIFICADO)
			continue;

		ultima_pagina->pagina->timestamp = 0;
		ultima_pagina->pagina->key = 0;
		//ultima_pagina->pagina->value = 0;
		return;
	}

	printf("Memoria está FULL, iniciando proceso de journal\n");
	hacer_journal(FULL);
	return;
}

static void hacer_journal(flag_full is_full){
	log_debug(logger, "Acá hay que bloquear todo\n");
	if(is_full){
		//enviar todas las paginas al FS y vaciar todoo
		t_list* listado_a_enviar = list_create();
		t_segmento* current_segmento = malloc(sizeof(t_segmento));
		t_pagina* current_page = malloc(sizeof(t_pagina));

		for(int i=0;!list_is_empty(tabla_segmentos);i++){
			current_segmento = list_remove(tabla_segmentos,i);
			for(int j=0;!list_is_empty(current_segmento->tabla);j++){
				current_page = list_remove(current_segmento->tabla,j);
				//TODO enviar INSERT nom_tabla key value
				list_add(listado_a_enviar,current_segmento->nombre_tabla);
				list_add(listado_a_enviar,string_itoa(current_page->pagina->key));
				list_add(listado_a_enviar,current_page->pagina->value);
			}
		}
		enviar_listado_de_strings(socket_FS,listado_a_enviar,INSERT);
	}else{
		t_list* modified_pages_list = list_create();
		unsigned int cant_segmentos = list_size(tabla_segmentos);
		unsigned int tam_current_page_table;
		t_segmento* current_segmento = malloc(sizeof(t_segmento));
		t_pagina* current_page = malloc(sizeof(t_pagina));

		for(int i=0;i<cant_segmentos;i++){
			current_segmento = list_get(tabla_segmentos,i);
			tam_current_page_table = list_size(current_segmento->tabla);
			for(int j=0;j<tam_current_page_table;j++){
				current_page = list_get(current_segmento->tabla,j);
				if(current_page->modificado == MODIFICADO){
					list_add(modified_pages_list, current_segmento->nombre_tabla);
					list_add(modified_pages_list, string_itoa(current_page->pagina->key));
					list_add(modified_pages_list, current_page->pagina->value);
				}
			}
		}
		enviar_listado_de_strings(socket_FS,modified_pages_list,INSERT);
	}
	return;
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

/*void iniciar_gossiping(){
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



	t_gossip memoria_actual;
	while(1){
		for(int i=0; ;i++){
			list_get(tabla_gossiping,i);
			//conectar_a();
		}
		sleep(config_MP.RETARDO_GOSSIPING);
	}

	pthread_exit(NULL);
}
*/


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

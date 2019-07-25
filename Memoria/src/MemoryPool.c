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
#include "MemoriaPrincipal.h"

#include <stdlib.h>
#include <stdio.h>


//void iniciar_gossiping();

static void thread_log(void (*log_func) (t_log*,char), t_log* log_file, const char* message);
static void inicializar_memoria();

int main(int argc, char** argv){
	logger = log_create("memoria.log", "MemoryPool", 1, LOG_LEVEL_TRACE);
	pthread_mutex_init(&mutex_logger, NULL);
	log_info(logger, "Iniciando Memoria\n");
	cola_LRU = queue_create();
	esta_full_memoria = false;

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

/*********************************** API ****************************************************/

char* ejecutar_API(command_api operacion, char** argumento){
	log_debug(logger, "Ejecutando la API\n");

	switch(operacion){
		case SELECT:
		{
			//if(argumento[0] == NULL || argumento[1] == NULL)
			//	return "Campos invalidos";
			//if(!isdigit(argumento[1]))
				//return "Key no numerica";

			string_to_upper(argumento[0]);
			const char* nombre_tabla = argumento[0];
			const int key = atoi(argumento[1]);

			thread_log((void*)log_debug,logger,string_from_format("SELECT %s %d\n", nombre_tabla, key));

			if(size_of_string(argumento[1]) > tamanio_value)
				return "Longitud del value muy grande, 'out of bounds'";

			t_cola_LRU* ultima_pagina = queue_peek(cola_LRU);

			if(!strcmp(ultima_pagina->segmento->nombre_tabla, nombre_tabla) && ultima_pagina->nro_pagina == key){
				queue_pop(cola_LRU);
				queue_push(cola_LRU,ultima_pagina);
				char* valor = obtener_valor(ultima_pagina->segmento, key);
				return string_from_format("El value es: %s", valor);
			}

			t_segmento* segmento_buscado = buscar_segmento(nombre_tabla);

			//Si el segmento NO existe ejecuta el if
			if(segmento_buscado == NULL)
				return "La tabla no existe";

			t_pagina* pagina_buscada = buscar_key(segmento_buscado, key);
			if(pagina_buscada != NULL){
				//Si encuentra la key devuelve su value y la retorna, sino sale del if y continua
				char* selected_value = obtener_valor(segmento_buscado, key);
				return string_from_format("El value es: %s", selected_value);
			}else{

				t_paquete* paquete_recibido = enviar_FS(argumento, SELECT);

				if(paquete_recibido->codigo_operacion == cop_ok){
					int desplazamiento = 0;
					char* new_value = deserializar_string(paquete_recibido->data,&desplazamiento /* 0 */ );
					solicitar_pagina(segmento_buscado,new_value,key,NO_MODIFICADO);
					return string_from_format("El value es: %s", new_value);
				}else
					if(paquete_recibido->codigo_operacion == codigo_error)
						return "La key solicitada no existe en la tabla";
			}
		}
		break;

		case INSERT:
		{
			string_to_upper(argumento[0]);
			const char* nombre_tabla = argumento[0];
			const int key = atoi(argumento[1]);
			const char* value = argumento[2];

			thread_log((void*)log_debug,logger,string_from_format("INSERT %s %d %s\n", nombre_tabla, key, value));
			if(size_of_string(argumento[1]) > tamanio_value)
				return "Longitud del value muy grande, 'out of bounds'";

			t_cola_LRU* ultima_pagina = queue_peek(cola_LRU);

			if(!strcmp(ultima_pagina->segmento->nombre_tabla, nombre_tabla) && ultima_pagina->nro_pagina == key){
				thread_log((void*)log_debug,logger,string_from_format("Key encontrada. Actualizando value y Timestamp\n"));
				actualizar_pagina(ultima_pagina->segmento,list_get(ultima_pagina->segmento->tabla, key), key, value);
				return string_from_format("Value Actualizado");
			}

			t_segmento* segmento_buscado = buscar_segmento(nombre_tabla);

			if(segmento_buscado != NULL){
				thread_log((void*)log_debug,logger,string_from_format("Ya existe la tabla\n"));
				t_pagina* pagina_buscada = buscar_key(segmento_buscado, key);

				if(pagina_buscada != NULL){
					thread_log((void*)log_debug,logger,string_from_format("Key encontrada. Actualizando value y Timestamp\n"));
					actualizar_pagina(segmento_buscado,pagina_buscada, key, value);
					return string_from_format("Value Actualizado");
				}else{
					thread_log((void*)log_debug,logger,string_from_format("Key no encontrada\n"));
					t_frame* nueva_pagina = solicitar_pagina(segmento_buscado, value, key, MODIFICADO);
					return string_from_format("Nueva key creada");
				}
			}else{
				thread_log((void*)log_debug,logger,string_from_format("No existe la tabla, creandola\n"));
				t_segmento* nuevo_segmento = crear_nuevo_segmento(nombre_tabla);
				t_frame* nueva_pagina = solicitar_pagina(nuevo_segmento, value, key, MODIFICADO);
				return string_from_format("Nueva página creada");
			}
		}
		break;

		case CREATE:
		{
			thread_log((void*)log_debug,logger,string_from_format("INSERT %s %s %s %s\n", argumento[0], argumento[1], argumento[2], argumento[3]));

			t_paquete* paquete_recibido = enviar_FS(argumento, CREATE);

			if(paquete_recibido->codigo_operacion == cop_ok)
				return "Tabla creada exitosamente";
			else if(paquete_recibido->codigo_operacion == codigo_error)
				return "La tabla ya existe!!";

			return "Recibí cualquier cosa";
		}
		break;

		case DESCRIBE:
			log_debug(logger, "DESCRIBE\n");
			thread_log((void*)log_debug,logger, "DESCRIBE\n");
			if(argumento[0] == NULL){
				enviar(socket_FS,DESCRIBE,0,NULL);
			}
			else{
				void* buffer = bufferear(argumento[0]);
				enviar(socket_FS,DESCRIBE,size_of_string(argumento[0])+sizeof(int),buffer);
			}
			t_paquete* paquete_recibido = recibir(socket_FS);

			//enviar(nuevo_socket,paquete_recibido);

			return "DESCRIBE exitoso";
			break;
		case DROP:
		{
			log_debug(logger, "DROP\n %s", argumento[0]);
			destruir_segmento(argumento[0]);


			void* buffer = bufferear(argumento[0]);
			int tamanio_buffer = size_of_string(argumento[0]) + sizeof(int);
			void* buffer2 =  malloc(tamanio_buffer);
			int desplazamiento = 0;
			serializar_string(buffer2,&desplazamiento,argumento[0]);

			printf("tam_buffer: %d, valor desplazamiento: %d",tamanio_buffer, desplazamiento);
			enviar(socket_FS, DROP,tamanio_buffer,buffer);

			t_paquete* paquete_recibido = recibir(socket_FS);

			if(paquete_recibido->codigo_operacion == cop_ok)
				return "Drop exitoso\n";
			else if(paquete_recibido->codigo_operacion == codigo_error)
				return "Drop fallido\n";

			return "Recibí cualquier cosa";
		}
		break;
		case JOURNAL:
			log_debug(logger, "JOURNAL\n");
			hacer_journal();
			return "Journal completado";
			break;
		default:
			log_info(logger,"Paquete no reconocido\n");
	}
	return NULL;
}

static void thread_log(void (*log_func) (t_log*,char), t_log* log_file, const char* message){
	pthread_mutex_lock(&mutex_logger);

	(*log_func)(log_file, message);

	pthread_mutex_unlock(&mutex_logger);
}

static void* bufferear(char* string){
	int desplazamiento = 0;
	int tamanio_buffer = size_of_string(string) + sizeof(int);
	void* buffer = malloc(tamanio_buffer);
	serializar_string(buffer,&desplazamiento,string);
	return buffer;
}

/****************** FUNCIONES DE MEMORIA **************************/

static void inicializar_memoria(){
	memoria_principal = calloc(CANTIDAD_FRAMES,TAMANIO_PAGINA);

	for(int i=0;i<CANTIDAD_FRAMES;i++){
	    frame[i].memAddr = memoria_principal + (i*TAMANIO_PAGINA);
	    frame[i].next = &frame[i+1];
	}
	frame_bitarray = bitarray_create_with_mode(memoria_principal, CANTIDAD_FRAMES, LSB_FIRST);
	log_debug(logger, "Malloc memoria exitoso\n");

	tabla_segmentos = list_create();
}

/************************************* CONEXIONES ******************************************/

t_paquete* enviar_FS(char** argumentos, command_api op_code){
		t_list* lista_argumentos = list_create();
		int i = 0;

		void _add_array_element(char* argumento){
			list_add(lista_argumentos,argumentos[i]);
			i++;
		}

		list_iterate(lista_argumentos,(void*)_add_array_element);
		enviar_listado_de_strings(socket_FS,lista_argumentos,op_code);
		return recibir(socket_FS);
}

static int conectarse_con_FS(){
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

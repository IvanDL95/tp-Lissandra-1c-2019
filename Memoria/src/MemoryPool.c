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

int main(void){

	logger = log_create("memoria.log", "MemoryPool", 1, LOG_LEVEL_DEBUG);
	log_info(logger, "Iniciando Memoria\n");

	get_configuracion();

	conectarse_con_FS();

	inicializar_memoria();
	log_info(logger, "Memoria Principal reservada\n");

	tabla_gossiping = iniciar_gossiping();
	log_debug(logger, "Tabla de gossiping inicializada\n");

	pthread_create(&hilo_consola, NULL, iniciar_consola, logger);
	int socket_listener = socket_escucha(IP,config_MP.PUERTO_ESCUCHA);
	log_debug(logger, "Estoy escuchando\n");
	pthread_create(&hilo_server, NULL, iniciar_servidor, &socket_listener);

	pthread_join(hilo_consola, NULL);
	pthread_join(hilo_server, NULL);

	terminar_programa(logger, &socket_listener);
}

void get_configuracion(){
	log_info(logger, "Leyendo archivo de configuracion del proceso Memoria\n");

	t_config* archivo_configuracion = config_create(pathMemoriaConfig);

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

void conectarse_con_FS(){
	FS = conectar_a(config_MP.IP_FS, config_MP.PUERTO_FS);
	log_info(logger, "Me conecté con Lissandra\n");
	realizar_handshake(FS); //recibir TAMANIO_VALUE
	log_info(logger, "Hadnshake exitoso!\n");
	t_paquete* paquete_recibido = malloc(sizeof(t_paquete));
	paquete_recibido = recibir(FS);

	if(paquete_recibido->codigo_operacion == cop_ok){
		tamanio_value = deserializar_int(paquete_recibido->data, 0);
		log_info(logger, "Tamaño del Value = %d\n", tamanio_value);
	}else
		log_error(logger, "Tamaño del Value no recibido");
	liberar_paquete(paquete_recibido);
}

void analizar_paquete(un_socket nuevo_socket){
	t_paquete* paquete_recibido = recibir(nuevo_socket);
	if(paquete_recibido->codigo_operacion == cop_handshake){
		log_info(logger, "Realizando handshake con Kernel\n");
		esperar_handshake(nuevo_socket, paquete_recibido);
		/* TODO meter estructura de control dentro de "esperar_handshake(un_socket, paquete)"
		while(paquete_recibido->codigo_operacion != cop_ok){
			log_error(logger,"No se recibió un valor correcto\n");
			paquete_recibido = NULL;
			paquete_recibido = recibir(nuevo_socket);
		}*/
		liberar_paquete(paquete_recibido);
		return;
	}else
		//ejecutar_API(paquete_recibido->codigo_operacion/*, paquete_recibido->data*/);
	liberar_paquete(paquete_recibido);
}

int ejecutar_API(command_api operacion, char** argumentos){
	log_debug(logger, "Ejecutando la API\n");
	switch(operacion){
		case SELECT:
			printf("hacer SELECT\n");
			break;
		case INSERT:
			printf("hacer INSERT\n");
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
	return 0;
}

void inicializar_memoria(){
	log_debug(logger, "SIGO ANDANDO\n");
	int tamanio_pagina = sizeof(int) + sizeof(long) + tamanio_value;
	unsigned int cantindad_frames = config_MP.TAM_MEM/tamanio_pagina;
	memoria_principal = calloc(cantindad_frames,tamanio_pagina);
	log_debug(logger, "Malloc memoria no me rompio\n");
	for(int i=0;i<cantindad_frames;i++){
		memoria_principal[i] = malloc(sizeof(t_pagina));
	}
	log_debug(logger, "reservar los frames no me rompio\n");
	tabla_segmentos = list_create();
	tabla_paginas tabla_0 = list_create();
	t_segmento segmento_0 = &tabla_0;
	list_add(tabla_segmentos,segmento_0);
}

void iniciar_servidor(un_socket *socket_listener){
    while(1) {  // main accept() loop
    	int  new_fd;
        new_fd = aceptar_conexion(*socket_listener);
        log_debug(logger, "Me llego una nueva conexión\n");
        analizar_paquete(new_fd);
        close(new_fd);  // El proceso padre no lo necesita
    }
}

t_list* iniciar_gossiping(){
	t_list* lista_inicial = list_create();
	int cantidad_seeds;
	for(cantidad_seeds=0;config_MP.PUERTO_SEEDS[cantidad_seeds] != NULL;cantidad_seeds++)
		log_debug(logger,"Cantidad de seeds: %d", cantidad_seeds);
	puts("\n");
	t_gossip memoria[cantidad_seeds];
	for(int i=0; i<cantidad_seeds;i++){
		memoria[i].Puerto = "0";
		memoria[i].dir_IP = "0";
	}

	for(int i=0;i<cantidad_seeds;i++){
		memoria[i].numero_memoria = i;
		memoria[i].Puerto = copy_string((config_MP.PUERTO_SEEDS)[i]);
		memoria[i].dir_IP = copy_string((config_MP.IP_SEEDS)[i]);
		memoria[i].conexion = NULL;
		if(memoria[i].Puerto == "0" || memoria[i].dir_IP  == "0")
			log_error(logger, "Distinta cantidad de Puertos e IPs, revisar config\n");
		log_debug(logger,"Memoria: %d, Puerto: %s, IP: %s \n", memoria[i].numero_memoria, memoria[i].Puerto, memoria[i].dir_IP);
		list_add(lista_inicial,&(memoria[i]));
	}

	return lista_inicial;
}

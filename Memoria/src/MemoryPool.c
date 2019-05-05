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

	logger = log_create("memoria.log", "MemoryPool", 1, LOG_LEVEL_INFO);
	log_info(logger, "Iniciando Memoria\n");

	get_configuracion();
	tabla_gossiping = list_create();

	conectarse_con_FS();

	log_info(logger, "Me conecté con Lissandra\n");

	inicializar_memoria();
	log_info(logger, "Memoria Principal reservada\n");

	//TODO Error en retorno de pthread. Revisar.
	pthread_create(&hilo_consola, NULL, iniciar_consola, logger);
	int socket_listener = socket_escucha(IP,config_MP.PUERTO_ESCUCHA);
	log_info(logger, "Estoy escuchando\n");
	pthread_create(&hilo_server, NULL, iniciar_servidor, &socket_listener);

	pthread_join(hilo_consola, NULL);
	pthread_join(hilo_server, NULL);

	terminar_programa(logger, &socket_listener);
}

void get_configuracion(){
	log_info(logger, "Leyendo archivo de configuracion del proceso Memoria\n");

	t_config* archivo_configuracion = config_create(pathMemoriaConfig);

	if (archivo_configuracion == NULL) {
		log_info(logger, "Error al abrir Archivo de Configuracion\n");
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
	realizar_handshake(FS); //recibir TAMANIO_VALUE
	t_paquete* paquete_recibido = malloc(sizeof(t_paquete));
	paquete_recibido = recibir(FS);

	if(paquete_recibido->codigo_operacion == cop_ok){
		tamanio_value = deserializar_int(paquete_recibido->data, 0);
	}
	liberar_paquete(paquete_recibido);
}

void administrar_conexion(un_socket nuevo_socket){
	t_paquete* paquete_recibido = recibir(nuevo_socket);
	if(paquete_recibido->codigo_operacion == cop_handshake){
		esperar_handshake(nuevo_socket, paquete_recibido);
		paquete_recibido = NULL;
		/* TODO meter estructura de control dentro de "recibir(un_socket)"
		paquete_recibido = recibir(nuevo_socket);
		while(paquete_recibido->codigo_operacion != cop_ok){
			log_error(logger,"No se recibió un valor correcto\n");
			paquete_recibido = NULL;
			paquete_recibido = recibir(nuevo_socket);
		}*/
		liberar_paquete(paquete_recibido);
		return;
	}
	ejecutar_API(paquete_recibido->codigo_operacion);
	liberar_paquete(paquete_recibido);
}

int ejecutar_API(command_api operacion){
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
	memoria_principal = malloc(sizeof(config_MP.TAM_MEM));
	tabla_segmentos = list_create();
}

void* iniciar_servidor(un_socket *socket_listener){
    //while(1) {  // main accept() loop
    	int  new_fd;
        new_fd = aceptar_conexion(*socket_listener);
        //if (!fork()) { // Este es el proceso hijo
        //    close(*socket_listener); // El hijo no necesita este descriptor
        administrar_conexion(new_fd);
        //    close(new_fd);
        //    exit(0);
        //}
        close(new_fd);  // El proceso padre no lo necesita
    //}
        return NULL;
}

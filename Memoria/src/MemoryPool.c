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


int main(void){

	logger = log_create("memoria.log", "MemoryPool", 1, LOG_LEVEL_INFO);
	log_info(logger, "Iniciando Memoria\n");

	get_configuracion();

	typedef struct {char(x)[maximo_value];} value;


	int socket_FS = crear_socket();

	conectar_a(config_MP.IP_FS, config_MP.PUERTO_FS);
	realizar_handshake(socket_FS); //recibir TAMANIO_VALUE

	//TODO inicializar_memoria();
	int socket_listener = socket_escucha(INADDR_ANY,config_MP.PUERTO_ESCUCHA);

	log_info(logger, "Estoy escuchando\n");

    while(1) {  // main accept() loop
    	int  new_fd;
        new_fd = aceptar_conexion(socket_listener);
        if (!fork()) { // Este es el proceso hijo
            close(socket_listener); // El hijo no necesita este descriptor
            //analizar_paquete(new_fd);
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // El proceso padre no lo necesita
    }

	terminar_programa(logger, &socket_listener);
}

void get_configuracion(){
	log_info(logger, "Levantando archivo de configuracion del proceso Memoria\n");

	t_config* archivo_configuracion = config_create(pathMemoriaConfig);

	if (archivo_configuracion == NULL) {
		log_info(logger, "Error al abrir Archivos de Configuracion\n");
		return;
	}

	config_MP.PUERTO_ESCUCHA = get_campo_config_string(archivo_configuracion,"PUERTO_ESCUCHA");
	config_MP.IP_FS = get_campo_config_string(archivo_configuracion, "IP_FS");
	config_MP.PUERTO_FS = get_campo_config_string(archivo_configuracion,"PUERTO_ESCUCHA");
	config_MP.IP_SEEDS = get_campo_config_array(archivo_configuracion, "IP_SEEDS");
	config_MP.PUERTO_SEEDS = get_campo_config_array(archivo_configuracion, "PUERTO_SEEDS");
	config_MP.RETARDO_MEM = get_campo_config_int(archivo_configuracion,"RETARDO_MEM");
	config_MP.RETARDO_FS = get_campo_config_int(archivo_configuracion,"RETARDO_FS");
	config_MP.TAM_MEM = get_campo_config_int(archivo_configuracion,"TAM_MEM");
	config_MP.RETARDO_JOURNAL = get_campo_config_int(archivo_configuracion,"RETARDO_JOURNAL");
	config_MP.RETARDO_GOSSIPING = get_campo_config_int(archivo_configuracion,"RETARDO_GOSSIPING");
	config_MP.MEMORY_NUMBER = get_campo_config_int(archivo_configuracion,"MEMORY_NUMBER");

	config_destroy(archivo_configuracion);
}

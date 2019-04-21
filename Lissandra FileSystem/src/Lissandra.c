/*
 * Lissandra.c
 *
 *  Created on: 6 abril 2019
 *      Authors: 	Iván De Luca
 *      			Lucas Cerliani
 *      			Lucas Rosende
 *      			Christian Digiorno
 *      			Alejandro González
 */

// This code doesn't have a license. Feel free to copy.

#include "../src/Lissandra.h"

#include <stdlib.h>
#include <stdio.h>



int main(void){

	logger = log_create("lissandra.log", "Lissandra", 1, LOG_LEVEL_INFO);
	log_info(logger, "Iniciando File System\n");

	get_configuracion();

	log_info(logger, "Levantando servidor\n");
	un_socket socket_listener = socket_escucha(INADDR_ANY,config_LS.PUERTO_ESCUCHA);

    log_info(logger, "Estoy escuchando\n");
    while(1) {  // main accept() loop
    	int  new_fd;
        new_fd = aceptar_conexion(socket_listener);
        if (!fork()) { // Este es el proceso hijo
            close(socket_listener); // El hijo no necesita este descriptor
            analizar_paquete(new_fd);
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // El proceso padre no lo necesita
    }

	terminar_programa(logger, &socket_listener);
return 0;
}

void get_configuracion(){
	log_info(logger, "Levantando archivo de configuracion del proceso Lissandra\n");

	t_config* archivo_configuracion = config_create(pathLissandraConfig);

	config_LS.PUERTO_ESCUCHA = get_campo_config_string(archivo_configuracion, "PUERTO_ESCUCHA");
	config_LS.PUNTO_MONTAJE = copy_string(get_campo_config_string(archivo_configuracion, "PUNTO_MONTAJE"));
	config_LS.RETARDO = get_campo_config_int(archivo_configuracion, "RETARDO");
	config_LS.TAMANIO_VALUE = get_campo_config_int(archivo_configuracion, "TAMAÑO_VALUE");
	config_LS.TIEMPO_DUMP = get_campo_config_int(archivo_configuracion, "TIEMPO_DUMP");

	config_destroy(archivo_configuracion);
}

void analizar_paquete(un_socket nuevo_socket){
	t_paquete* paquete_recibido = recibir(nuevo_socket);

	if(paquete_recibido->codigo_operacion == cop_handshake)
		esperar_handshake(nuevo_socket, paquete_recibido);
	liberar_paquete(paquete_recibido);
}


int ejecutar_API(command_api operacion){
	switch(operacion){

		case SELECT:
			printf("hacer SELECT");
			break;
		case INSERT:
			printf("hacer INSERT");
			break;
		case CREATE:
			printf("hacer CREATE");
			break;
		case DESCRIBE:
			printf("hacer DESCRIBE");
			break;
		case DROP:
			printf("hacer DROP");
			break;
		default:
			log_info(logger, "Paquete no reconocido\n");
	}
	return 0;
}


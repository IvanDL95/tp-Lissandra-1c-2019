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

	logger = log_create("log FileSystem.txt", "Lissandra", 1, LOG_LEVEL_INFO);
	log_info(logger, "Iniciando File System\n");

	get_configuracion();

	log_info(logger, "Levantando servidor\n");
	un_socket socket_listener = levantar_servidor(INADDR_ANY,config_LS.PUERTO_ESCUCHA);

    log_info(logger, "Estoy escuchando\n");
    while(1) {  // main accept() loop
    	int sin_size, new_fd;
        sin_size = sizeof(struct sockaddr_in);
        new_fd = aceptar_conexion(socket_listener);
        if (!fork()) { // Este es el proceso hijo
            close(socket_listener); // El hijo no necesita este descriptor
            if (send(new_fd, "Hello, world!\n", 14, 0) == -1)
                perror("send");
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // El proceso padre no lo necesita
    }

	terminar_programa(logger);
return 0;
}

void get_configuracion(){
	log_info(logger, "Levantando archivo de configuracion del proceso Lissandra\n");

	t_config* archivo_configuracion = config_create(pathLissandraConfig);

	config_LS.PUERTO_ESCUCHA = get_campo_config_int(archivo_configuracion, "PUERTO_ESCUCHA");
	config_LS.PUNTO_MONTAJE = copy_string(get_campo_config_string(archivo_configuracion, "PUNTO_MONTAJE"));
	config_LS.RETARDO = get_campo_config_int(archivo_configuracion, "RETARDO");
	config_LS.TAMANIO_VALUE = get_campo_config_int(archivo_configuracion, "TAMAÑO_VALUE");
	config_LS.TIEMPO_DUMP = get_campo_config_int(archivo_configuracion, "TIEMPO_DUMP");
}



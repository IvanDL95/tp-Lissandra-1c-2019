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
#include <Libraries.h>

#include <stdlib.h>
#include <stdio.h>



int main(int argc, char** argv){

	logger = log_create("lissandra.log", "Lissandra", 1, LOG_LEVEL_INFO);
	log_info(logger, "Iniciando File System\n");

	get_configuracion(argv[1]);

	log_info(logger, "Levantando servidor\n");
	//TODO uso una IP definida o INADDR_ANY?

	un_socket socket_servidor = socket_escucha(IP,config_LS.PUERTO_ESCUCHA);

    log_info(logger, "Estoy escuchando\n");
	//un_socket nuevo_cliente;

    while(1) {  // main accept() loop
        un_socket nuevo_cliente = aceptar_conexion(socket_servidor);
    	//nuevo_cliente = aceptar_conexion(socket_servidor);
        if (!fork()) { // Este es el proceso hijo
            close(socket_servidor); // El hijo no necesita este descriptor
            analizar_paquete(nuevo_cliente);
            close(nuevo_cliente);
            exit(0);
        }
        close(nuevo_cliente);  // El proceso padre no lo necesita
    }
	terminar_programa(logger, &socket_servidor);
}

void get_configuracion(char* ruta){
	log_info(logger, "Levantando archivo de configuracion del proceso Lissandra\n");

	t_config* archivo_configuracion = config_create(ruta);

	config_LS.PUERTO_ESCUCHA = copy_string(get_campo_config_string(archivo_configuracion, "PUERTO_ESCUCHA"));
	config_LS.IP_MEM = copy_string(get_campo_config_string(archivo_configuracion, "IP_MEM"));
	config_LS.PUERTO_MEM = copy_string(get_campo_config_string(archivo_configuracion,"PUERTO_MEM"));
	config_LS.PUNTO_MONTAJE = copy_string(get_campo_config_string(archivo_configuracion, "PUNTO_MONTAJE"));
	config_LS.RETARDO = get_campo_config_int(archivo_configuracion, "RETARDO");
	config_LS.TAMANIO_VALUE = get_campo_config_int(archivo_configuracion, "TAMAÑO_VALUE");
	config_LS.TIEMPO_DUMP = get_campo_config_int(archivo_configuracion, "TIEMPO_DUMP");

	puts("\n");
	config_destroy(archivo_configuracion);
}

int conectarse_con_Memoria(){
	socket_MEM = conectar_a(config_LS.IP_MEM, config_LS.PUERTO_MEM);
	log_info(logger, "Me conecté con Memoria\n");
	if(!realizar_handshake(socket_MEM))
		return -1;
	log_info(logger, "Handshake exitoso!\n");
	t_paquete* paquete_recibido = recibir(socket_MEM);

	if(paquete_recibido->codigo_operacion == cop_ok){
		tamanio_value = deserializar_int(paquete_recibido->data, 0);
		log_info(logger, "Tamaño del Value = %d\n", tamanio_value);
	}else
		return 1;
	liberar_paquete(paquete_recibido);
	return cop_ok;
}

void analizar_paquete(un_socket nuevo_socket){
	t_paquete* paquete_recibido = recibir(nuevo_socket);

	if(paquete_recibido->codigo_operacion == cop_handshake)
		esperar_handshake(nuevo_socket, paquete_recibido);
	liberar_paquete(paquete_recibido);
}


int ejecutar_API(command_api operacion, char** argumentos){
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

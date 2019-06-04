/*
 * Lissandra.h
 *
 *  Created on: 6 abril 2019
 *      Authors: 	Iván De Luca
 *      			Lucas Cerliani
 *      			Lucas Rosende
 *      			Christian Digiorno
 *      			Alejandro González
 */

#ifndef SRC_LISSANDRA_H_
#define SRC_LISSANDRA_H_

#include <Libraries.h>
#include <API.h>
#include <commons/log.h>

#define IP "127.0.0.1"


t_log* logger;

typedef struct {
	char* PUERTO_ESCUCHA;
	char* PUNTO_MONTAJE;
	unsigned int RETARDO;
	unsigned int TAMANIO_VALUE;
	unsigned int TIEMPO_DUMP;
} Configuracion_LS;

Configuracion_LS config_LS;

un_socket socket_MEM;

pthread_t hilo_Server;
pthread_attr_t hilo_attr_Server;
pthread_mutex_t mutex_mensaje;
pthread_mutex_t mutex_logger;

#endif /* SRC_LISSANDRA_H_ */




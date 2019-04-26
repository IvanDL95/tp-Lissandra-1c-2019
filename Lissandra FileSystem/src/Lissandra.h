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


t_log* logger;
char* pathLissandraConfig = "LissandraConfig.cfg";

typedef struct {
	uint16_t PUERTO_ESCUCHA;
	char* PUNTO_MONTAJE;
	unsigned int RETARDO;
	unsigned int TAMANIO_VALUE;
	unsigned int TIEMPO_DUMP;
} Configuracion_LS;

Configuracion_LS config_LS;

#endif /* SRC_LISSANDRA_H_ */




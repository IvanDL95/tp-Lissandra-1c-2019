/*
 * Filesystem.h
 *
 *  Created on: 1 jun. 2019
 *      Author: utnso
 */

#ifndef SRC_FILESYSTEM_H_
#define SRC_FILESYSTEM_H_
#include "Lissandra.h"


typedef struct{
	int tamanio;
	char* bloques;

}tParticion;

typedef struct{

	char* consistency;
	int particiones;
	int tiempoCompactacion;

}tInfoMetadata;


typedef struct{
	int timestamp;
	int clave;
	char* valor;

}tEntradaBloque;

char* pathTablas;
char* pathBloques;

#endif /* SRC_FILESYSTEM_H_ */

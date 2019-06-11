/*
 * Planificador.h
 *
 *  Created on: 9 jun. 2019
 *      Author: Alejandro González
 */

#ifndef SRC_PLANIFICACION_H_
#define SRC_PLANIFICACION_H_

typedef enum{
	NEW,
	READY,
	EXEC,
	EXIT
} estadosDeScript;

typedef struct{
	int scriptID;
	char* rutaDelArchivo;
	int punteroProximaLinea;
	estadosDeScript estado;
}t_SCB; //Script Control Block


#endif /* SRC_PLANIFICACION_H_ */

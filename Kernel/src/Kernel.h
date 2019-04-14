/*
 * Kernel.h
 *
 *  Created on: 6 abril 2019
 *      Authors: 	Iván De Luca
 *      			Lucas Cerliani
 *      			Lucas Rosende
 *      			Christian Digiorno
 *      			Alejandro González
 */

#ifndef SRC_KERNEL_H_
#define SRC_KERNEL_H_

#include <Libraries.h>
#include <commons/log.h>

t_log* logger;
char* pathKernelconfig = "KernelConfig.cfg";

typedef struct {
	char* IP_MEMORIA;
	int PUERTO_MEMORIA;
	int QUANTUM;
	int MULTIPROCESAMIENTO;
	int METADATA_REFRESH;
} Configuracion_Kernel;

Configuracion_Kernel config_Kernel;

#endif /* SRC_KERNEL_H_ */


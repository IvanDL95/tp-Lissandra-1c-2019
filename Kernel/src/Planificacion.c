/*
 * Planificacion.c
 *
 *  Created on: 10 jun. 2019
 *      Author: Alejandro González
 */

#include "Planificacion.h"

pthread_mutex_t lockPlanificador = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queuePlanificador = PTHREAD_COND_INITIALIZER;
int scriptID = 0;

int planificador(int config[]) {

	int configQuantum;
	int configMultiprocesamiento;
	int execOK;

	configQuantum = config[0];
	configMultiprocesamiento = config[1];

	t_SCB* requestPlanificar;
	t_SCB* requestReady;
	t_SCB* requestExec;
	t_SCB* requestExit;

	colaNew = list_create();
	colaReady = list_create();
	colaExec = list_create();
	colaExit = list_create();

	// Espero a que haya un Script a Planificar
	while(1) {
		printf("\nSe bloquea el Hilo hasta que tenga algo a Planificar\n");

	    pthread_mutex_lock(&lockPlanificador);
		pthread_cond_wait(&queuePlanificador, &lockPlanificador);

		requestPlanificar = list_remove(colaNew, 0);

		list_add(colaReady, requestPlanificar);
		requestReady = list_get(colaReady, 0);

		// Si tengo scripts a Planificar en Ready o Exec
		while(list_size(colaReady) > 0 || list_size(colaExec) > 0) {
			//printf("\nSe Desbloquea el Hilo - Tengo un script a Planificar\n");
			printf("\n ------------------------ \n");
			printf("\nCantidad de scripts a Planificar : %i \n", list_size(colaReady));
			int cont = 1;

			requestExec = list_remove(colaReady, 0);
			list_add(colaExec, requestExec);
			while (cont <= configQuantum ) {

				printf("\n ------------------------ \n");
				printf("\nCantidad en Cola Ready : %i \n", list_size(colaReady));
				printf("\nCantidad en Cola Exec : %i \n", list_size(colaExec));
				if (requestExec->tipo == REQUEST_API) {
					//Ejecuto la request

					execOK = ejecutarRequest(requestExec->dataRequest);
					if (execOK == 0) {
						requestExit = list_remove(colaExec, 0);
						list_add(colaExit, &requestExit);
						break;
					}

				}
				cont++;
			}

			//break;

		}
		pthread_mutex_unlock(&lockPlanificador);

	}
	return 0;
}

int planificarRequest(t_requestAMemoria dataRequest) {

	printf("\nEnvio al Planificador\n");
	data = crearEstructuraPlanificacion(dataRequest);
	list_add(colaNew, &data);
	pthread_cond_signal(&queuePlanificador);
	return 0;
}

t_SCB crearEstructuraPlanificacion(t_requestAMemoria request) {
	t_SCB requestSCB;
	scriptID++;
	requestSCB.scriptID = scriptID;
	requestSCB.tipo = REQUEST_API;
	requestSCB.dataRequest = request;
	requestSCB.rutaDelArchivo = NULL;
	requestSCB.punteroProximaLinea = 1;
	requestSCB.estado = NEW;


	return requestSCB;
}

int ejecutarRequest(t_requestAMemoria requestExec) {

	if(requestExec.comando >= 0 && requestExec.comando <= 5) {
		enviar_listado_de_strings(requestExec.socketMemoria, requestExec.listaArgumentos, requestExec.comando);
		return 0;
	}
	return -1;
}

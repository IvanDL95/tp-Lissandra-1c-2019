/*
 * Kernel.c
 *
 *  Created on: 6 abril 2019
 *      Authors: 	Iván De Luca
 *      			Lucas Cerliani
 *      			Lucas Rosende
 *      			Christian Digiorno
 *      			Alejandro González
 */

// This code doesn't have a license. Feel free to copy.

#include "../src/Kernel.h"

#include <stdlib.h>
#include <stdio.h>

#include "Globals.h"


int main(int argc, char** argv){

	int arg_planificacion[2];
	logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_INFO);
	log_info(logger, "Iniciando Kernel\n");

	get_configuracion(argv[1]);

	if (conectar_con_Memoria() == -1) return -1;

	asignar_Criterios_Memoria();


	arg_planificacion[0] = config_Kernel.QUANTUM;
	arg_planificacion[1] = config_Kernel.MULTIPROCESAMIENTO;
	pthread_create(&thread_planificador, NULL, (void*) planificador, arg_planificacion);

	iniciar_consola(logger);


	terminar_programa(logger,NULL);
}

void get_configuracion(char* ruta){
	log_info(logger, "Levantando archivo de configuracion del proceso Kernel\n");

	t_config* archivo_configuracion = config_create(ruta);

	if (archivo_configuracion == NULL) {
		log_info(logger, "Error al abrir Archivos de Configuracion");
		return;
	} else {
		config_Kernel.IP_MEMORIA = copy_string(get_campo_config_string(archivo_configuracion, "IP_MEMORIA"));
		config_Kernel.PUERTO_MEMORIA = copy_string(get_campo_config_string(archivo_configuracion, "PUERTO_MEMORIA"));
		config_Kernel.QUANTUM = get_campo_config_int(archivo_configuracion, "QUANTUM");
		config_Kernel.MULTIPROCESAMIENTO = get_campo_config_int(archivo_configuracion, "MULTIPROCESAMIENTO");
		config_Kernel.METADATA_REFRESH = get_campo_config_int(archivo_configuracion, "METADATA_REFRESH");
	}

	puts("\n");
	config_destroy(archivo_configuracion);
	return;
}

char* ejecutar_API(command_api operacion, char** argumentos){
	log_debug(logger, "Ejecutando la API\n");
	t_list * lista_argumentos = list_create();
	int i = 0;
	while(argumentos[i] != NULL) {
		list_add(lista_argumentos, argumentos[i]);
		i++;
	}

	requestAPlanificar = crearEstructuraRequest(lista_argumentos, operacion);

	if (planificarRequest(requestAPlanificar) != 0 )
		log_error(logger, "Error al Planificar Request");

	switch(operacion){
		case SELECT:
			log_info(logger, "Planificando Request Select \n");
			//enviar_listado_de_strings(socket_Memoria, lista_argumentos, SELECT);
			//enviarAPLanificador();
			break;
		case INSERT:
			log_info(logger, "Enviando comando INSERT a la Memoria\n");
			enviar_listado_de_strings(socket_Memoria, lista_argumentos, INSERT);
			break;
		case CREATE:
			log_info(logger, "Enviando comando CREATE a la Memoria\n");
			enviar_listado_de_strings(socket_Memoria, lista_argumentos, CREATE);
			break;
		case DESCRIBE:
			log_info(logger, "Enviando comando DESCRIBE a la Memoria\n");
			enviar_listado_de_strings(socket_Memoria, lista_argumentos, DESCRIBE);
			break;
		case DROP:
			log_info(logger, "Enviando comando DROP a la Memoria\n");
			enviar_listado_de_strings(socket_Memoria, lista_argumentos, DROP);
			break;
		case JOURNAL:
			log_info(logger, "Ejecutar JOURNAL en cada Memoria Asociada\n");
			break;
		case ADD:
			printf("\nEjecutando ADD\n");
			break;
		case RUN:
			log_info(logger, "Ejecutando RUN - Parseando Archivo LQL");
			//pthread_create(&thread_parser, NULL, (void*) parsear_archivo_lql, argumentos[0]);
			parsear_archivo_lql(argumentos[0]);

			break;
		case METRICS:
			printf("\nEjecutando METRICS\n");
			mostrarMetricas();
			break;
		default:
			printf("\nComando no reconocido\n\n");
	}
	return NULL;
}

int conectar_con_Memoria(){
	socket_Memoria = conectar_a(config_Kernel.IP_MEMORIA, config_Kernel.PUERTO_MEMORIA);

	if(socket_Memoria != -1){
		log_info(logger, "Conectado a la Memoria en %s:%s / socket:%d",config_Kernel.IP_MEMORIA,config_Kernel.PUERTO_MEMORIA,socket_Memoria);
	}else{
		log_error(logger, "No se pudo conectar a la Memoria.\n");
		return -1;
	}

	if (realizar_handshake(socket_Memoria)) //recibir TAMANIO_VALUE
		log_info(logger,"Handshake con Memoria : Realizado");
	else
		log_error(logger, "Handshake con Memoria : No Realizado");
	t_paquete* paquete_recibido = malloc(sizeof(t_paquete));
	/* Ver de recibir el paquete de vuelta */
//	paquete_recibido = recibir(socket_Memoria);
//
//	if(paquete_recibido->codigo_operacion == cop_ok){
//			tamanio_value = deserializar_int(paquete_recibido->data, 0);
//	}
	liberar_paquete(paquete_recibido);
	return 0;
}

void asignar_Criterios_Memoria() {


}


//------------ Funciones de API ------------//

void parsear_archivo_lql(char* path_archivo_lql) {
	char sentencia_lql[1000];
	char* comando[20];
	char* string_value;
	char* aux;
	FILE *archivo_lql;
	t_SCB SCB;

	if ((archivo_lql = fopen(path_archivo_lql, "r")) == NULL)
	{
		log_error(logger, "No se pudo abrir el Archivo : %s", path_archivo_lql);
		// Program exits if file pointer returns NULL.
		//return;
	}

	SCB.estado = NEW;
	SCB.punteroProximaLinea = 1;
	SCB.rutaDelArchivo = archivo_lql;
	SCB.scriptID = 1;

	//enviarAPlanificador(SCB);


	while ( fgets ( sentencia_lql, sizeof(sentencia_lql), archivo_lql ) != NULL ) {
		printf("Data from the file: %s \n", sentencia_lql);
		log_info(logger, sentencia_lql);
		aux = calloc(strlen(sentencia_lql)+1, sizeof(char));
		strcpy(aux, sentencia_lql);
		int j=0;
		comando[j] = strtok(sentencia_lql, " ");
		while(comando[j] != NULL && j < 20) {
			j++;
			comando[j] = strtok(NULL, " ");
		}

		char* argumentos[4];
		for(int i=0;i<4 && comando[i+1] != NULL;i++){
			argumentos[i] = comando[i+1];
		}
		string_to_upper(comando[0]);

		command_api operacion = convertir_commando(comando[0]);

		if(operacion == INSERT) {
			string_value = strtok(aux, "\"");
			string_value = strtok(NULL, "\"");
			strcpy(argumentos[2], string_value);
			argumentos[3] = NULL;
		}

		ejecutar_API(operacion, argumentos);
	}
//	while ( archivo_lql != NULL ) {
//		fscanf(archivo_lql,"%[^\n]", sentencia_lql);
//
//  }
	fclose(archivo_lql);
	//return;
}


void mostrarMetricas() {
	printf("\x1b[32m////////////////////////////////////////////////////////\n\t\t\tMetricas\x1b[0m\n");
	printf("\x1b[32m////////////////////////////////////////////////////////\x1b[0m\n");
	printf("\x1b[33mRead Latency / 30s :\x1b[0m \n");
	//printf("%.2f \n\n",readLatency);
	printf("\x1b[33mWrite Latency / 30s :\x1b[0m \n");
	//printf("%.2f \n\n",writeLatency);
	printf("\x1b[33mReads / 30s :\x1b[0m \n");
	//printf("%d \n\n",cantReads);
	printf("\x1b[33mWrites / 30s :\x1b[0m \n");
	//printf("%d \n\n",cantWrites);
	printf("\x1b[33mMemory Load :\x1b[0m \n");
	//printf("%d \n\n",memoryLoad);
	printf("\x1b[32m////////////////////////////////////////////////////////\x1b[0m\n");
}

t_requestAMemoria crearEstructuraRequest(t_list* argumentosRequest, command_api comandoRequest) {
	t_requestAMemoria dataRequest;
	dataRequest.socketMemoria = socket_Memoria;
	dataRequest.listaArgumentos = argumentosRequest;
	dataRequest.comando = comandoRequest;
	return dataRequest;
}


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



int main(void){

	logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_INFO);
	log_info(logger, "Iniciando Kernel\n");

	get_configuracion();

	if (conectar_con_Memoria() == -1) return -1;

	iniciar_consola(logger); //Moví la estructura de consola a otro archivo, testeado y funciona


	terminar_programa(logger,NULL);
}

void get_configuracion(){
	log_info(logger, "Levantando archivo de configuracion del proceso Kernel\n");

	t_config* archivo_configuracion = config_create(pathKernelconfig);

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

int ejecutar_API(char** comando){
	command_api operacion = convertir_commando(comando[0]);
	switch(operacion){
		case SELECT:
			printf("\nEjecutando SELECT\n");

			break;
		case INSERT:
			printf("\nEjecutando INSERT\n");
			break;
		case CREATE:
			printf("\nEjecutando CREATE\n");
			break;
		case DESCRIBE:
			printf("\nEjecutando DESCRIBE\n");
			break;
		case DROP:
			printf("\nEjecutando DROP\n");
			break;
		case JOURNAL:
			printf("\nEjecutando JOURNAL\n");
			break;
		case ADD:
			printf("\nEjecutando ADD\n");
			break;
		case RUN:
			printf("\nEjecutando RUN\n");
			break;
		case METRICS:
			printf("\nEjecutando METRICS\n");
			mostrarMetricas();
			break;
		default:
			printf("\nComando no reconocido\n\n");
	}
	return 0;
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
	paquete_recibido = recibir(socket_Memoria);

	if(paquete_recibido->codigo_operacion == cop_ok){
			tamanio_value = deserializar_int(paquete_recibido->data, 0);
	}
	liberar_paquete(paquete_recibido);
	return 0;
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

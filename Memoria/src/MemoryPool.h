/*
 * MemoryPool.h
 *
 *  Created on: 6 abril 2019
 *      Authors: 	Iván De Luca
 *      			Lucas Cerliani
 *      			Lucas Rosende
 *      			Christian Digiorno
 *      			Alejandro González
 */

#ifndef SRC_MEMORYPOOL_H_
#define SRC_MEMORYPOOL_H_

#include <Libraries.h>
#include <API.h>
#include <ClientServer.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/temporal.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <sys/timeb.h>
#include "MemoriaPrincipal.h"
#include "Config_&_Log.h"
#define IP "127.0.0.1"


pthread_mutex_t mutex_logger;

typedef struct{
	int numero_memoria;
	char* dir_IP;
	char* Puerto;
	un_socket conexion;
}t_gossip;


void iniciar_servidor_select();
static void administrar_conexion(t_paquete* paquete_recibido, un_socket nuevo_socket);
static int conectarse_con_FS();
static void* bufferear(char*);
static t_paquete* enviar_FS(char** argumentos, command_api op_code);


/*
enum flag_full{
	NOT_FULL,
	FULL
};

typedef enum flag_full estado_memoria;

estado_memoria ocuapdo;
*/

#endif /* SRC_MEMORYPOOL_H_ */

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

#include <commons/collections/list.h>
#include <Libraries.h>
#include <API.h>
#include <commons/log.h>
#include "headers/structs.h"
#include "Planificacion.h"

typedef enum API command_api;

t_log* logger;

enum Criterios_Memoria{
	Null,
	SC, //Criterio Strong Consistency
	SHC, //Criterio Strong-Hash Consistency
	EC //Criterio Eventual Consistency
};

typedef enum Criterios_Memoria criterio_memoria;

typedef struct {
	int id;
	char* ip;
	int puerto;
	int socket_memoria;
} t_memoria;

//t_memoria estructura_memoria;

t_list* memorias;
t_list* memoriasSC;
t_list* memoriasEC;
t_list* memoriasSHC;

typedef struct {
	int id;
	char* table_name;
	criterio_memoria criterio;
}t_metadata_tabla;

t_metadata_tabla metadata_tabla;

t_list* tabla_metadatas;

t_requestAMemoria requestAPlanificar;

int tamanio_value;

t_requestAMemoria crearEstructuraRequest(int, t_list*, command_api);

int conectar_con_Memoria(char*, char*);

void parsear_archivo_lql(char*);

void mostrarMetricas();

int socket_memoria_request();

void inicializar_metadata_tablas();
criterio_memoria buscar_tabla_en_tablametadata(char*);

void inicializar_memorias();
void asignar_memoria(t_memoria*);
int asignar_memoria_criterio(char**);
criterio_memoria convertir_string_criterio(char *);
int verificar_existe_memoria(int);
int buscar_socket_memoria_request(criterio_memoria);

//CREATE
void actualizar_tabla_metadatos(char*, char*);
int id_tabla = 0;

//JOURNAL
void journal_memorias();

pthread_t thread_parser;


#endif /* SRC_KERNEL_H_ */

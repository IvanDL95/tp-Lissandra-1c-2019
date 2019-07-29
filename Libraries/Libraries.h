#ifndef LIBRARIES_H_
#define LIBRARIES_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <commons/string.h>
#include <commons/error.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/txt.h>
#include <commons/log.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <bits/time.h>
#include <fcntl.h>
#include <linux/if_link.h>
#include <stdarg.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include "API.h"
#include "ClientServer.h"
#define MAX_LEN 128

//typedef char* byte;

/**	@NAME: log_and_free
 * 	@DESC: Loguea un mensaje y libera la memoria reservada por el string.
 *
 */

void log_and_free(t_log* logger, char* mensaje);

/**	@NAME: log_and_free
 * 	@DESC: Loguea un error y libera la memoria reservada por el string.
 *
 */

void log_error_and_free(t_log* logger, char* mensaje);

/**	@NAME: terminar_programa
 * 	@DESC: Libera/destruye estructuras y cierra el programa
 *
 */

void terminar_programa(t_log* logger, int*);

/**	@NAME: conectar_a
 * 	@DESC: Intenta conectarse.
 * 	@RETURN: Devuelve el socket o te avisa si hubo un error al conectarse.
 *
 */


/* ---------------------------------------------------------------------------
 * -------------------------- CONFIGURACIÓN ----------------------------------
 * ---------------------------------------------------------------------------
 */

/**	@NAME: get_configuracion
 *	@DESC: 	Obtiene la configuracion del archivo .cfg.
 *			Se define dentro de cada proceso, ya que varian las estructuras.
 *	@RETURN: Modifica la estructura global de configuracion.
 *
 */

void get_configuracion();

char get_campo_config_char(t_config* archivo_configuracion, char* nombre_campo);

int get_campo_config_int(t_config* archivo_configuracion, char* nombre_campo);

char** get_campo_config_array(t_config* archivo_configuracion,
		char* nombre_campo);

char* get_campo_config_string(t_config* archivo_configuracion,
		char* nombre_campo);

/* ---------------------------------------------------------------------------
 * -------------------------- SERIALIZACIÓN ----------------------------------
 * ---------------------------------------------------------------------------
 */

void serializar_int(void * buffer, int * desplazamiento, int valor);

int deserializar_int(void * buffer, int * desplazamiento);

void serializar_string(void * buffer, int * desplazamiento, char* valor);

char* deserializar_string(void * buffer, int * desplazamiento);

void serializar_lista_strings(void * buffer, int * desplazamiento, t_list * lista);

t_list * deserializar_lista_strings(void * buffer, int * desplazamiento);

int size_of_list_of_strings_to_serialize(t_list * list);

/* ---------------------------------------------------------------------------
 * ----------------------------- LISTAS --------------------------------------
 * ---------------------------------------------------------------------------
 */

/**	@NAME: recibir_listado_de_strings
 * 	@DESC: Recibe una lista de string desde un socket
 * 	@RETURN: La lista recibida
 */
t_list * recibir_listado_de_strings(un_socket socket);

/**	@NAME: enviar_listado_de_strings
 * 	@DESC: Envía un comand con una lista de string a un socket
 */
void enviar_listado_de_strings(un_socket socket, t_list * listado_strings, command_api comando);

void destruir_lista_strings(t_list * lista);

/**	@NAME: copy_list
 *	@DESC: Copia una lista.
 *	@RETURN: La lista copiada.
 */

t_list * copy_list(t_list * lista);

/**	@NAME: list_remove_all_by_condition
 *	@DESC: Remueve de la lista todos los elementos que cumplen la condicion.
 *	@RETURN: Lista de los elementos eliminados.
 */

t_list * list_remove_all_by_condition(t_list * lista, bool(*condicion)(void*));



/* ---------------------------------------------------------------------------
 * ----------------------------- STRINGS -------------------------------------
 * ---------------------------------------------------------------------------
 */

/**	@NAME: enviar_string
 *	@DESC: Envía un string a un socket con un codigo de operación.
 */

void enviar_string(un_socket socket_para_enviar, int codigo_de_operacion, char* mensaje);

size_t size_of_string(const char* string);

char* string_concat(int cant_strings, ...);

char** str_split(char* a_str, const char a_delim);

char *str_replace(char *orig, char *rep, char *with);

char* copy_string(char* value);

/**	@NAME: strings_equal
 *	@DESC: Compara dos strings
 *	@RETURN: Si son iguales o distintas.
 */

bool strings_equal(char* string1, char* string2);

int countOccurrences(char * str, char * toSearch);

char *randstring(size_t length);

int size_of_strings(int cant_strings, ...);

int array_of_strings_length(char** array);

void free_array(char** array, int array_size);


/* ---------------------------------------------------------------------------
 * ----------------------- ARCHIVOS Y CARPETAS -------------------------------
 * ---------------------------------------------------------------------------
 */

void print_image(FILE *fptr);

void imprimir(char*);

unsigned long int lineCountFile(const char *filename);

void crear_subcarpeta(char* nombre);

char* generarDirectorioTemporal(char* carpeta);

#endif /* LIBRARIES_H_ */

/*
 * Filesystem.c
 *
 *  Created on: 1 jun. 2019
 *      Author: utnso
 */

#include "Filesystem.h"


int montarFS(pathTablas,pathBloques){
	int e;
	struct stat info;

	log_info(logger, "Preparando punto de montaje.");
	e = stat(config_LS.PUNTO_MONTAJE, &info);

	if (errno == ENOENT) {
		log_warning(logger,
				"El punto de montaje no existe. Se creará el directorio.");
		//e = mkdir(config_LS.PUNTO_MONTAJE,  ACCESSPERMS | S_IRWXU);
		e = mkdir(config_LS.PUNTO_MONTAJE, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

		crearDirectorioTablas(pathTablas);
		crearDirectorioBloques(pathBloques);

	}

	return e;


}


int crearDirectorioTablas(char* pathTablas){

	int e;
	struct  stat info;
	log_info(logger, "Creando directorio de tablas...");

	strcpy(pathTablas,config_LS.PUNTO_MONTAJE);
	strcat(pathTablas,"Tablas/");

	e = stat(config_LS.PUNTO_MONTAJE, &info);

	mkdir(pathTablas, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	if(e!=0){
		log_error(logger,"Se produjo un error al crear el directorio. [%d - %s]", errno, strerror(errno));
		return 0;
	}else
		log_info(logger,"Se creo el directorio para almacenar las tablas");


	return 1;

}


int crearDirectorioTabla(char* nombreTabla,char* path){
	int e=0;
	strcpy(path, pathTablas);
	strcat(path, nombreTabla);
	e=mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	if(e!=0){
		log_error(logger,"Error al crear el directorio de la tabla actual");
		return 0;

	}else
		log_info(logger,"Directorio creado");

	return 1;
}


int crearDirectorioBloques(char* pathBloques){
	int e = 0;
	pathBloques=malloc(50);
	strcpy(pathBloques, config_LS.PUNTO_MONTAJE);
	strcat(pathBloques, "Bloques/");
	e = mkdir(pathBloques, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	if (e != 0) {
		log_error(logger, "Error al crear el directorio de Bloques");
		return 0;

	} else
		log_info(logger, "Directorioc de bloques creado");


	return 1;

}

void operacionSelect(char* nombreTabla, int key) {

//Abrir directorio de la tabla pasada por parametro, si existe
	//Leer la metadata de esa tabla asi, para obtener el valor de la cantidad de particiones

	//con el valor del total de particiones, calculo en que particion se encuentra
//leer la particion y guardarme el dato (en una lista de valores)
	//escanear archivos temporales y la mem table y guardar tambien en la lista de valores
	//devolver el que tenga el timestamp mas chcio

	//
	FILE* archivo;
	tInfoMetadata* infoMetadata = malloc(sizeof(tInfoMetadata));
	infoMetadata->consistency = malloc(5);
	char* pathMetadata = malloc(10);
	pathTablas = malloc(50);
	pathBloques = malloc(50);
	char* pathTablaActual = malloc(50);
	t_dictionary* metadata = dictionary_create(); // DICT INFO METADATA
	int particionObjetivo;
	struct stat info;
	char* pathParticion = malloc(50);
	tParticion* particion = malloc(sizeof(tParticion));
	t_list* bloques = list_create();
	t_list* clavesEncontradas = list_create();
	int i;


	char* pathTabla = malloc(100);

	strcpy(pathTablas,config_LS.PUNTO_MONTAJE);
	strcat(pathTablas,"Tablas/");

	strcpy(pathTabla,pathTablas);
	strcat(pathTabla,nombreTabla);



	strcpy(pathBloques,config_LS.PUNTO_MONTAJE);
	strcat(pathBloques,"Bloques/");

	strcpy(pathMetadata,pathTabla);
	strcat(pathMetadata,"/metadata.txt");
	leerMetadata(metadata, pathMetadata);

	infoMetadata->consistency = dictionary_get(metadata, "CONSISTENCIA");
	infoMetadata->particiones = dictionary_get(metadata, "PARTICIONES");
	infoMetadata->tiempoCompactacion = dictionary_get(metadata,
			"TIEMPO_COMPACTACION");

	particionObjetivo = calcularParticionObjetivo(key,
			infoMetadata->particiones);

	strcpy(pathParticion, pathTablaActual);
	strcat(pathParticion, (char*) key);

	t_config* cParticion = config_create(pathParticion);

	particion->tamanio = config_get_int_value(cParticion, "TAMANIO");
	particion->bloques = config_get_int_value(cParticion, "BLOQUES");

	crearListaDeBloques(particion, bloques);

	//obtenerBloquesDelArchivo(pathParticion, key);

	char* pathBloqueActual = malloc(50);
	strcpy(pathBloqueActual, pathBloques);

	for (i = 0; i < bloques->elements_count; i++) {
		int bloque = list_get(bloques, i);
		strcat(pathBloqueActual, (char*) bloque);
		buscarEnArchivoDeBloque(pathBloqueActual, key, bloque,
				clavesEncontradas);
	}

	//abro el archivo metadata*/

}

void leerMetadata(t_dictionary* metadata, char* pathMetadata) {
	struct stat infoArchivo;
	metadata = dictionary_create();
	FILE* archivoMetadata = fopen(pathMetadata, "rw");

	stat(pathMetadata, &infoArchivo);

	char* buffer = calloc(1, infoArchivo.st_size + 1);
	//char* buffer = calloc(1, 100); //MODIFICAR EL SEGUNDO PARAMETRO

	fread(buffer, infoArchivo.st_size, 1, archivoMetadata);

	char** lineas = string_split(buffer, "\n");

	void agregarInfoMetadata(char* linea) {
		char** propiedadValor = string_n_split(linea, 2, "=");
		dictionary_put(metadata, propiedadValor[0], propiedadValor[1]);
		free(propiedadValor[0]);
		free(propiedadValor);
	}

	string_iterate_lines(lineas, agregarInfoMetadata);
	string_iterate_lines(lineas, (void*) free);

	free(lineas);
	free(buffer);
	fclose(archivoMetadata);

}

int calcularParticionObjetivo(int key, int cantParticiones) {

	return (key % cantParticiones);

}

void buscarEnArchivoBloque(char* path, int key) {

	FILE* archivoPart = fopen(path, "r");
	struct stat infoArch;

	stat(path, &infoArch);

}

void crearListaDeBloques(tParticion* particion, t_list* bloquesParticion) {

	void crearListaMemoriasSeeds() {

		char** bloques = string_split(particion->bloques, ",");
		int i = 0;

		while (true) {

			if (bloques[i] != NULL) {
				printf("Valor: %s\n", bloques[i]);
				list_add(bloquesParticion, atoi(bloques[i]));
				i++;
			} else
				break;
		}
		free(bloques);
	}
}

void buscarEnArchivoDeBloque(char* pathBloqueActual, int key, int bloque,
		t_list* clavesEncontradas) {

	struct stat infoArchivo;
	char* dirArchivoBloque = malloc(50);
	char* charUno = malloc(4);
	strcpy(charUno, "1.bin");
	strcat(pathBloqueActual, charUno);
	char* bufferBloques = malloc(30);
	t_list* entradasBloque = list_create();
	tEntradaBloque* entrada = malloc(sizeof(tEntradaBloque));

	FILE* arch = fopen(pathBloqueActual, "r");

	stat(pathBloqueActual, &infoArchivo);

	if (!(infoArchivo.st_size) == 0) {
		fread(bufferBloques, infoArchivo.st_size, 1, arch);

		char** lineas = string_split(bufferBloques, "\n");

		void agregarInfoBloque(char* linea) {
			char** propiedadValor = string_n_split(linea, 3, "=");
			entrada->clave = propiedadValor[1];

			if (entrada->clave == key) {
				list_add(clavesEncontradas, entrada);

			}
			free(propiedadValor);
			free(propiedadValor[1]);

		}

		//string_iterate_lines(lineas, agregarInfoBloque);
		//string_iterate_lines(lineas, (void*) free);
		agregarInfoBloque("100=1=casa");

		free(entrada);
		free(lineas);
		free(bufferBloques);
		fclose(arch);
	}

}



/*
 * Filesystem.c
 *
 *  Created on: 1 jun. 2019
 *      Author: utnso
 */

#include "Filesystem.h"

int montarFS(char* pathTablas,char* pathBloques);
int crearDirectorioTablas(char* pathTablas);
int crearDirectorioTabla(char* nombreTabla);
int crearDirectorioBloques(char* pathBloques);
void leerMetadata(t_dictionary* metadata, char* pathMetadata);
int calcularParticionObjetivo(int key, int cantParticiones);
void crearListaDeBloques(tParticion* particion, t_list* bloquesParticion);
void buscarEnArchivoDeBloque(char* pathBloqueActual, int key, int bloque,
		t_list* clavesEncontradas);


int montarFS(char* pathTablas,char *pathBloques){
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


int crearDirectorioTabla(char* nombreTabla){
	int e=0;
	char* path=malloc(60);
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

int crearDirectorioMetadata(){
	int e=0;
	char* pathMetadata=malloc(60);
	strcpy(pathMetadata, config_LS.PUNTO_MONTAJE);
	strcat(pathMetadata, "Metadata");
	e=mkdir(pathMetadata, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	if(e!=0){
		log_error(logger,"Error al crear el directorio Metadata");
		return 0;

	}else
		log_info(logger,"Directorio creado");

	return 1;
}


void leerMetadata(t_dictionary* metadata, char* pathMetadata) {
	struct stat infoArchivo;
	metadata = dictionary_create();
	FILE* archivoMetadata;
	int e;


	archivoMetadata= fopen(pathMetadata, "r");

	if(archivoMetadata==NULL){
		printf("fopen fallo, errno = %d\n", errno);

	}else printf("Archivo abierto exitosamente");

	e=stat(pathMetadata, &infoArchivo);

	char* buffer = calloc(1, 100);
	//char* buffer = calloc(1, 100); //MODIFICAR EL SEGUNDO PARAMETRO

	fread(buffer, 55, 1, archivoMetadata);

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


		char** bloques = string_split(particion->bloques, ",");
		int i = 0;

		while (true) {

			if (bloques[i] != NULL) {
				printf("Valor: %s\n", bloques[i]);
				int bloque = atoi(bloques[i]);
				list_add(bloquesParticion, bloque);
				i++;
			} else
				break;
		}
		free(bloques);
	}


void buscarEnArchivoDeBloque(char* pathBloqueActual, int key, int bloque,
		t_list* registrosEncontrados) {

	struct stat infoArchivo;
	char* dirArchivoBloque = malloc(60);
	//char* charUno = malloc(4);
	//strcpy(charUno, "1.bin");
	strcpy(pathBloqueActual, pathBloques);
	strcat(pathBloqueActual, string_itoa(bloque));
	strcat(pathBloqueActual, ".bin");

	char* bufferBloques = malloc(30);
	tEntrada* entrada = malloc(sizeof(tEntrada));
	entrada->valor=malloc(20);

	FILE* arch = fopen(pathBloqueActual, "r");

	stat(pathBloqueActual, &infoArchivo);

	if (!(infoArchivo.st_size) == 0) {
		fread(bufferBloques, infoArchivo.st_size, 1, arch);

		char** lineas = string_split(bufferBloques, "\n");

		void agregarInfoBloque(char* linea) {
			char** propiedadValor = string_n_split(linea, 3, ";");
			entrada->timestamp = atoi(propiedadValor[0]);
			entrada->clave = atoi(propiedadValor[1]);
			strcpy(entrada->valor,propiedadValor[2]);


			if (entrada->clave == key) {
				list_add(registrosEncontrados, entrada);

			}
			//free(propiedadValor);
			//free(propiedadValor[1]);

		}

		string_iterate_lines(lineas, agregarInfoBloque);
		string_iterate_lines(lineas, (void*) free);
		agregarInfoBloque("100;1;casa");

		free(entrada);
		free(lineas);
		free(bufferBloques);
		fclose(arch);
	}

}

void buscarEnArchivoTemporal(char* pathArchivoTemporal, int key,t_list* registrosEncontrados) {

	struct stat infoArchivo;
	tEntrada* entrada = malloc(sizeof(tEntrada));
	entrada->valor=malloc(20);

	char* buffer=malloc(100);

	FILE* arch = fopen(pathArchivoTemporal, "r");

	stat(pathArchivoTemporal, &infoArchivo);

	if (!(infoArchivo.st_size) == 0) {
		fread(buffer, infoArchivo.st_size, 1, arch);

		char** lineas = string_split(buffer, "\n");

		void agregarInfoBloque(char* linea) {
			char** propiedadValor = string_n_split(linea, 3, ";");
			entrada->timestamp = atoi(propiedadValor[0]);
			entrada->clave = atoi(propiedadValor[1]);
			strcpy(entrada->valor,propiedadValor[2]);


			if (entrada->clave == key) {
				list_add(registrosEncontrados, entrada);

			}
			//free(propiedadValor);
			//free(propiedadValor[1]);

		}

		string_iterate_lines(lineas, agregarInfoBloque);
		string_iterate_lines(lineas, (void*) free);
		//agregarInfoBloque("100=1=casa");

		free(entrada);
		free(lineas);
		free(buffer);
		fclose(arch);
	}

}



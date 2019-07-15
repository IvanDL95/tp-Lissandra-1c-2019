/*
 * Lissandra.c
 *
 *  Created on: 6 abril 2019
 *      Authors: 	Iván De Luca
 *      			Lucas Cerliani
 *      			Lucas Rosende
 *      			Christian Digiorno
 *      			Alejandro González
 */

// This code doesn't have a license. Feel free to copy.

#include "../src/Lissandra.h"
#include <Libraries.h>

#include <stdlib.h>
#include <stdio.h>

void analizar_paquete(un_socket*);
char* obtenerValorMayorTimestamp(t_list* registrosEncontrados){
	int i;
	int timestampMayor=0;
	tEntrada* entradaMayor=malloc(sizeof(tEntrada));
	for (i=0;i<registrosEncontrados->elements_count;i++){
		tEntrada* entradaActual=((tEntrada*)(list_get(registrosEncontrados,i)));

		if (entradaActual->timestamp>timestampMayor){
			entradaMayor=entradaActual;
		}
	}

	return entradaMayor->valor;


}

asignarPathParticion(char* pathTablaActual, char* pathParticion,int key){

	strcpy(pathParticion, pathTablaActual);
	strcat(pathParticion, "/");
	strcat(pathParticion, string_itoa(key));
	strcat(pathParticion, ".bin");
}

void asignarPathsTabla(char* pathTablaActual,char* nombreTabla,int key,char* pathMetadata,char* pathParticion){

	strcpy(pathTablaActual, pathTablas);
	strcat(pathTablaActual, nombreTabla);

	strcpy(pathMetadata, pathTablaActual);
	strcat(pathMetadata, "/metadata.bin");


}

const char *extensionArchivo(const char *nombreArch) {
    const char *punto = strrchr(nombreArch, '.');
    if(!punto || punto == nombreArch) return "";
    return punto + 1;
}

char *removerExtension(char *nombreArch) {
	char *retstr;
	char *lastdot;
	if (nombreArch == NULL)
		return NULL;
	if ((retstr = malloc(strlen(nombreArch) + 1)) == NULL)
		return NULL;
	strcpy(retstr, nombreArch);
	lastdot = strrchr(retstr, '.');
	if (lastdot != NULL)
		*lastdot = '\0';
	return retstr;
}

void listarBloques() {
	DIR* dir;
	struct dirent* ent;

	if ((dir = opendir(pathBloques)) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			if (strcmp(extensionArchivo(ent->d_name), "bin") == 0) {
				list_add(bloques, ent->d_name);

			}
		}

		closedir(dir);
	} else {
		/* could not open directory */
		perror("");
	}
}

void leerTemporales(char* pathTablaActual,int key, t_list* entradasEncontradas,char* tipoTemporal){

	DIR* dir;
	struct dirent *ent;
	char* pathArchTemporal=malloc(60);
	strcpy(pathArchTemporal,pathTablaActual);
	strcat(pathArchTemporal,"/");


	if ((dir = opendir (pathTablaActual)) != NULL) {
	  /* print all the files and directories within directory */
	  while ((ent = readdir (dir)) != NULL) {
	    if (strcmp(extensionArchivo(ent->d_name),tipoTemporal)==0){
	    	strcat(pathArchTemporal,ent->d_name);
	    	buscarEnArchivoTemporal(pathArchTemporal,key,entradasEncontradas);
	    }
	  }
	  closedir (dir);
	} else {
	  /* could not open directory */
	  perror ("");
	  }

}

void buscarValorEnMemtable(int key,t_list* registrosEncontrados){
int i;
tEntradaMemTable* entradaActual=malloc(sizeof(tEntradaMemTable));
tEntrada* entrada=malloc(sizeof(tEntrada));

	for (i=0;i<memTable->elements_count;i++){
		entradaActual=list_get(memTable,i);

		if(entradaActual->clave==key){
			entrada->timestamp=entradaActual->timestamp;
			entrada->clave=entradaActual->clave;
			strcpy(entrada->valor,entradaActual->valor);
			list_add(registrosEncontrados,entrada);
		}
	}

}

void listarMemTable(){
	int i;
	int posX=40;
	printf("\033[%d;%dH%s\n", 30, 30, "MEM TABLE");
	puts("----------------------------------------------------------------------------");
	printf("\033[%d;%dH%s", posX, 20, "Tabla:");
	printf("\033[%d;%dH%s", posX, 30, "T.Stamp:");
	printf("\033[%d;%dH%s", posX, 40, "Clave:");
	printf("\033[%d;%dH%s\n", posX, 50, "Valor:");


	for(i=0;i<memTable->elements_count;i++){
		posX=posX+20;
		tEntradaMemTable* entrada= list_get(memTable,i);
		printf("\033[%d;%dH%s", posX, 20, entrada->nombreTabla);
		printf("\033[%d;%dH%d", posX, 30, entrada->timestamp);
		printf("\033[%d;%dH%d", posX, 40, entrada->clave);
		printf("\033[%d;%dH%s\n", posX,50, entrada->valor);


	}
}

int crearYGrabarArchivoMetadataDeTabla(char* path, tInfoMetadata* infoMetadata){
	struct stat infoArchvo;
	FILE* arch=fopen(path,"w");
	char* propiedades=malloc(80);
	sprintf(propiedades,"%s%s%s%s%d%s%s%d","CONSISTENCY=",infoMetadata->consistency,"\n","PARTICIONES=",infoMetadata->particiones,"\n","TIEMPO_COMPACTACION=",infoMetadata->tiempoCompactacion);

	if(arch==NULL){
		log_error(logger, "Error al abrir el archivo");
		return -1;

	}else{
		//fwrite(infoMetadata->consistency,sizeof(infoMetadata->consistency),1,arch);
		fwrite(propiedades,strlen(propiedades),1,arch);
		return 1;
	}
	fclose(arch);


}


char* operacionSelect(char* nombreTabla,int key){
	printf("%s\n",pathTablas);
	char* pathTablaActual = malloc(80);
	char* pathMetadata = malloc(80);
	char* pathParticion = malloc(80);
	char* pathBloqueActual = malloc(80);
	char* valorBuscado=malloc(10);
	int i;

	asignarPathsTabla(pathTablaActual,nombreTabla,key,pathMetadata,pathParticion);
	asignarPathParticion(pathTablaActual,pathParticion,key);

	tParticion* particion = malloc(sizeof(tParticion));
	tInfoMetadata* infoMetadata = malloc(sizeof(tInfoMetadata));
	t_config* cParticion = config_create(pathParticion);
	t_list* bloques = list_create();
	t_list* registrosEncontrados = list_create();
	infoMetadata->consistency = malloc(5);
	int particionObjetivo;



	int existe=existeTablaEnFS(pathTablaActual);
	if(existe==1){
		//t_config* metadata = config_create("~/home/utnso/LISSANDRA_FS/Tables/Table1/metadata.txt");
		t_config* metadata=config_create(pathMetadata);


		infoMetadata->consistency=config_get_string_value(metadata,"CONSISTENCIA");
		infoMetadata->particiones=config_get_int_value(metadata,"PARTICIONES");
		infoMetadata->tiempoCompactacion = config_get_int_value(metadata,"TIEMPO_COMPACTACION");

		particionObjetivo = calcularParticionObjetivo(key,infoMetadata->particiones);

		t_config* cParticion = config_create(pathParticion);

		particion->tamanio = config_get_int_value(cParticion, "SIZE");
		particion->bloques = config_get_string_value(cParticion, "BLOCKS");

		crearListaDeBloques(particion, bloques);


		for (i = 0; i < bloques->elements_count; i++) {
				int bloque = 0;
				bloque=list_get(bloques, i);
				//strcat(pathBloqueActual, string_itoa(bloque));
				buscarEnArchivoDeBloque(pathBloqueActual, key, bloque,registrosEncontrados);
			}

		leerTemporales(pathTablaActual,key,registrosEncontrados,"tmp");
		buscarValorEnMemtable(key,registrosEncontrados);
		valorBuscado=obtenerValorMayorTimestamp(registrosEncontrados);
		return valorBuscado;

	}

	free(pathTablaActual);
	free(pathMetadata);
	free(pathParticion);
	free(pathBloqueActual);
	free(valorBuscado);
}

int operacionInsert(char* nombreTabla, int key, char* value, int timestamp ){
	char* pathTablaActual = malloc(80);
	char* pathMetadata = malloc(80);
	char* pathParticion = malloc(80);
	char* pathBloqueActual = malloc(80);

	asignarPathsTabla(pathTablaActual,nombreTabla,key,pathMetadata,pathParticion);

	int existe = existeTablaEnFS(pathTablaActual);

	if(existe==1){

		if(memTable->elements_count==0){
			tEntradaMemTable* entrada=malloc(sizeof(tEntradaMemTable));
			entrada->clave=key;
			strcpy(entrada->nombreTabla,nombreTabla);
			entrada->valor=value;

			if(timestamp!=NULL){
				entrada->timestamp=timestamp;
			}else entrada->timestamp=(int)time(NULL);

			list_add(memTable,entrada);

			return 1;
		}
	}else return -1;

	free(pathTablaActual);
	free(pathMetadata);
	free(pathParticion);
	free(pathBloqueActual);
}

int operacionCreate(char* nombreTabla, char* tipoConsistencia, int numParticiones, int tiempoCompactacion){
	char* pathTablaActual = malloc(80);
	char* pathMetadata = malloc(80);
	char* pathParticion = malloc(80);
	char* pathBloqueActual = malloc(80);
	tInfoMetadata* infoMetadata=malloc(sizeof(tInfoMetadata)+10);
	infoMetadata->consistency=malloc(4);
	strcpy(infoMetadata->consistency,tipoConsistencia);
	infoMetadata->particiones=numParticiones;
	infoMetadata->tiempoCompactacion=tiempoCompactacion;


	asignarPathsTabla(pathTablaActual,nombreTabla,NULL,pathMetadata,pathParticion);

	int existe = existeTablaEnFS(pathTablaActual);

	if(existe==-1){

		crearDirectorioTabla(nombreTabla);
		crearYGrabarArchivoMetadataDeTabla(pathMetadata,infoMetadata);
		crearArchivosBinariosDeTabla(pathTablaActual,infoMetadata->particiones);

	}else log_error(logger,"La tabla ya existe en el FS");

}

///ACA FALTAN LAS OPERACIONES DESCRIBE Y DROP

//******************************************************************//

int procesarArchivosTMPC(DIR* dirTablaActual) {
	struct dirent* entradasTMPC;
	int e=0;
	struct stat infoArchivo;
	char* buffer=malloc(100);
	int leidos=0;

	while ((entradasTMPC = readdir(dirTablaActual)) != NULL) { //LEO CADA ENTRADA DE LA TABLA
		if (strcmp(extensionArchivo(entradasTMPC->d_name), "tmpc") == 0) { //CONSIDEROS SOLO LOS TMPV

			FILE* arch=fopen(entradasTMPC->d_name, "w+");
			stat(entradasTMPC->d_name, &infoArchivo);
			if(e==0){
				log_error(logger,"Error al abrir el Archvio TMPC");
			}else {
				leidos=fread(buffer, infoArchivo.st_size, 1, arch);
				printf("LEIDOS: %d",leidos);

			}



		}
	}

}


void recorrerTablasACompactar() {

	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	int hayTemporales;
	char* nombreNuevo = malloc(20);
	int ret=0;
	int i=0;
	char* bufferArchivoTMPC=malloc(30);
	char* pathParticionActual=malloc(80);

	if ((dp = opendir(pathTablas)) == NULL) {//LEO CARPETA POR CARPETA DE CADA TABLA
		fprintf(stderr, "cannot open directory: %s\n", pathTablas);
		return;
	}
	chdir(pathTablas); //ME POSICIONO EN EL DIRECTORIO DE TABLAS
	while ((entry = readdir(dp)) != NULL) {
		DIR* dirTablaActual;
		char* pathTablaActual = malloc(80);
		struct dirent* ent2;

		lstat(entry->d_name, &statbuf);
		if (S_ISDIR(statbuf.st_mode)) {
			/* Found a directory, but ignore . and .. */
			if (strcmp(".", entry->d_name) == 0 	//TOMO LAS ENTRADAS QUE NO SEAN .. y .
					|| strcmp("..", entry->d_name) == 0)
				continue;
			strcpy(pathTablaActual, pathTablas);
			strcat(pathTablaActual, entry->d_name);

			if ((dirTablaActual = opendir(pathTablaActual)) != NULL) { //ABRO CADA TABLA
				/* print all the files and directories within directory */
				chdir(pathTablaActual); //ME POSICIONO EN LA TABLA ACTUAL
				archivosACompactar=list_create();

				while ((ent2 = readdir(dirTablaActual)) != NULL) { 	//LEO CADA ENTRADA DE LA TABLA
					if (strcmp(extensionArchivo(ent2->d_name), "tmp") == 0) { //CONSIDEROS SOLO LOS TEMPORALES
						strcpy(nombreNuevo, removerExtension(ent2->d_name));
						strcat(nombreNuevo, ".tmpc");
						ret = rename(ent2->d_name, nombreNuevo);
						list_add(archivosACompactar,ent2->d_name);
						struct stat info;
						int particionObjetivo=0;
						t_dictionary* metadataTablaActual=dictionary_create();
						char* bloques=malloc(20);

						//CAMBIAR EL HARDCODEO DE PATHS
						metadataTablaActual=leerMetadataTabla("/home/utnso/LISSANDRA_FS/Tables/Table1/metadata.txt");
						stat("/home/utnso/LISSANDRA_FS/Tables/Table1/A.tmpc", &info);
						FILE* arch=fopen("/home/utnso/LISSANDRA_FS/Tables/Table1/A.tmpc","r");
						memset(bufferArchivoTMPC, 0, sizeof(bufferArchivoTMPC));
						memset(bloques, 0, sizeof(bloques));

						//procesarArchivosTMPC(dirTablaActual);
						//fopen(list_get(archivosACompactar,0),"r");
						fseek(arch,0,SEEK_SET);
						fread(bufferArchivoTMPC,info.st_size,1,arch);

						string_trim(&bufferArchivoTMPC);
						char** substrings=string_split(bufferArchivoTMPC,"\n");

						while (substrings[i]!=NULL){
							char** propiedades= string_n_split(substrings[i],3,";");
							particionObjetivo=calcularParticionObjetivo(atoi(propiedades[1]),atoi(dictionary_get(metadataTablaActual,"PARTICIONES")));
							sprintf(pathParticionActual,"%s%s%d%s",pathTablaActual,"/",particionObjetivo,".bin");
							bloques=leerBloquesDeParticion(pathParticionActual);
							recorrerBloquesYBuscarClave(bloques,propiedades[0],propiedades[1],propiedades[2]);
							i++;
						}

						//dictionary_put(archivosACompactarPorTabla,entry->d_name,archivosACompactar); //ME GUARDO TODOS LOS ARCHIVOS A COMPACTAR POR TABLA EN UN DICCIONARIO
					}//SE PUEDE AGREGAR A UNA LISTA DE TABLAS A COMPACTAR O ALGO ASI Y REALIZAR LA COMPACTACION DE ESAS TABLAS
					//fopen(list_get(dictionary_get(archivosACompactarPorTabla,"Table1"),0),"r");

				}

			closedir(dirTablaActual);
			} else {
				/* could not open directory */
				perror("");
			}
			//printf("%*s%s/\n", depth, "", entry->d_name);
			/* Recurse at a new indent level */
			//printdir(entry->d_name, depth + 4);
		}
		//printf("%*s%s\n", depth, "", entry->d_name);
	}
	chdir("..");
	closedir(dp);
}




int inicarProcesoCompactacion(){


	recorrerTablasACompactar();

	}

	//renombrarArchivosTemporales();

crearHiloCompactador(){
	pthread_create(&hilo_compactador, NULL, (void*) inicarProcesoCompactacion, logger);

}



int main(int argc, char** argv){

logger = log_create("lissandra.log", "Lissandra", 1, LOG_LEVEL_INFO);
	log_info(logger, "Iniciando File System\n");
	char* valorBuscado=malloc(10);
	memTable=list_create();
	get_configuracion(argv[1]);
	bloques=list_create();
	pathTablas = malloc(60);
	pathBloques = malloc(60);

	strcpy(pathTablas,config_LS.PUNTO_MONTAJE);
	strcat(pathTablas,"Tables/");
	strcpy(pathBloques,config_LS.PUNTO_MONTAJE);
	strcat(pathBloques,"Bloques/");
	listarBloques();
	archivosACompactar=dictionary_create();

	void listar(char* el){
		printf("%s\n", el);
	}
	list_iterate(bloques,(void*) listar);

	//operacionCreate("Table2","SC",3,5);
	recorrerTablasACompactar();
	/*pthread_t hilo_consola;

	pthread_create(&hilo_consola, NULL, (void*) iniciar_consola, logger);

	log_info(logger, "Levantando servidor\n");
	//TODO uso una IP definida o INADDR_ANY?

	pthread_attr_init(&hilo_attr_Server);
	pthread_attr_setdetachstate(&hilo_attr_Server, PTHREAD_CREATE_DETACHED);
	pthread_mutex_init(&mutex_mensaje, NULL);

	un_socket socket_servidor = socket_escucha(IP,config_LS.PUERTO_ESCUCHA);

    log_info(logger, "Estoy escuchando\n");

    while(1) {  // main accept() loop
        un_socket nuevo_cliente =  aceptar_conexion(socket_servidor);

        log_info(logger,"Me llegó una nueva conexión\n");
        pthread_create(&hilo_Server,&hilo_attr_Server,(void*)analizar_paquete,&nuevo_cliente);

    }
	terminar_programa(logger, &socket_servidor);*/
}

void get_configuracion(char* ruta){
	log_info(logger, "Levantando archivo de configuracion del proceso Lissandra\n");

	t_config* archivo_configuracion = config_create(ruta);

	config_LS.PUERTO_ESCUCHA = copy_string(get_campo_config_string(archivo_configuracion, "PUERTO_ESCUCHA"));
	config_LS.PUNTO_MONTAJE = copy_string(get_campo_config_string(archivo_configuracion, "PUNTO_MONTAJE"));
	config_LS.RETARDO = get_campo_config_int(archivo_configuracion, "RETARDO");
	config_LS.TAMANIO_VALUE = get_campo_config_int(archivo_configuracion, "TAMAÑO_VALUE");
	config_LS.TIEMPO_DUMP = get_campo_config_int(archivo_configuracion, "TIEMPO_DUMP");

	puts("\n");
	config_destroy(archivo_configuracion);
}


void analizar_paquete(un_socket* nuevo_socket){
	while(1){
	/*
	fd_set temp_set, read_set;
	FD_ZERO(&read_set);
	FD_SET(nuevo_socket, &read_set);
	memcpy(&temp_set, &read_set, sizeof(temp_set));

	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	int result = select(nuevo_socket + 1, &temp_set, NULL, NULL, &tv);


	if (result == 0) {
		printf("select() timed out!\n");
		pthread_detach(pthread_self());
	}
	else if (result < 0 && errno != EINTR) {
		printf("Error in select(): %s\n", strerror(errno));
		pthread_detach(pthread_self());
	}
	*/
    t_paquete* paquete_recibido = recibir(*nuevo_socket);
    switch(paquete_recibido->codigo_operacion){
    	case codigo_error:
    		log_info(logger,"Se desconecto una memoria \n");
    	    close(*nuevo_socket);
    	    pthread_detach(pthread_self());
    	    pthread_exit(NULL);
    		break;
    	case cop_handshake:
    		log_info(logger, "Realizando Handshake con Memoria x\n");
    		esperar_handshake(*nuevo_socket, paquete_recibido);
    		log_info(logger, "Handshake exitoso!\n");

    		enviar(*nuevo_socket, cop_ok,sizeof(int),&config_LS.TAMANIO_VALUE);
    		paquete_recibido = recibir(*nuevo_socket);
				if(paquete_recibido->codigo_operacion == cop_ok)
					log_info(logger,"La memoria recibió el tamaño del value\n");
				else{
					if(paquete_recibido->codigo_operacion == codigo_error)
						log_info(logger,"La memoria no recibió el paquete y se cayó\n");
					else
						log_info(logger,"Recibí un paquete fantasma\n");
				}
			break;
    	default:
    	{
    		command_api comando = paquete_recibido->codigo_operacion;
    		/* envio argumentos como lista de strings */
    		int desplazamiento = 0;
    		t_list* lista_argumentos = deserializar_lista_strings(paquete_recibido->data, &desplazamiento);
    		int tamanio_lista = list_size(lista_argumentos);
    		char* argumentos[tamanio_lista];
    		for(int i=0;i<tamanio_lista;i++){
    			argumentos[i] = list_get(lista_argumentos,i);
    		}
    		list_destroy(lista_argumentos);
    		ejecutar_API(comando, argumentos);
    		/* segfault glorioso
    		char* mensaje_retorno = ejecutar_API(comando, argumentos);
    		if(mensaje_retorno == NULL)
    			enviar(*nuevo_socket,codigo_error,0,NULL);
    		else{
    			unsigned int tamanio_buffer = size_of_string(mensaje_retorno)+sizeof(int);
    			void* buffer = malloc(tamanio_buffer);
    			desplazamiento = 0;
    			serializar_string(buffer, &desplazamiento, mensaje_retorno);
    		}
    	*/
    		enviar(*nuevo_socket,codigo_error,0,NULL);
    	}
    	break;
    	}
    	liberar_paquete(paquete_recibido);
	}
}




char* ejecutar_API(command_api operacion, char** argumentos){
	switch(operacion){

		case SELECT:
			printf("hacer SELECT\n");
			break;
		case INSERT:
			printf("hacer INSERT");
			break;
		case CREATE:
			printf("hacer CREATE");
			break;
		case DESCRIBE:
			printf("hacer DESCRIBE");
			break;
		case DROP:
			printf("hacer DROP");
			break;
		default:
			log_info(logger, "Paquete no reconocido\n");
	}
	return NULL;
}

int existeTablaEnFS(char* pathTabla){
	int e=0;
	struct stat infoArch;

	e=stat(pathTabla,&infoArch);

	if (e == 0) {

		if (infoArch.st_mode & S_IFDIR)
			//log_info(logger,"Tabla encontrada en FS.");
		return 1;
	}else {
		log_info(logger,"La tabla no existe en el FS");
		return -1;
	}

}




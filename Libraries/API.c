#include "API.h"


void iniciar_consola(t_log* logger) {

	log_info(logger, "Iniciando Consola\n"); //Me tiene que importar si se desincroniza el log? No creo que importe
	char* comando[5];
	char* string_value;
	char* aux;
	char* command_quit = "Quit";
	while (1) {
		char* comando_consola = readline("Kernel_bash:>");
		if (strlen(comando_consola)==0 || strcmp(comando_consola,command_quit) == 0) {
			free(comando_consola);
			break;
		} else {
			log_info(logger, comando_consola);
			aux = calloc(strlen(comando_consola)+1, sizeof(char));
			strcpy(aux, comando_consola);
			int j=0;
			comando[j] = strtok(comando_consola, " ");
			while(comando[j] != NULL && j < 5) {
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


			char* mensaje_retorno = ejecutar_API(operacion, argumentos);

			if(mensaje_retorno != NULL)
				printf("%s\n", mensaje_retorno);
			else
				printf("Mensaje no reconocido\n");
			comando_consola = NULL;
			free(comando_consola);
		}
	}
}

command_api convertir_commando(char* command) {
	    static struct comm_api {
	        const char *key;
	        command_api token;
	    } token_table[] = {
	        { "SELECT", SELECT },
	        { "INSERT", INSERT },
			{ "CREATE", CREATE },
			{ "DESCRIBE", DESCRIBE },
			{ "DROP", DROP },
			{ "JOURNAL", JOURNAL },
			{ "ADD", ADD },
			{ "RUN", RUN },
			{ "METRICS", METRICS },
	        { NULL, 99 }
	    };
	    struct comm_api *p = token_table;
	    for(; p->key != NULL && strcmp(p->key, command) != 0; ++p);
	    return p->token;
}

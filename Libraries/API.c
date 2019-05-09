#include "API.h"


void* iniciar_consola(t_log* logger) {

	log_info(logger, "Iniciando Consola\n"); //Me tiene que importar si se desincroniza el log? No creo que importe

	char* comando_consola;
	char* comando[5];
	char* command_quit = "Quit";
	int j;

	while (1) {
		comando_consola = NULL;
		comando_consola = readline("Kernel_bash:>");
	        if (strlen(comando_consola)==0 || strcmp(comando_consola,command_quit) == 0) {
	        	free(comando_consola);
	        	break;
	        } else {
	        	log_info(logger, comando_consola);
	        	j=0;
				comando[j] = strtok(comando_consola, " ");
				while(comando[j] != NULL && j < 5) {
					//printf("\n%s", comando[j]);
					j++;
					comando[j] = strtok(NULL, " ");
				}
	        	char* argumentos[4];
	        	for(int i=0;i<4 && comando[i+1] != NULL;i++)
	        		argumentos[i] = comando[i+1];
	        	command_api operacion = convertir_commando(comando[0]);
	        	ejecutar_API(operacion, argumentos);
	        	free(comando_consola);
	        }

	    }
	return NULL;
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

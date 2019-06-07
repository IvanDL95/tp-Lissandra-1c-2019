## Proceso Kernel

El proceso Kernel será el punto de entrada a nuestro sistema. 
Él tendrá la responsabilidad de planificar las distintas request que se quieran efectuar, así como también la administración del pool de memorias y distribución de los pedidos entre las mismas.
Las request pueden llegar a este componente de dos formas posibles:
 - Archivos LQL (Lissandra Query Language)
 - Request unitarias realizadas mediante la API de este componente, las cuales serán consideradas como archivos LQL de una única línea

----------------------------------------------------------------------------------------------------------------------

La consola verifica que el primer argumento de la query se corresponda a algunos de los comandos de la API
SELECT, INSERT, CREATE, DESCRIBE, DROP, JOURNAL, ADD, RUN, METRICS.
Agregué el comando Quit para salir de la consola. TAmbién lo hace cuando se presiona sólo Enter (No hay query).

## Test de archivo lql

ejecutar en la consola
run ../prueba.lql

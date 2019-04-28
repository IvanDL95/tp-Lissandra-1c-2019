# tp-2019-1c-La-kernel-de-los-s-bados

Enunciado del TP: https://docs.google.com/document/d/1QlzXwpSvI5ua2lbO8pF6ZgjlgMndFlwzlAci7qhZmqE/edit

Para poder commitear:

git config --global user.email "(email@mail.com)"
git config --global user.name "(usuario Github)"

Para importar el proyecto en eclipse:

	1. 	File -> Import

	2. 	Existing Projects into Workspace

	3. 	Importar cada proceso por separado, son proyectos distintos en la práctica
		Que NO se hace:	_ Añadir nested projects
						_ Copiar solo la carpeta padre (tp-2c-...)


Para poder usar las commons (y con readline), además de instalarlas como en el video tutorial de la cátedra, hay que linkearla.
Para eso hay que decirle al compilador que linkee usando -lcommons. Esto no hay que hacerlo manualmente, sino que se hace
desde el makefile, el cual ya se encuentra creado.

En caso de que no esté tomando las commons hacer lo siguiente para cada carpeta (proyecto, proceso).

	1. Abrir propiedades (click derecho en la carpeta raíz del proyecto, abajo de todo o con alt+enter)

	2. Abrir el combo ("arbol") de "C/C++ Build" -> Pestaña Tool Settings

	3. GCC C Linker -> Libraries.

	4. A la derecha, en la ventan superior dice "Libraries (-l)", esto es el linker, para que cree el makefile linkeando las
		carpetas que le indiquemos. Click en el botón de "Add..." (forma de archivo con una flecha verde)

	5. Agregar "commons" (sin comillas). También agregar readline de paso ya que lo vamos a usar para la consola


# Linkear Libraries al proyecto

Como esta libreria no se va a instalar, se debe hacer lo siguiente:

1_ Referenciarla en el proyecto

Propiedades -> C/C++ General -> Paths and symbols -> Relacionar la libreria

https://youtu.be/Aw9kXFqWu_I?t=300

2_ Setear el entorno de cada proyecto para que la tome

Agregar LD_LIBRARY_PATH

https://youtu.be/Aw9kXFqWu_I?t=475

https://youtu.be/Aw9kXFqWu_I?t=512

## Run Configurations (Eclipse)

Name : Memoria

Main :

Project : Memoria
C/C++ Applications : Debug/Memoria
Build (if required) before launching :
Build COnfiguration : Debug

Environment:
LD_LIBRARY_PATH (el path absoluto en donde tengan instaladas las librerias)
/home/utnso/Documentos/SisOp/tp-2019-1c-La-kernel-de-los-s-bados/Libraries/Debug


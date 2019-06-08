
sudo git clone https://github.com/sisoputnfrba/so-commons-library.git ./so-commons-library

cd so-commons-library

sudo make

sudo makeinstall

git clone https://github.com/sisoputnfrba/tp-2019-1c-La-kernel-de-los-s-bados.git


cd /home/utnso/tp-2019-1c-La-kernel-de-los-s-bados/Libraries/Debug

sudo makefile

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2019-1c-La-kernel-de-los-s-bados/Libraries/Debug

cd /home/utnso/tp-2019-1c-La-kernel-de-los-s-bados/Lissandra FileSystem/src

gcc -I/home/utnso/tp-2019-1c-La-kernel-de-los-s-bados/Libraries/ -L/home/utnso/tp-2019-1c-La-kernel-de-los-s-bados/Libraries/Debug/ Filesystem.c Lissandra.c /home/utnso/tp-2019-1c-La-kernel-de-los-s-bados/Libraries/ClientServer.c -Wl,-rpath=/home/utnso/tp-2019-1c-La-kernel-de-los-s-bados/Libraries/Debug/ -o Filesystem -lcommons -lLibraries -lreadline -lpthread

cd /home/utnso/tp-2019-1c-La-kernel-de-los-s-bados/Memoria/src

gcc -I/home/utnso/tp-2019-1c-La-kernel-de-los-s-bados/Libraries/ -L/home/utnso/tp-2019-1c-La-kernel-de-los-s-bados/Libraries/Debug/ MemoryPool.c /home/utnso/tp-2019-1c-La-kernel-de-los-s-bados/Libraries/ClientServer.c -Wl,-rpath=/home/utnso/tp-2019-1c-La-kernel-de-los-s-bados/Libraries/Debug/ -o Memoria -lcommons -lLibraries -lreadline -lpthread


cd /home/utnso/tp-2019-1c-La-kernel-de-los-s-bados/Kernel/src


gcc -I/home/utnso/tp-2019-1c-La-kernel-de-los-s-bados/Libraries/ -L/home/utnso/tp-2019-1c-La-kernel-de-los-s-bados/Libraries/Debug/ Kernel.c /home/utnso/tp-2019-1c-La-kernel-de-los-s-bados/Libraries/ClientServer.c -Wl,-rpath=/home/utnso/tp-2019-1c-La-kernel-de-los-s-bados/Libraries/Debug/ -o Kernel -lcommons -lLibraries -lreadline -lpthread



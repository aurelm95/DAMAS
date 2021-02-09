all:
	clear
	gcc milisec.c -O3 -Wall -Wno-missing-braces -Wno-unused-variable -Wno-unused-function -pedantic

runn:
	./a.out > respuesta.txt

run:
	./a.out

so:
	gcc -O3 -shared -o damas.so -fPIC milisec.c


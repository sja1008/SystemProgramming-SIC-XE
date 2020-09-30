20160563.out : 20160563.o utility.o shell.o memory.o opcode.o assemble.o loader.o
	gcc -Wall -o 20160563.out 20160563.o utility.o shell.o memory.o opcode.o assemble.o loader.o

20160563.o : 20160563.c
	gcc -Wall -c 20160563.c

utility.o : utility.c
	gcc -Wall -c utility.c

shell.o : shell.c
	gcc -Wall -c shell.c

memory.o : memory.c
	gcc -Wall -c memory.c

opcode.o : opcode.c
	gcc -Wall -c opcode.c

assemble.o : assemble.c
	gcc -Wall -c assemble.c

loader.o : loader.c
	gcc -Wall -c loader.c

clean :
	rm 20160563.out *.o
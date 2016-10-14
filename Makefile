obj-m += lab3.o
all:
	gcc -c project1.2.c
	gcc -o project1.2 project1.2.o
clean:
	rm project1.2 *.o

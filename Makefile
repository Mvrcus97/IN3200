#INSPIRED BY THE DOCUMENT GIVEN AT IN3200's HOMEPAGE.

PROGRAMS = run

.PHONY : clean

run : main.o functions.o exam_header.h
	gcc main.o functions.o -o run -Wall -fopenmp

functions.o : PE_functions_15219.c
	gcc -c PE_functions_15219.c -o functions.o -Wall -fopenmp

main.o : PE_main_15219.c exam_header.h
	gcc -c PE_main_15219.c -o main.o -Wall

clean :
	rm -f *.o $(PROGRAMS)

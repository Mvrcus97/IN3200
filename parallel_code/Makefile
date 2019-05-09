CC = mpicc
CFLAGS = -O2
LDFLAGS = -L../simple-jpeg/ -lsimplejpeg

PROJ = parallel_main
OBJS = parallel_main.o functions.o
DEP = parallel_header.h

all: simple-jpeg $(PROJ)

parallel_main: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

parallel_main.o : $(DEP)
functions.o : $(DEP)

simple-jpeg:
	$(MAKE) -C ../simple-jpeg

obj-clean:
	$(RM) *.o

exec-clean:
	$(RM) $(PROJ)

autosave-clean:
	$(RM) *~

clean:
	$(MAKE) obj-clean
	$(MAKE) exec-clean
	$(MAKE) autosave-clean
	$(MAKE) -C ../simple-jpeg clean

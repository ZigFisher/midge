
CFLAGS+=-g

all: dev_interface.o mxconfig.o mxobjects.o
	$(CC) $(CFLAGS) -o mxconfig mxconfig.o dev_interface.o mxobjects.o

clean:
	rm --force *.o mxconfig
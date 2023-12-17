CC=gcc
CFLAGS=-fPIC -Wall -Wextra -g -lpthread
LDFLAGS=-ldl
OUTPUT_DIR=$(abspath ../checker/output/)
CFLAGS+=-DOUTPUT_TEMPLATE="\"$(OUTPUT_DIR)/out-XXXXXX\""
LDLIBS=$(abspath ./libipc.so ./liblog.so)
CMODULES=server.o data_structures.o globals.o client_handler.o

build: outdir server

outdir:
	@mkdir -p $(OUTPUT_DIR)

server: server.o data_structures.o globals.o client_handler.o libipc.so liblog.so
	$(CC) -o $@ $(CMODULES) $(LDLIBS) $(LDFLAGS)

server.o: server.c ipc.h
	$(CC) $(CFLAGS) -o $@ -c $<

data_structures.o: data_structures.c data_structures.h
	$(CC) $(CFLAGS) -o $@ -c $<

globals.o: globals.c globals.h
	$(CC) $(CFLAGS) -o $@ -c $<

client_handler.o: client_handler.c client_handler.h
	$(CC) $(CFLAGS) -o $@ -c $<

libipc.so: ipc.o log.o
	$(CC) -shared -o $@ $^

liblog.so: log.o
	$(CC) -shared -o $@ $^

ipc.o: ipc.c ipc.h
	$(CC) $(CFLAGS) -o $@ -c $<

log.o: log.c log.h
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf server *.o libipc.so liblog.so

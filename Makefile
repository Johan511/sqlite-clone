CC=gcc
OPT=-O3
DEBUG=1

BINARY=sqlite
OBJECTS=objects/repl.o
INLCUDES=includes/repl.h

all : $(BINARY)

# $@ => left of colon
# $^ => everything on right
# $< => first object
$(BINARY) : $(OBJECTS) $(INLCUDES)
	$(CC) -o $@ $^

objects/%.o : src/%.c includes/%.h
	$(CC) -c -o $@ $<

clean:
	rm -rf objects/* && rm sqlite
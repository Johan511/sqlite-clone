CC=gcc
OPT=-O3
DEBUG=1

INCDIR=includes
OBJDIR=objects
BINARY=sqlite
SRC=src
OBJECTS=$(OBJDIR)/repl.o
INLCUDES=$(INCDIR)/repl.h

all : $(BINARY)

# $@ => left of colon
# $^ => everything on right
# $< => first object
$(BINARY) : $(OBJECTS) $(INLCUDES)
	$(CC) -o $@ $^

$(OBJDIR)/%.o : $(SRC)/%.c $(INCDIR)/%.h
	$(CC) -c -o $@ $<

clean:
	rm -rf $(OBJDIR)/* && rm sqlite
commit:
	git add .
	git commit -m "$(CM)"
	git push remote origin
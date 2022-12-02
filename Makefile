CC=gcc
OPT=-O3
DEBUG=1

INCDIR=includes
OBJDIR=objects
BINARY=sqlite
SRC=src
TESTDIR=test
OBJECTS=$(OBJDIR)/repl.o
INLCUDES=$(INCDIR)/repl.h
TEST_FILES=$(TESTDIR)/genTest.py
DB_FILE=.db
TEST_INPUT_FILE=$(TESTDIR)/input.txt
PYTHON_INTR=python3

all : $(BINARY)

# $@ => left of colon
# $^ => everything on right
# $< => first object
$(BINARY) : $(OBJECTS) $(INLCUDES)
	$(CC) -o $@ $^

$(OBJDIR)/%.o : $(SRC)/%.c $(INCDIR)/%.h
	$(CC) -c -g -o $@ $<

$(TEST_INPUT_FILE) : $(TEST_FILES)
	$(PYTHON_INTR) $(TEST_FILES)

test: $(BINARY) $(TEST_FILES) $(TEST_INPUT_FILE)
	make all && ./sqlite $(DB_FILE) < $(TEST_INPUT_FILE)

clean:
	rm -rf $(OBJDIR)/* && rm sqlite
commit:
	git add .
	git commit -m "$(CM)"
	git push
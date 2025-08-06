CC = gcc
CFLAGS = -Wall -Wextra -g

all: molecule_supplier molecule_requester atom_supplier

molecule_supplier: molecule_supplier.c
	$(CC) $(CFLAGS) -o $@ $^

molecule_requester: molecule_requester.c
	$(CC) $(CFLAGS) -o $@ $^

atom_supplier: atom_supplier.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f molecule_supplier molecule_requester atom_supplier *.o *.gcda *.gcno

coverage: CFLAGS += -fprofile-arcs -ftest-coverage
coverage: clean all
	@echo "Run your tests, then run 'gcov *.c' in this directory to see coverage"
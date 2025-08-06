CC = gcc
CFLAGS = -Wall -Wextra -g

all: atom_warehouse atom_supplier

atom_warehouse: atom_warehouse.c
	$(CC) $(CFLAGS) -o $@ $^

atom_supplier: atom_supplier.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f atom_warehouse atom_supplier *.o *.gcda *.gcno

coverage: CFLAGS += -fprofile-arcs -ftest-coverage
coverage: clean all
	@echo "Run your tests, then run 'gcov *.c' in this directory to see coverage"
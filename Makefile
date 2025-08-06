SUBDIRS := Q1 Q2 Q3 Q4 Q5 Q6

.PHONY: all clean coverage

all:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

coverage:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir coverage; \
	done
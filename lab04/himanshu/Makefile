CC=gcc
DEPS = utils.h
OBJ = jash.o

%.o : %.c $(DEPS)
	$(CC) -c -o $@ $< 
jash: $(OBJ)
	$(CC) -o $@ $(OBJ)

.PHONY: clean
clean:
	rm -rf jash
	rm -rf *.o
	rm -rf *.out


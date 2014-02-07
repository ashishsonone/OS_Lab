CC=g++
DEPS = utils.h event_manager.h scheduler.h
OBJ = main.o event_manager.o utils.o scheduler.o

%.o : %.cpp $(DEPS)
	$(CC) -c -o $@ $< 

a.out: $(OBJ)
	$(CC) -o $@ $(OBJ)

.PHONY: clean
clean:
	rm -rf *.o
	rm -rf *.out


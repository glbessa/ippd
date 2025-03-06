FILENAME = main.c
OUTPUT = main
NUM_PROCS = 4

build:
	mpicc $(FILENAME) -o $(OUTPUT) -lm

run:
	mpirun -np $(NUM_PROCS) ./$(OUTPUT)

clean:
	rm $(OUTPUT)
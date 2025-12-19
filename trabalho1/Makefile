CC = gcc
CFLAGS_SEQ = -O2 -Wall -Wextra
CFLAGS_OMP = -O2 -Wall -Wextra -fopenmp

SRC_SEQ_DIR = src/seq
SRC_OMP_DIR = src/omp

BIN_DIR = bin

.PHONY: all seq omp run plot clean task-d

all: seq omp

# Alvo para compilar versões sequenciais (quando você as implementar em src/seq)
seq: $(BIN_DIR)/task_a_seq

$(BIN_DIR)/task_a_seq: $(SRC_SEQ_DIR)/task_a.c | $(BIN_DIR)
	$(CC) $(CFLAGS_SEQ) $< -o $@

# Alvo para compilar versões paralelas com OpenMP
omp: $(BIN_DIR)/task_a_omp

$(BIN_DIR)/task_a_omp: $(SRC_OMP_DIR)/task_a/v1.c | $(BIN_DIR)
	$(CC) $(CFLAGS_OMP) $< -o $@

$(BIN_DIR):
	mkdir -p $@

# Executa toda a matriz de experimentos (gera results/results.csv)
run: omp
	./run.sh

# Gera gráficos a partir do CSV (usa plot.py)
plot:
	python plot.py

task-d: $(BIN_DIR)/task_d

$(BIN_DIR)/task_d: task-d.c | $(BIN_DIR)
	$(CC) $(CFLAGS_OMP) $< -o $@

clean:
	rm -rf results plots
	rm -rf $(BIN_DIR)

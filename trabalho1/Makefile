CC = gcc
CFLAGS_SEQ = -O2 -Wall -Wextra
CFLAGS_OMP = -O2 -Wall -Wextra -fopenmp

SRC_SEQ_DIR = src/seq
SRC_OMP_DIR = src/omp

BIN_DIR = bin

.PHONY: all seq omp run plot clean

all: seq omp

# Alvo para compilar versões sequenciais (quando você as implementar em src/seq)
seq: $(BIN_DIR)/task_a_seq

$(BIN_DIR)/task_a_seq: $(SRC_SEQ_DIR)/task_a.c | $(BIN_DIR)
	$(CC) $(CFLAGS_SEQ) $< -o $@

# Alvo para compilar versões paralelas com OpenMP (Tarefa A: static, dynamic, guided)
omp: $(BIN_DIR)/task_a_omp_static $(BIN_DIR)/task_a_omp_dynamic $(BIN_DIR)/task_a_omp_guided

$(BIN_DIR)/task_a_omp_static: $(SRC_OMP_DIR)/task_a/v1.c | $(BIN_DIR)
	$(CC) $(CFLAGS_OMP) $< -o $@

$(BIN_DIR)/task_a_omp_dynamic: $(SRC_OMP_DIR)/task_a/v2.c | $(BIN_DIR)
	$(CC) $(CFLAGS_OMP) $< -o $@

$(BIN_DIR)/task_a_omp_guided: $(SRC_OMP_DIR)/task_a/v3.c | $(BIN_DIR)
	$(CC) $(CFLAGS_OMP) $< -o $@

$(BIN_DIR):
	mkdir -p $@

# Executa toda a matriz de experimentos (gera results/results.csv)
run: omp
	./run.sh

# Gera gráficos a partir do CSV (usa plot.py)
plot:
	python plot.py

clean:
	rm -rf results plots
	rm -rf $(BIN_DIR)

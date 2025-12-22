## REPRODUCIBILIDADE – Tarefa A (Laço irregular)

Este documento descreve o ambiente usado e os passos necessários para
reproduzir **exatamente** os resultados da Tarefa A.

> Ajuste os campos marcados com `<>` de acordo com a sua máquina, se
> necessário.

---

### 1. Ambiente de hardware

- **CPU**: `<modelo exato da CPU`>  
	Exemplo: `AMD Ryzen 5 5600H (6c/12t, 3.3–4.2 GHz)`
- **Núcleos lógicos**: `<número de CPUs lógicas reportado por nproc>`  
	Comando para conferir:

	```bash
	nproc
	lscpu
	```

### 2. Sistema operacional

- **SO**: Linux `<distribuição e versão>`  
	Exemplo: `Ubuntu 22.04.4 LTS 64‑bit`

Para obter essas informações:

```bash
uname -a
cat /etc/os-release
```

### 3. Compilador e versão

- **Compilador C**: `gcc`  
- **Versão**: saída de `gcc --version` (exemplo):

	```bash
	gcc --version
	```

	Exemplo de resultado:

	```
	gcc (Ubuntu 13.2.0-23ubuntu4) 13.2.0
	```

### 4. Flags de compilação

As flags são definidas em `Makefile` e foram usadas sem alterações:

- Versão sequencial:

	```
	CFLAGS_SEQ = -O2 -Wall -Wextra
	```

- Versão OpenMP (Tarefa A):

	```
	CFLAGS_OMP = -O2 -Wall -Wextra -fopenmp
	```

Compilação específica da Tarefa A (binários paralelos):

```bash
cd trabalho1
make clean
make omp
```

Isso gera os executáveis:

- `bin/task_a_omp_static`   – variante `schedule(static)`  
- `bin/task_a_omp_dynamic`  – variante `schedule(dynamic, chunk)`  
- `bin/task_a_omp_guided`   – variante `schedule(guided, chunk)`

### 5. Afinidade de CPU

Nenhuma afinidade explícita foi configurada no código ou nos scripts
(`run.sh`). O escalonador padrão do sistema operacional foi utilizado.

Caso deseje **fixar afinidade** para maior controle, pode-se executar
o script com `taskset`. Exemplo para restringir aos 8 primeiros CPUs
lógicos:

```bash
taskset -c 0-7 ./run.sh A
```

> Se utilizar `taskset`, documente aqui o conjunto de CPUs usado.

### 6. Semente do gerador de números aleatórios

A Tarefa A **não utiliza gerador de números pseudo-aleatórios**: o
kernel é determinístico (`fib(i % K)`), portanto **não há semente a
configurar**.

Se futuramente for introduzida aleatoriedade, recomenda‑se:

- Fixar uma semente constante no código (por exemplo `srand(12345);`), e
- Registrar aqui o valor adotado.

### 7. Matriz de experimentos

A matriz de experimentos utilizada segue `INSTRUCTIONS.md`:

- $N \in \{100000, 500000, 1000000\}$  
- $K \in \{20, 24, 28\}$  
- Threads: `{1, 2, 4, 8, 16}` (ajustar se a máquina tiver menos CPUs)  
- Repetições por ponto: `REPS = 5`

As variantes avaliadas na Tarefa A são:

- `static`  
- `dynamic_1`, `dynamic_4`, `dynamic_16`, `dynamic_64`  
- `guided_1`, `guided_4`, `guided_16`, `guided_64`

### 8. Execução dos experimentos (Tarefa A)

No diretório `trabalho1`:

```bash
cd trabalho1
make omp          # compila binários paralelos da Tarefa A
./run.sh A        # executa apenas a Tarefa A
```

O script `run.sh`:

- Lê `OMP_NUM_THREADS` internamente para cada ponto da matriz.  
- Executa cada ponto 5 vezes.  
- Salva a média e o desvio‑padrão em `results/results.csv`.  
- É **idempotente** para a Tarefa A: se interrompido, ao rodar novamente
	pula os pontos já presentes no CSV.

### 9. Geração dos gráficos

Com o CSV já gerado:

```bash
cd trabalho1
python3 plot.py
```

Os gráficos da Tarefa A são salvos em `plots/` com nomes como:

- `taskA_N<N>_K<K>_total.png`   – tempo total vs threads  
- `taskA_N<N>_K<K>_kernel.png`  – tempo de kernel vs threads  
- `taskA_N<N>_K<K>_speedup.png` – speedup vs threads

Inclua esses arquivos em `RESULTADOS.md` com a análise correspondente.

### 10. Resumo para reexecução rápida

1. Garantir ambiente com `gcc` + OpenMP em Linux.  
2. Ajustar (se necessário) valores de `THREADS` e demais parâmetros em
	 `run.sh`.  
3. Compilar:

	 ```bash
	 cd trabalho1
	 make clean
	 make omp
	 ```

4. Executar a matriz de Tarefa A:

	 ```bash
	 ./run.sh A
	 ```

5. Gerar gráficos:

	 ```bash
	 python3 plot.py
	 ```

6. Usar `results/results.csv` e as figuras em `plots/` como base da
	 análise apresentada em `RESULTADOS.md`.

## Integrantes do grupo

- Gabriel Leite Bessa — responsável pela Tarefa A (Laço irregular e políticas de schedule).
- Yago Martins Pinto — responsável pela Tarefa D (Organização de região paralela).

## Como compilar e executar

Todos os comandos abaixo devem ser executados a partir da pasta `trabalho1/`.

### Compilação

- Compilar versões sequenciais e paralelas:

```bash
make all   # ou separadamente: make seq  e  make omp
```

Os executáveis são gerados na pasta `bin/`:

- Versão sequencial da Tarefa A: `bin/task_a_seq`
- Versão paralela (OpenMP) da Tarefa A: `bin/task_a_omp`

### Executar a matriz de experimentos

Para rodar toda a matriz de experimentos (todas as tarefas implementadas) e gerar o arquivo `results/results.csv`:

```bash
make run
```

O *script* `run.sh` será chamado automaticamente.

### Gerar gráficos

Para gerar os gráficos a partir do CSV (serão salvos em `plots/`):

```bash
make plot
```

O *script* `plot.py` será chamado automaticamente.


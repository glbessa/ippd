A implementação para a **Tarefa A — Laço irregular e políticas de schedule** exige a codificação de um *kernel* específico e a avaliação de três variantes de escalonamento (schedule) do OpenMP.

O **kernel** central a ser implementado dentro do laço paralelo é:
```
para i = 0..N-1, compute fib(i % K) e grave em v[i]
```
É crucial que a função `fib` (que calcula o número de Fibonacci) seja **custosa e implementada sem memoização**.

A implementação do código deve então explorar as seguintes diretivas OpenMP para o laço:

1.  **Variante 1 (Escalonamento Estático):**
    *   Uso da diretiva `#pragma omp parallel for schedule(static)`.
2.  **Variante 2 (Escalonamento Dinâmico):**
    *   Uso da diretiva `schedule(dynamic,chunk)`.
    *   Esta variante deve ser testada com valores de `chunk` pertencentes ao conjunto $\{1, 4, 16, 64\}$.
3.  **Variante 3 (Escalonamento Guiado):**
    *   Uso da diretiva `schedule(guided,chunk)`.
    *   Esta variante também deve ser testada com valores de `chunk` pertencentes ao conjunto $\{1, 4, 16, 64\}$.

A análise resultante deve focar no **Impacto de schedule e chunk** no desempenho e nos tempos de execução.

Para manter uma boa organização da região paralela, se for necessário ter dois laços paralelos em sequência, a diretriz é usar uma **única região `parallel`** que englobe os dois laços `for` internos, evitando assim o paralelismo aninhado desnecessário.
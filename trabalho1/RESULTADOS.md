## Tarefa A – Laço irregular e políticas de *schedule*

### Metodologia

- Kernel: para cada $i \in [0, N)$, computar `fib(i % K)` (sem memoização)
	e armazenar em `v[i]`.
- Parâmetros varridos (conforme `INSTRUCTIONS.md`):
	- $N \in \{100000, 500000, 1000000\}$
	- $K \in \{20, 24, 28\}$
	- Threads: $p \in \{1, 2, 4, 8\}$
- Para cada ponto $(N, K, p)$: 5 repetições, reportando média e desvio‑padrão
	do tempo total e do tempo de *kernel* (linhas `TOTAL_SECONDS` e
	`KERNEL_SECONDS`).
- Variantes de *schedule* avaliadas:
	- `static`
	- `dynamic_1`, `dynamic_4`, `dynamic_16`, `dynamic_64`
	- `guided_1`, `guided_4`, `guided_16`, `guided_64`
- Os dados brutos estão em `results/results.csv` e os gráficos gerados
	em `plots/` (arquivos `taskA_*.png`).

### Escalabilidade com *schedule(static)*

Como referência, a Tabela 1 resume o tempo médio total e o *speedup*
para a variante `static`, em um caso típico ($N = 100000$, $K = 20$):

**Tabela 1 – Tarefa A, `static`, N=100000, K=20**

| Threads | Tempo médio total (s) | Speedup vs 1 thread |
|--------:|----------------------:|--------------------:|
| 1       | 0.3303               | 1.0                 |
| 2       | 0.1674               | ≈ 2.0               |
| 4       | 0.1396               | ≈ 2.4               |
| 8       | 0.1017               | ≈ 3.3               |

Observações:

- A escalabilidade é razoável até 4 threads e ainda há ganho com 8
	threads, embora já distante do ideal (speedup 8).
- Para valores maiores de $N$ e $K$ (maior custo de `fib`), os gráficos
	em `taskA_N*_K*_speedup.png` mostram *speedups* melhores, próximos do
	ideal para 2–4 threads, pois há mais trabalho por thread e o *overhead*
	de criação/encerramento da região paralela torna‑se relativamente
	pequeno.

Gráfico ilustrativo (N=100000, K=20):

- Tempo total vs threads: `plots/taskA_N100000_K20_total.png`
- Tempo de *kernel* vs threads: `plots/taskA_N100000_K20_kernel.png`
- *Speedup* vs threads: `plots/taskA_N100000_K20_speedup.png`

### Comparação entre *schedules* (static, dynamic, guided)

Tomando como exemplo novamente $N = 100000$, $K = 20$ e 8 threads, os
tempos médios totais são aproximadamente:

**Tabela 2 – Tarefa A, N=100000, K=20, 8 threads**

| Variante          | Tempo médio total (s) |
|-------------------|----------------------:|
| `static`          | 0.1017               |
| `dynamic_1`       | 0.0813               |
| `dynamic_4`       | 0.0997               |
| `dynamic_16`      | 0.0798               |
| `dynamic_64`      | 0.0841               |
| `guided_1`        | 0.1166               |
| `guided_4`        | 0.0842               |
| `guided_16`       | 0.1016               |
| `guided_64`       | 0.1351               |

Padrões observados (corroborados pelos gráficos `taskA_N*_K*_total.png`
e `taskA_N*_K*_kernel.png`; ver, por exemplo,
`plots/taskA_N100000_K20_total.png` e `plots/taskA_N100000_K20_kernel.png`):

- Para cargas **mais leves** (N menor), `static` sofre um pouco mais com
	desbalanceamento, e variantes `dynamic_*` com *chunk* moderado
	(`dynamic_16`) apresentam até ~20–25% de ganho em relação a
	`static` em 8 threads.
- Para cargas **mais pesadas** (N ou K maiores), todos os *schedules*
	convergem para tempos muito próximos: o custo do kernel domina o
	*overhead* de escalonamento, e a diferença entre `static`, `dynamic`
	e `guided` fica dentro da variância experimental.
- *Chunks* extremos têm desvantagens claras:
	- *chunk* muito pequeno (1) aumenta o *overhead* de escalonamento e
		nem sempre compensa, especialmente para N grandes.
	- *chunk* muito grande (64) reduz a flexibilidade de redistribuição de
		iterações, podendo deixar alguns *threads* ociosos perto do fim do
		laço.

### Impacto de K (custo do kernel)

Nos gráficos para $K \in \{20, 24, 28\}$, mantendo N fixo, observa‑se:

- O tempo cresce rapidamente com K, pois o custo de `fib(i % K)` é
	dominado pelos valores altos de `i % K` (entre 0 e K−1), e `fib(n)`
	cresce exponencialmente em n.
- A forma das curvas de speedup é semelhante para diferentes K: o
	paralelismo compensa bem o aumento do custo do kernel, e o *shape* das
	curvas de speedup vs threads muda pouco; o efeito principal de K é
	alongar o eixo do tempo.

### Decisões de *schedule*

Com base nos dados e gráficos:

- Para cenários **equilibrados e com muito trabalho** (N grande, K
	moderado/alto), `schedule(static)` é simples, determinístico e
	apresenta desempenho equivalente às demais opções dentro da variância
	de medição.
- Para cenários **mais sensíveis a desbalanceamento** (N menor ou
	quando a irregularidade por iteração for maior), `schedule(dynamic,
	16)` mostrou um bom compromisso entre custo de escalonamento e
	redistribuição de carga, sendo uma escolha razoável como política
	"robusta".
- `guided` não trouxe ganhos consistentes neste kernel específico em
	relação a `dynamic` com *chunks* moderados; seu uso não foi adotado
	como padrão.

Em resumo, a implementação final da Tarefa A considera `static` como
*baseline* pela simplicidade, e recomenda `dynamic, chunk ≈ 16` quando
se deseja maior robustez a desbalanceamentos sem penalizar casos em que
o custo por iteração é elevado.

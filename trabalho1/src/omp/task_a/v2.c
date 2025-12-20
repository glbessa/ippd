#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/*
 * Variante 2 da Tarefa A: laço irregular com schedule(dynamic, chunk).
 * Kernel: para i = 0..N-1, compute fib(i % K) e grave em v[i].
 */

static long long fib(int n) {
	if (n <= 1) {
		return n;
	}
	return fib(n - 1) + fib(n - 2);
}

int main(int argc, char **argv) {
	if (argc != 5) {
		fprintf(stderr, "Uso: %s N K schedule chunk\n", argv[0]);
		return 1;
	}

	/* Leitura dos argumentos de linha de comando */
	const char *n_str = argv[1];
	const char *k_str = argv[2];
	const char *schedule_str = argv[3];
	const char *chunk_str = argv[4];

	long long N = atoll(n_str);
	long long K = atoll(k_str);
	long long chunk = atoll(chunk_str);

	if (N <= 0 || K <= 0) {
		fprintf(stderr, "Erro: N e K devem ser positivos. N=%lld K=%lld\n", N, K);
		return 1;
	}
	if (chunk <= 0) {
		fprintf(stderr, "Erro: chunk deve ser positivo para schedule(dynamic). chunk=%lld\n", chunk);
		return 1;
	}

	/* Para esta variante, esperamos schedule == "dynamic". */
	if (strcmp(schedule_str, "dynamic") != 0) {
		fprintf(stderr,
			"Aviso: variante 2 usa sempre schedule(dynamic,chunk); argumento schedule='%s' será ignorado.\n",
			schedule_str);
	}

	long long *v = (long long *)malloc((size_t)N * sizeof(long long));
	if (v == NULL) {
		fprintf(stderr, "Erro: falha ao alocar vetor de tamanho %lld.\n", N);
		return 1;
	}

	/* Medição de tempo total: começa após parsing e alocação. */
	double t_start_total = omp_get_wtime();

	/* Medição de tempo de kernel: apenas o laço paralelo. */
	double t_start_kernel = omp_get_wtime();

	#pragma omp parallel for schedule(dynamic, chunk)
	for (long long i = 0; i < N; i++) {
		int idx = (int)(i % K);
		v[i] = fib(idx);
	}

	double t_end_kernel = omp_get_wtime();

	/* Pequeno pós-processamento para evitar que o compilador elimine o laço. */
	long long checksum = 0;
	for (long long i = 0; i < N; i++) {
		checksum += v[i];
	}

	double t_end_total = omp_get_wtime();

	double kernel_seconds = t_end_kernel - t_start_kernel;
	double total_seconds = t_end_total - t_start_total;

	/* Saída no formato esperado por run.sh */
	printf("CHECKSUM=%lld\n", checksum);
	printf("TOTAL_SECONDS=%.9f\n", total_seconds);
	printf("KERNEL_SECONDS=%.9f\n", kernel_seconds);

	free(v);

	return 0;
}


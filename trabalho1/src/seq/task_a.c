#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * Versão sequencial da Tarefa A (laço irregular).
 * Kernel: para i = 0..N-1, compute fib(i % K) e grave em v[i].
 */

static long long fib(int n) {
	if (n <= 1) {
		return n;
	}
	return fib(n - 1) + fib(n - 2);
}

static double now_seconds(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
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

	(void)schedule_str; /* Não usamos schedule na versão sequencial */
	(void)chunk_str;    /* Nem chunk */

	long long N = atoll(n_str);
	long long K = atoll(k_str);

	if (N <= 0 || K <= 0) {
		fprintf(stderr, "Erro: N e K devem ser positivos. N=%lld K=%lld\n", N, K);
		return 1;
	}

	long long *v = (long long *)malloc((size_t)N * sizeof(long long));
	if (v == NULL) {
		fprintf(stderr, "Erro: falha ao alocar vetor de tamanho %lld.\n", N);
		return 1;
	}

	double t_start_total = now_seconds();
	double t_start_kernel = now_seconds();

	for (long long i = 0; i < N; i++) {
		int idx = (int)(i % K);
		v[i] = fib(idx);
	}

	double t_end_kernel = now_seconds();

	long long checksum = 0;
	for (long long i = 0; i < N; i++) {
		checksum += v[i];
	}

	double t_end_total = now_seconds();

	double kernel_seconds = t_end_kernel - t_start_kernel;
	double total_seconds = t_end_total - t_start_total;

	printf("CHECKSUM=%lld\n", checksum);
	printf("TOTAL_SECONDS=%.9f\n", total_seconds);
	printf("KERNEL_SECONDS=%.9f\n", kernel_seconds);

	free(v);
	return 0;
}

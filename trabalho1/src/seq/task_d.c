
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * Versão sequencial da Tarefa D (organização de região paralela).
 *
 * A versão paralela irá comparar duas formas de executar DOIS laços:
 *   1) Variante ingênua: dois "parallel for" consecutivos.
 *   2) Variante arrumada: uma única região "parallel" englobando dois "for".
 *
 * Aqui implementamos apenas o kernel SEQUENCIAL correspondente, isto é,
 * dois laços simples em sequência sobre vetores de tamanho N. A string
 * "variant" é aceita apenas para manter a mesma interface de linha de
 * comando da versão paralela (task_d_omp), mas é ignorada.
 */

static double now_seconds(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

int main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "Uso: %s N variant\n", argv[0]);
		return 1;
	}

	const char *n_str = argv[1];
	const char *variant_str = argv[2];
	(void)variant_str; /* Ignorado na versão sequencial */

	long long N = atoll(n_str);
	if (N <= 0) {
		fprintf(stderr, "Erro: N deve ser positivo. N=%lld\n", N);
		return 1;
	}

	double *a = (double *)malloc((size_t)N * sizeof(double));
	double *b = (double *)malloc((size_t)N * sizeof(double));
	if (a == NULL || b == NULL) {
		fprintf(stderr, "Erro: falha ao alocar vetores de tamanho %lld.\n", N);
		free(a);
		free(b);
		return 1;
	}

	double t_start_total = now_seconds();
	double t_start_kernel = now_seconds();

	/* Primeiro laço: inicializa o vetor a[] com alguma computação simples. */
	for (long long i = 0; i < N; i++) {
		a[i] = (double)i * 0.5;
	}

	/* Segundo laço: usa a[] para preencher b[] com outra computação simples. */
	for (long long i = 0; i < N; i++) {
		b[i] = a[i] * a[i] + 1.0;
	}

	double t_end_kernel = now_seconds();

	double checksum = 0.0;
	for (long long i = 0; i < N; i++) {
		checksum += b[i];
	}

	double t_end_total = now_seconds();

	double kernel_seconds = t_end_kernel - t_start_kernel;
	double total_seconds = t_end_total - t_start_total;

	printf("CHECKSUM=%.9f\n", checksum);
	printf("TOTAL_SECONDS=%.9f\n", total_seconds);
	printf("KERNEL_SECONDS=%.9f\n", kernel_seconds);

	free(a);
	free(b);
	return 0;
}


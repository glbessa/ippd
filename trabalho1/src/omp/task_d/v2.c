
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/*
 * Tarefa D — Organização de região paralela (variante arrumada).
 *
 * Versão paralela correspondente ao kernel sequencial em src/seq/task_d.c,
 * porém usando UMA ÚNICA região "#pragma omp parallel" que engloba os dois
 * laços internos (boa organização de região paralela).
 *
 * Kernel:
 *   - Dois laços em sequência sobre vetores de tamanho N.
 *       1) a[i] = i * 0.5
 *       2) b[i] = a[i] * a[i] + 1.0
 *
 * Interface simples:
 *   ./task_d_v2_omp N
 *   (lê número de threads via OMP_NUM_THREADS, como de costume em OpenMP).
 */

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Uso: %s N\n", argv[0]);
		return 1;
	}

	const char *n_str = argv[1];
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

	/* Tempo total: após parsing e alocação, até após o checksum. */
	double t_start_total = omp_get_wtime();

	double t_start_kernel = omp_get_wtime();
	double t_end_kernel;

	/* Variante arrumada: única região parallel com dois for internos. */
	#pragma omp parallel
	{
		#pragma omp for
		for (long long i = 0; i < N; i++) {
			a[i] = (double)i * 0.5;
		}

		#pragma omp for
		for (long long i = 0; i < N; i++) {
			b[i] = a[i] * a[i] + 1.0;
		}
	}

	t_end_kernel = omp_get_wtime();

	double checksum = 0.0;
	for (long long i = 0; i < N; i++) {
		checksum += b[i];
	}

	double t_end_total = omp_get_wtime();

	double kernel_seconds = t_end_kernel - t_start_kernel;
	double total_seconds = t_end_total - t_start_total;

	printf("CHECKSUM=%.9f\n", checksum);
	printf("TOTAL_SECONDS=%.9f\n", total_seconds);
	printf("KERNEL_SECONDS=%.9f\n", kernel_seconds);

	free(a);
	free(b);
	return 0;
}



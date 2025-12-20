
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/*
 * Tarefa D — Organização de região paralela (versão paralela v1).
 *
 * Nesta versão v1 implementamos APENAS a variante ingênua
 *   "two_parallel_for": dois "#pragma omp parallel for" consecutivos,
 *   correspondente a dois laços em sequência sobre vetores de tamanho N:
 *     1) a[i] = i * 0.5
 *     2) b[i] = a[i] * a[i] + 1.0
 *
 * A interface ainda recebe o parâmetro "variant" para ser compatível com
 * o script run.sh, mas qualquer valor diferente de "two_parallel_for"
 * resulta em erro.
 */

int main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "Uso: %s N variant\n", argv[0]);
		fprintf(stderr, "  variant ∈ {two_parallel_for, single_parallel}\n");
		return 1;
	}

	const char *n_str = argv[1];
	const char *variant_str = argv[2];

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

	double t_start_kernel = 0.0;
	double t_end_kernel = 0.0;

	if (strcmp(variant_str, "two_parallel_for") == 0) {
		/* Variante ingênua: dois parallel for consecutivos. */
		t_start_kernel = omp_get_wtime();

		#pragma omp parallel for
		for (long long i = 0; i < N; i++) {
			a[i] = (double)i * 0.5;
		}

		#pragma omp parallel for
		for (long long i = 0; i < N; i++) {
			b[i] = a[i] * a[i] + 1.0;
		}

		t_end_kernel = omp_get_wtime();
	} else {
		fprintf(stderr,
			"Erro: variant inválida '%s'. Use apenas two_parallel_for.\n",
			variant_str);
		free(a);
		free(b);
		return 1;
	}

	/* Pós-processamento sequencial para evitar que o compilador elimine o trabalho. */
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



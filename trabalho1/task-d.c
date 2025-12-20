// Experimento de overhead OpenMP: duas regioes parallel for ingênuas vs uma região parallel arrumada englobando dois for.
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

static void inicializa_entrada(double *vetor_a, double *vetor_b, size_t n_elementos) {
	for (size_t i = 0; i < n_elementos; i++) {
		vetor_a[i] = (double)(i % 1024) * 0.5;
		vetor_b[i] = 0.0;
	}
}

static double soma_verificacao(const double *vetor, size_t n_elementos) {
	double acumulador = 0.0;
	for (size_t i = 0; i < n_elementos; i++) {
		acumulador += vetor[i];
	}
	return acumulador;
}

static double executa_ingenua(double *vetor_a, double *vetor_b, size_t n_elementos) {
	double t_inicio = omp_get_wtime();

	#pragma omp parallel for schedule(static)
	for (size_t i = 0; i < n_elementos; i++) {
		vetor_a[i] = vetor_a[i] * 1.0001 + 1.0;
	}

	#pragma omp parallel for schedule(static)
	for (size_t i = 0; i < n_elementos; i++) {
		vetor_b[i] = vetor_a[i] * 2.0 + 3.0;
	}

	double t_fim = omp_get_wtime();
	return t_fim - t_inicio;
}

static double executa_arrumada(double *vetor_a, double *vetor_b, size_t n_elementos) {
	double t_inicio = omp_get_wtime();

	#pragma omp parallel
	{
		#pragma omp for schedule(static)
		for (size_t i = 0; i < n_elementos; i++) {
			vetor_a[i] = vetor_a[i] * 1.0001 + 1.0;
		}

		#pragma omp for schedule(static)
		for (size_t i = 0; i < n_elementos; i++) {
			vetor_b[i] = vetor_a[i] * 2.0 + 3.0;
		}
	}

	double t_fim = omp_get_wtime();
	return t_fim - t_inicio;
}

int main(int argc, char **argv) {
	size_t n_elementos = 1000000UL;
	int repeticoes = 5;
	int num_threads = 0; // 0 significa usar o padrão do OMP

	if (argc > 1) {
		n_elementos = strtoull(argv[1], NULL, 10);
	}
	if (argc > 2) {
		repeticoes = atoi(argv[2]);
		if (repeticoes < 1) {
			repeticoes = 1;
		}
	}
	if (argc > 3) {
		num_threads = atoi(argv[3]);
	}

	if (num_threads > 0) {
		omp_set_num_threads(num_threads);
	}

	num_threads = omp_get_max_threads();

	double *vetor_a = (double *)malloc(n_elementos * sizeof(double));
	double *vetor_b = (double *)malloc(n_elementos * sizeof(double));
	if (!vetor_a || !vetor_b) {
		fprintf(stderr, "Erro ao alocar vetores.\n");
		free(vetor_a);
		free(vetor_b);
		return 1;
	}

	// Aquecimento para reduzir ruído de primeira execução.
	inicializa_entrada(vetor_a, vetor_b, n_elementos);
	executa_ingenua(vetor_a, vetor_b, n_elementos);

	double total_ingenua = 0.0;
	double total_arrumada = 0.0;
	double ultima_soma = 0.0;

	for (int r = 0; r < repeticoes; r++) {
		inicializa_entrada(vetor_a, vetor_b, n_elementos);
		total_ingenua += executa_ingenua(vetor_a, vetor_b, n_elementos);
		ultima_soma = soma_verificacao(vetor_b, n_elementos);

		inicializa_entrada(vetor_a, vetor_b, n_elementos);
		total_arrumada += executa_arrumada(vetor_a, vetor_b, n_elementos);
	}

	printf("variant,n,threads,reps,avg_time_sec,checksum\n");
	printf("naive,%zu,%d,%d,%.6f,%.6f\n", n_elementos, num_threads, repeticoes, total_ingenua / repeticoes, ultima_soma);
	printf("tidy,%zu,%d,%d,%.6f,%.6f\n", n_elementos, num_threads, repeticoes, total_arrumada / repeticoes, ultima_soma);

	free(vetor_a);
	free(vetor_b);
	return 0;
}

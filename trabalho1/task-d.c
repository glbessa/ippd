// OpenMP overhead experiment: naive vs tidy parallel regions for two loops.
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

static void init_input(double *a, double *b, size_t n) {
	for (size_t i = 0; i < n; i++) {
		a[i] = (double)(i % 1024) * 0.5;
		b[i] = 0.0;
	}
}

static double checksum(const double *v, size_t n) {
	double acc = 0.0;
	for (size_t i = 0; i < n; i++) {
		acc += v[i];
	}
	return acc;
}

static double run_naive(double *a, double *b, size_t n) {
	double t0 = omp_get_wtime();

	#pragma omp parallel for schedule(static)
	for (size_t i = 0; i < n; i++) {
		a[i] = a[i] * 1.0001 + 1.0;
	}

	#pragma omp parallel for schedule(static)
	for (size_t i = 0; i < n; i++) {
		b[i] = a[i] * 2.0 + 3.0;
	}

	double t1 = omp_get_wtime();
	return t1 - t0;
}

static double run_tidy(double *a, double *b, size_t n) {
	double t0 = omp_get_wtime();

	#pragma omp parallel
	{
		#pragma omp for schedule(static)
		for (size_t i = 0; i < n; i++) {
			a[i] = a[i] * 1.0001 + 1.0;
		}

		#pragma omp for schedule(static)
		for (size_t i = 0; i < n; i++) {
			b[i] = a[i] * 2.0 + 3.0;
		}
	}

	double t1 = omp_get_wtime();
	return t1 - t0;
}

int main(int argc, char **argv) {
	size_t n = 1000000UL;
	int reps = 5;
	int threads = 0; // 0 means use OMP default

	if (argc > 1) {
		n = strtoull(argv[1], NULL, 10);
	}
	if (argc > 2) {
		reps = atoi(argv[2]);
		if (reps < 1) {
			reps = 1;
		}
	}
	if (argc > 3) {
		threads = atoi(argv[3]);
	}

	if (threads > 0) {
		omp_set_num_threads(threads);
	}

	threads = omp_get_max_threads();

	double *a = (double *)malloc(n * sizeof(double));
	double *b = (double *)malloc(n * sizeof(double));
	if (!a || !b) {
		fprintf(stderr, "Erro ao alocar vetores.\n");
		free(a);
		free(b);
		return 1;
	}

	// Warm-up run to reduce first-use noise.
	init_input(a, b, n);
	run_naive(a, b, n);

	double total_naive = 0.0;
	double total_tidy = 0.0;
	double last_checksum = 0.0;

	for (int r = 0; r < reps; r++) {
		init_input(a, b, n);
		total_naive += run_naive(a, b, n);
		last_checksum = checksum(b, n);

		init_input(a, b, n);
		total_tidy += run_tidy(a, b, n);
	}

	printf("variant,n,threads,reps,avg_time_sec,checksum\n");
	printf("naive,%zu,%d,%d,%.6f,%.6f\n", n, threads, reps, total_naive / reps, last_checksum);
	printf("tidy,%zu,%d,%d,%.6f,%.6f\n", n, threads, reps, total_tidy / reps, last_checksum);

	free(a);
	free(b);
	return 0;
}

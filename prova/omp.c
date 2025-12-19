// Resultados:


// Resultados 1M schedule runtime
// Parametros:
//   N (treino)   = 1000000
//   M (consultas)= 2000
//   k (vizinhos) = 8

// Acumulados:
//   global_sum   = 0.035824642067694625
//   sum_k[0]     = 1.7554499217198316e-05
//   sum_k[M-1]   = 1.2502539908587629e-05

// Execucao paralela:
//   threads      = 8
//   tempo        = 0.523750 s



// Resultados 10M schedule runtime
// Parametros:
//   N (treino)   = 10000000
//   M (consultas)= 2000
//   k (vizinhos) = 8

// Acumulados:
//   global_sum   = 0.0035368502156536948
//   sum_k[0]     = 1.7345882960123626e-06
//   sum_k[M-1]   = 1.5078112490241047e-06

// Execucao paralela:
//   threads      = 8
//   tempo        = 9.468315 s



// Resultados 10M schedule(static)
// Parametros:
//   N (treino)   = 10000000
//   M (consultas)= 2000
//   k (vizinhos) = 8

// Acumulados:
//   global_sum   = 0.0035368502156536944
//   sum_k[0]     = 1.7345882960123626e-06
//   sum_k[M-1]   = 1.5078112490241047e-06

// Execucao paralela:
//   threads      = 8
//   tempo        = 8.356272 s



// Resultados 10M schedule(static) 4 threads
// Parametros:
//   N (treino)   = 10000000
//   M (consultas)= 2000
//   k (vizinhos) = 8

// Acumulados:
//   global_sum   = 0.0035368502156536948
//   sum_k[0]     = 1.7345882960123626e-06
//   sum_k[M-1]   = 1.5078112490241047e-06

// Execucao paralela:
//   threads      = 4
//   tempo        = 7.244153 s



// Resultados 10M schedule(dynamic)
// Parametros:
//   N (treino)   = 10000000
//   M (consultas)= 2000
//   k (vizinhos) = 8

// Acumulados:
//   global_sum   = 0.0035368502156536944
//   sum_k[0]     = 1.7345882960123626e-06
//   sum_k[M-1]   = 1.5078112490241047e-06

// Execucao paralela:
//   threads      = 8
//   tempo        = 8.735684 s



// Resultados 10M schedule(dynamic) 4 threads
// Parametros:
//   N (treino)   = 10000000
//   M (consultas)= 2000
//   k (vizinhos) = 8

// Acumulados:
//   global_sum   = 0.0035368502156536948
//   sum_k[0]     = 1.7345882960123626e-06
//   sum_k[M-1]   = 1.5078112490241047e-06

// Execucao paralela:
//   threads      = 4
//   tempo        = 8.272772 s


// Resultados 10M schedule(guided)
// Parametros:
//   N (treino)   = 10000000
//   M (consultas)= 2000
//   k (vizinhos) = 8

// Acumulados:
//   global_sum   = 0.0035368502156536948
//   sum_k[0]     = 1.7345882960123626e-06
//   sum_k[M-1]   = 1.5078112490241047e-06

// Execucao paralela:
//   threads      = 8
//   tempo        = 8.881167 s



// Resultados 10M schedule(guided) 4 threads
// Parametros:
//   N (treino)   = 10000000
//   M (consultas)= 2000
//   k (vizinhos) = 8

// Acumulados:
//   global_sum   = 0.0035368502156536948
//   sum_k[0]     = 1.7345882960123626e-06
//   sum_k[M-1]   = 1.5078112490241047e-06

// Execucao paralela:
//   threads      = 4
//   tempo        = 6.579765 s

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

static inline void topk_insert(double *best, int k, double x) {
    // Mantém best[0..k-1] em ordem crescente (best[0] menor).
    // Inserção por deslocamento, O(k). k é pequeno.
    if (x >= best[k - 1]) return;

    int pos = k - 1;
    while (pos > 0 && x < best[pos - 1]) {
        best[pos] = best[pos - 1];
        pos--;
    }
    best[pos] = x;
}

int main(void) {
    // Ajuste aqui se quiser.
    const int N = 10000000;   // treino
    const int M = 2000;      // consultas
    const int k = 8;         // vizinhos
    const unsigned seed = 0;

    double *train = (double*)malloc((size_t)N * sizeof(double));
    double *query = (double*)malloc((size_t)M * sizeof(double));
    double *sum_k = (double*)malloc((size_t)M * sizeof(double));
    if (!train || !query || !sum_k) {
        fprintf(stderr, "Falha de alocacao.\n");
        free(train); free(query); free(sum_k);
        return 1;
    }

    srand(seed);
    for (int i = 0; i < N; i++)  // Inicializa no range [0, 1)
        train[i] = (double)rand() / (double)RAND_MAX;
  
    for (int j = 0; j < M; j++)
        query[j] = (double)rand() / (double)RAND_MAX;
  

    double global_sum = 0.0;

    int num_threads = 0;
    #pragma omp parallel
    {
        #pragma omp single
        num_threads = omp_get_num_threads();
    }

    double start_time = omp_get_wtime();

    #pragma omp parallel for reduction(+:global_sum) schedule(dynamic)
    for (int j = 0; j < M; j++) {     // Inicializa top-k com +infinito
        double best[64]; // aqui pode dar erro se extrapolar, mas usaremos só k
        for (int t = 0; t < k; t++) best[t] = INFINITY;

        const double q = query[j];

        for (int i = 0; i < N; i++) {
            double d = fabs(train[i] - q);
            topk_insert(best, k, d);
        }

        double s = 0.0;
        for (int t = 0; t < k; t++) s += best[t];

        sum_k[j] = s;
        global_sum += s;
    }

    double end_time = omp_get_wtime();
    double elapsed = end_time - start_time;

    printf("================ RESULTADOS KNN =================\n");
    printf("Parametros:\n");
    printf("  N (treino)   = %d\n", N);
    printf("  M (consultas)= %d\n", M);
    printf("  k (vizinhos) = %d\n", k);

    printf("\nAcumulados:\n");
    printf("  global_sum   = %.17g\n", global_sum);
    printf("  sum_k[0]     = %.17g\n", sum_k[0]);
    printf("  sum_k[M-1]   = %.17g\n", sum_k[M - 1]);

    printf("\nExecucao paralela:\n");
    printf("  threads      = %d\n", num_threads);
    printf("  tempo        = %.6f s\n", elapsed);

    free(train);
    free(query);
    free(sum_k);
    return 0;
}
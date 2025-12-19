#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
    const int N = 1000000;   // treino
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

    printf("N=%d M=%d k=%d\n", N, M, k);
    printf("global_sum=%.17g\n", global_sum);
    printf("sum_k[0]=%.17g sum_k[M-1]=%.17g\n", sum_k[0], sum_k[M - 1]);

    free(train);
    free(query);
    free(sum_k);
    return 0;
}
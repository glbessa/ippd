#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    long n = 100000000000; // Reduzi para 100M para ser mais rápido no teste
    double h = 1.0 / (double)n;
    double sum = 0.0, x, pi;
    int size, rank, i;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Cada um calcula sua parte
    for (i = rank + 1; i <= n; i += size) {
        x = h * ((double)i - 0.5);
        sum += (4.0 / (1.0 + x * x));
    }
    double my_pi = h * sum;

    MPI_Reduce(&my_pi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    /*
    if (rank != 0) {
        // Workers enviam seus resultados para o Mestre
        MPI_Send(&my_pi, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    } else {
        // Mestre começa com o seu próprio valor
        pi = my_pi;
        double tmp;
        for (int j = 1; j < size; j++) {
            // Mestre recebe de cada worker e soma manualmente
            MPI_Recv(&tmp, 1, MPI_DOUBLE, j, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            pi += tmp;
        }
        printf("Pi calculado manualmente: %.16f\n", pi);
    }
    */

    if (rank == 0)
        printf("Pi calculado manualmente: %.16f\n", pi);

    MPI_Finalize();

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

#define TOTAL_POINTS 100000000

double random_double(double min, double max);

int main(int argc, char** argv)
{
    int rank, size;
    long long int local_points, local_inside = 0, total_inside;
    double x, y;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(rank + time(NULL));

    local_points = TOTAL_POINTS / size;

    for (long long int i = 0; i < local_points; i++) {
        x = random_double(-1, 1);
        y = random_double(-1, 1);
        if (x * x + y * y <= 1.0)
            local_inside++;
    }

    // MPI_Reduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
    MPI_Reduce(&local_inside, &total_inside, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double pi = 4.0 * (double) total_inside / TOTAL_POINTS;
        printf("Estimativa de π \t= %.10f\n", pi);
        printf("Erro \t\t\t= %.10f %%\n", fabs(M_PI - pi) / M_PI * 100);
    }

    MPI_Finalize();
    return 0;
}

double random_double(double min, double max)
{
    return (double) rand() / RAND_MAX * (max - min) + min;
}
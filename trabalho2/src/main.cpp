#include <iostream>
#include <mpi.h>
#include <omp.h>
#include "territorio.hpp"

int main(int argc, char** argv) {
    int rank, size;
    
    // Inicialização do MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Configurações Globais Sugeridas (Devem ser múltiplos do número de proc MPI)
    int TOTAL_CICLOS = 10;   // Número total de ciclos
    int TAMANHO_CICLO_SAZONAL = 4;    // Tamanho do ciclo sazonal
    int LARGURA_GRID = 1000; // Dimensões do grid global
    int ALTURA_GRID = 1000;
    
    // Divisão do subgrid local (Simplificada: divisão 1D nas linhas do Grid Global)
    // Para simplificar a demonstração, o particionamento será efetuado pelo número de processos
    int local_height = ALTURA_GRID / size;
    int local_offsetY = rank * local_height;
    int local_width = LARGURA_GRID; 
    int local_offsetX = 0;
    
    // Instancia o território local particionado
    Territorio subgrid(local_width, local_height, Posicao(local_offsetX, local_offsetY));
    Estacao estacao_atual = Estacao::SECA;
    
    // Inicialização OpenMP paralela (First Touch Policy)
    subgrid.inicializar(estacao_atual);
    
    if (rank == 0) {
        std::cout << "Simulação Sazonal Indígena inicializada com " << size << " processos." << std::endl;
        #pragma omp parallel
        {
            #pragma omp single
            std::cout << "OpenMP Threads disponiveis por MPI rank: " << omp_get_num_threads() << std::endl;
        }
    }
    
    // Simulação principal
    for (int t = 0; t < TOTAL_CICLOS; ++t) {
        // 5.1 Atualizar estação
        if (t > 0 && t % TAMANHO_CICLO_SAZONAL == 0) {
            estacao_atual = (estacao_atual == Estacao::SECA) ? Estacao::CHEIA : Estacao::SECA;
            subgrid.atualizar_acessibilidade(estacao_atual);
            if (rank == 0) std::cout << "Mudanca de estacao no ciclo " << t << std::endl;
        }
        
        // 5.2 Troca de halo MPI (ainda a ser abstraída/implementada dependendo do particionamento)
        // ...
        
        // 5.3 Processar agentes com OpenMP (ainda a ser implementado)
        // Aqui dentro as threads executarão:
        // subgrid.registrar_consumo(x, y, consumo);
        // ...
        
        // 5.4 Migração de agentes com MPI (ainda a ser implementado)
        // ...
        
        // 5.5 Atualizar grid local via OpenMP paralelizável
        subgrid.atualizar_recursos(estacao_atual);
        
        // Barrerira MPI por garantia de ciclo síncrono
        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    if (rank == 0) {
        std::cout << "Simulacao concluida." << std::endl;
    }
    
    MPI_Finalize();
    return 0;
}

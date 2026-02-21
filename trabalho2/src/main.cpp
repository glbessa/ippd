#include <iostream>
#include <vector>
#include <cstdlib>
#include <mpi.h>
#include <omp.h>
#include "territorio.hpp"
#include "agente.hpp"

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
    subgrid.alocar_halos(rank > 0, rank < size - 1);
    
    srand(12345 + rank); // Seed por processo para garantir reprodutibilidade na execução 
    
    // 4) Inicializar agentes locais
    int N_AGENTS = 10000;
    int local_agents_count = N_AGENTS / size;
    std::vector<Agente> agentes_locais;
    for (int i = 0; i < local_agents_count; ++i) {
        int gx = rand() % local_width + local_offsetX;
        int gy = rand() % local_height + local_offsetY;
        agentes_locais.push_back(Agente(rank * local_agents_count + i, Posicao(gx, gy), 100.0f));
    }

    // Criar datatypes MPI para as estruturas
    MPI_Datatype mpi_celula;
    MPI_Type_contiguous(sizeof(Celula), MPI_BYTE, &mpi_celula);
    MPI_Type_commit(&mpi_celula);

    MPI_Datatype mpi_agente;
    MPI_Type_contiguous(sizeof(Agente), MPI_BYTE, &mpi_agente);
    MPI_Type_commit(&mpi_agente);
    
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
        
        // 5.2 Troca de halo MPI
        MPI_Request reqs[4];
        int num_reqs = 0;
        
        if (rank > 0) {
            MPI_Irecv(subgrid.ptr_halo_sup(), local_width, mpi_celula, rank - 1, 0, MPI_COMM_WORLD, &reqs[num_reqs++]);
            MPI_Isend(subgrid.ptr_linha_sup(), local_width, mpi_celula, rank - 1, 1, MPI_COMM_WORLD, &reqs[num_reqs++]);
        }
        if (rank < size - 1) {
            MPI_Irecv(subgrid.ptr_halo_inf(), local_width, mpi_celula, rank + 1, 1, MPI_COMM_WORLD, &reqs[num_reqs++]);
            MPI_Isend(subgrid.ptr_linha_inf(), local_width, mpi_celula, rank + 1, 0, MPI_COMM_WORLD, &reqs[num_reqs++]);
        }
        if (num_reqs > 0) {
            MPI_Waitall(num_reqs, reqs, MPI_STATUSES_IGNORE);
        }
        
        // 5.3 Processar agentes com OpenMP
        std::vector<Agente> buffer_envio_cima;
        std::vector<Agente> buffer_envio_baixo;
        std::vector<Agente> nova_lista_local;

        #pragma omp parallel
        {
            std::vector<Agente> envio_cima_thread;
            std::vector<Agente> envio_baixo_thread;
            std::vector<Agente> lista_local_thread;

            #pragma omp for
            for (int i = 0; i < (int)agentes_locais.size(); ++i) {
                const Agente& a = agentes_locais[i];
                Posicao celula_atual = a.get_posicao();
                int lnx = celula_atual.x - local_offsetX;
                int lny = celula_atual.y - local_offsetY;
                
                float r = 0;
                if(lny >= 0 && lny < local_height && lnx >= 0 && lnx < local_width) {
                     r = subgrid.get_celula(Posicao(lnx, lny)).recurso;
                }
                
                a.executar_carga(r);
                
                Posicao destino;
                a.decidir(subgrid, destino);
                
                Agente a_atualizado = a;
                a_atualizado.set_posicao(destino);
                
                int dest_y = destino.y;
                if (dest_y < local_offsetY) {
                    if (rank > 0) envio_cima_thread.push_back(a_atualizado);
                } else if (dest_y >= local_offsetY + local_height) {
                    if (rank < size - 1) envio_baixo_thread.push_back(a_atualizado);
                } else {
                    a_atualizado.consumir_recurso(subgrid);
                    lista_local_thread.push_back(a_atualizado);
                }
            }

            #pragma omp critical
            {
                buffer_envio_cima.insert(buffer_envio_cima.end(), envio_cima_thread.begin(), envio_cima_thread.end());
                buffer_envio_baixo.insert(buffer_envio_baixo.end(), envio_baixo_thread.begin(), envio_baixo_thread.end());
                nova_lista_local.insert(nova_lista_local.end(), lista_local_thread.begin(), lista_local_thread.end());
            }
        }
        
        // 5.4 Migração de agentes com MPI
        int recv_cima_size = 0, recv_baixo_size = 0;
        int send_cima_size = buffer_envio_cima.size();
        int send_baixo_size = buffer_envio_baixo.size();
        
        MPI_Request reqs_migr[8];
        int num_reqs_migr = 0;
        
        if (rank > 0) {
            MPI_Irecv(&recv_cima_size, 1, MPI_INT, rank - 1, 2, MPI_COMM_WORLD, &reqs_migr[num_reqs_migr++]);
            MPI_Isend(&send_cima_size, 1, MPI_INT, rank - 1, 3, MPI_COMM_WORLD, &reqs_migr[num_reqs_migr++]);
        }
        if (rank < size - 1) {
            MPI_Irecv(&recv_baixo_size, 1, MPI_INT, rank + 1, 3, MPI_COMM_WORLD, &reqs_migr[num_reqs_migr++]);
            MPI_Isend(&send_baixo_size, 1, MPI_INT, rank + 1, 2, MPI_COMM_WORLD, &reqs_migr[num_reqs_migr++]);
        }
        if (num_reqs_migr > 0) {
            MPI_Waitall(num_reqs_migr, reqs_migr, MPI_STATUSES_IGNORE);
        }
        
        std::vector<Agente> recv_buffer_cima(recv_cima_size);
        std::vector<Agente> recv_buffer_baixo(recv_baixo_size);
        num_reqs_migr = 0;
        
        if (rank > 0) {
            if (recv_cima_size > 0)
                MPI_Irecv(recv_buffer_cima.data(), recv_cima_size, mpi_agente, rank - 1, 4, MPI_COMM_WORLD, &reqs_migr[num_reqs_migr++]);
            if (send_cima_size > 0)
                MPI_Isend(buffer_envio_cima.data(), send_cima_size, mpi_agente, rank - 1, 5, MPI_COMM_WORLD, &reqs_migr[num_reqs_migr++]);
        }
        if (rank < size - 1) {
            if (recv_baixo_size > 0)
                MPI_Irecv(recv_buffer_baixo.data(), recv_baixo_size, mpi_agente, rank + 1, 5, MPI_COMM_WORLD, &reqs_migr[num_reqs_migr++]);
            if (send_baixo_size > 0)
                MPI_Isend(buffer_envio_baixo.data(), send_baixo_size, mpi_agente, rank + 1, 4, MPI_COMM_WORLD, &reqs_migr[num_reqs_migr++]);
        }
        if (num_reqs_migr > 0) {
            MPI_Waitall(num_reqs_migr, reqs_migr, MPI_STATUSES_IGNORE);
        }
        
        agentes_locais = std::move(nova_lista_local);
        agentes_locais.insert(agentes_locais.end(), recv_buffer_cima.begin(), recv_buffer_cima.end());
        agentes_locais.insert(agentes_locais.end(), recv_buffer_baixo.begin(), recv_buffer_baixo.end());

        // 5.5 Atualizar grid local via OpenMP paralelizável
        subgrid.atualizar_recursos(estacao_atual);
        
        // 5.6 Métricas globais
        int local_num_agentes = agentes_locais.size();
        int global_num_agentes = 0;
        MPI_Allreduce(&local_num_agentes, &global_num_agentes, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        
        if (rank == 0) {
            std::cout << "Ciclo " << t << " - Agentes Globais: " << global_num_agentes << std::endl;
        }

        // 5.7 Barreira MPI por garantia de ciclo síncrono
        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    MPI_Type_free(&mpi_celula);
    MPI_Type_free(&mpi_agente);
    
    if (rank == 0) {
        std::cout << "Simulacao concluida." << std::endl;
    }
    
    MPI_Finalize();
    return 0;
}

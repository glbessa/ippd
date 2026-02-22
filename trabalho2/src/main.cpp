#include <iostream>
#include <vector>
#include <cstdlib>
#include <mpi.h>
#include <omp.h>
#include "territorio.hpp"
#include "agente.hpp"
#include "config.hpp"

// Protótipos das funções auxiliares
std::vector<Agente> inicializar_agentes_locais(int size, int rank, int local_width, int local_height, int local_offsetX, int local_offsetY);
void trocar_halos_territorio(Territorio& subgrid, int local_width, MPI_Datatype mpi_celula, int rank, int size);
void processar_agentes(const std::vector<Agente>& agentes_locais, Territorio& subgrid, int local_offsetX, int local_offsetY, int local_width, int local_height, int rank, int size, std::vector<Agente>& nova_lista_local, std::vector<Agente>& buffer_envio_cima, std::vector<Agente>& buffer_envio_baixo);
void migrar_agentes_entre_processos(int rank, int size, MPI_Datatype mpi_agente, std::vector<Agente>& agentes_locais, std::vector<Agente>& nova_lista_local, std::vector<Agente>& buffer_envio_cima, std::vector<Agente>& buffer_envio_baixo);

int main(int argc, char** argv) {
    int rank, size;
    
    // Inicialização do MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Divisão do subgrid local (Simplificada: divisão 1D nas linhas do Grid Global)
    // Para simplificar a demonstração, o particionamento será efetuado pelo número de processos
    int local_height = Config::ALTURA_GRID / size;
    int local_offsetY = rank * local_height;
    int local_width = Config::LARGURA_GRID; 
    int local_offsetX = 0;
    
    // Instancia o território local particionado
    Territorio subgrid(local_width, local_height, Posicao(local_offsetX, local_offsetY));
    Estacao estacao_atual = Estacao::SECA;
    
    // Inicialização OpenMP paralela (First Touch Policy)
    subgrid.inicializar(estacao_atual);
    subgrid.alocar_halos(rank > 0, rank < size - 1);
    
    srand(Config::SEED); // Seed por processo para garantir reprodutibilidade na execução 
    
    // 4) Inicializar agentes locais
    std::vector<Agente> agentes_locais = inicializar_agentes_locais(size, rank, local_width, local_height, local_offsetX, local_offsetY);
    
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
    for (int t = 0; t < Config::TOTAL_CICLOS; ++t) {
        // 5.1 Atualizar estação
        if (t > 0 && t % Config::TAMANHO_CICLO_SAZONAL == 0) {
            estacao_atual = (estacao_atual == Estacao::SECA) ? Estacao::CHEIA : Estacao::SECA;
            subgrid.atualizar_acessibilidade(estacao_atual);
            if (rank == 0) std::cout << "Mudanca de estacao no ciclo " << t << std::endl;
        }
        
        // 5.2 Troca de halo MPI
        trocar_halos_territorio(subgrid, local_width, mpi_celula, rank, size);
        
        // 5.3 Processar agentes com OpenMP
        std::vector<Agente> buffer_envio_cima;
        std::vector<Agente> buffer_envio_baixo;
        std::vector<Agente> nova_lista_local;

        processar_agentes(agentes_locais, subgrid, local_offsetX, local_offsetY, local_width, local_height, rank, size, nova_lista_local, buffer_envio_cima, buffer_envio_baixo);
        
        // 5.4 Migração de agentes com MPI
        migrar_agentes_entre_processos(rank, size, mpi_agente, agentes_locais, nova_lista_local, buffer_envio_cima, buffer_envio_baixo);

        // 5.5 Atualizar grid local via OpenMP paralelizável
        subgrid.atualizar_recursos(estacao_atual);
        
        // 5.6 Métricas globais
        int local_num_agentes = agentes_locais.size();
        int global_num_agentes = 0;
        MPI_Allreduce(&local_num_agentes, &global_num_agentes, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        
        float local_recursos = subgrid.get_recursos_totais();
        float global_recursos = 0;
        MPI_Reduce(&local_recursos, &global_recursos, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
        
        if (rank == 0) {
            std::cout << "Ciclo " << t << " - Agentes Globais: " << global_num_agentes 
                      << " - Recursos Totais: " << global_recursos << std::endl;
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

std::vector<Agente> inicializar_agentes_locais(int size, int rank, int local_width, int local_height, int local_offsetX, int local_offsetY) 
{
    int local_agents_count = Config::N_AGENTS / size;
    std::vector<Agente> agentes;
    
    // Otimização: reserva o espaço no vetor de uma vez para evitar múltiplas realocações
    agentes.reserve(local_agents_count); 
    
    for (int i = 0; i < local_agents_count; ++i) {
        // Gera coordenadas globais aleatórias dentro do subgrid local deste processo
        int gx = rand() % local_width + local_offsetX;
        int gy = rand() % local_height + local_offsetY;
        
        // Cria o agente com um ID único global (rank * contagem + i)
        agentes.push_back(Agente(rank * local_agents_count + i, Posicao(gx, gy), Config::ENERGIA_INICIAL_AGENTE));
    }
    
    return agentes;
}

// Função auxiliar para trocar halos entre processos
// Otimizada para ser não bloqueante
void trocar_halos_territorio(Territorio& subgrid, int local_width, MPI_Datatype mpi_celula, int rank, int size) {
    MPI_Request reqs[4];
    int num_reqs = 0;
    
    // Recebe do vizinho de cima e envia sua borda superior para ele
    if (rank > 0) {
        MPI_Irecv(subgrid.ptr_halo_sup(), local_width, mpi_celula, rank - 1, 0, MPI_COMM_WORLD, &reqs[num_reqs++]);
        MPI_Isend(subgrid.ptr_linha_sup(), local_width, mpi_celula, rank - 1, 1, MPI_COMM_WORLD, &reqs[num_reqs++]);
    }
    
    // Recebe do vizinho de baixo e envia sua borda inferior para ele
    if (rank < size - 1) {
        MPI_Irecv(subgrid.ptr_halo_inf(), local_width, mpi_celula, rank + 1, 1, MPI_COMM_WORLD, &reqs[num_reqs++]);
        MPI_Isend(subgrid.ptr_linha_inf(), local_width, mpi_celula, rank + 1, 0, MPI_COMM_WORLD, &reqs[num_reqs++]);
    }
    
    // Aguarda todas as comunicações não-bloqueantes terminarem antes de prosseguir
    if (num_reqs > 0) {
        MPI_Waitall(num_reqs, reqs, MPI_STATUSES_IGNORE);
    }
}

void processar_agentes(
    const std::vector<Agente>& agentes_locais,
    Territorio& subgrid,
    int local_offsetX, int local_offsetY,
    int local_width, int local_height,
    int rank, int size,
    std::vector<Agente>& nova_lista_local,
    std::vector<Agente>& buffer_envio_cima,
    std::vector<Agente>& buffer_envio_baixo) 
{
    // Limpa os buffers para a nova iteração
    nova_lista_local.clear();
    buffer_envio_cima.clear();
    buffer_envio_baixo.clear();

    #pragma omp parallel
    {
        // Vetores privados para cada thread (evita contenção no início)
        std::vector<Agente> envio_cima_thread;
        std::vector<Agente> envio_baixo_thread;
        std::vector<Agente> lista_local_thread;

        #pragma omp for
        for (int i = 0; i < (int)agentes_locais.size(); ++i) {
            Agente a_atualizado = agentes_locais[i];
            Posicao celula_atual = a_atualizado.get_posicao();
            int lnx = celula_atual.x - local_offsetX;
            int lny = celula_atual.y - local_offsetY;
            
            float r = 0;
            if(lny >= 0 && lny < local_height && lnx >= 0 && lnx < local_width) {
                 r = subgrid.get_celula(Posicao(lnx, lny)).recurso;
            }
            
            // 1. Executa carga de trabalho e CONSOME energia (Agora afeta o agente)
            a_atualizado.executar_carga(r);
            
            // 2. Verifica se o agente ainda está vivo
            if (a_atualizado.get_energia() <= 0) {
                continue; // O agente morreu e não será adicionado a nenhuma lista
            }

            // 3. Se vivo, decide o próximo passo
            Posicao destino;
            a_atualizado.decidir(subgrid, destino);
            a_atualizado.set_posicao(destino);
            
            // Lógica de Migração ou Permanência Local
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

        // Consolidação segura (Região Crítica)
        #pragma omp critical
        {
            buffer_envio_cima.insert(buffer_envio_cima.end(), envio_cima_thread.begin(), envio_cima_thread.end());
            buffer_envio_baixo.insert(buffer_envio_baixo.end(), envio_baixo_thread.begin(), envio_baixo_thread.end());
            nova_lista_local.insert(nova_lista_local.end(), lista_local_thread.begin(), lista_local_thread.end());
        }
    }
}

void migrar_agentes_entre_processos(
    int rank, int size, MPI_Datatype mpi_agente,
    std::vector<Agente>& agentes_locais,
    std::vector<Agente>& nova_lista_local,
    std::vector<Agente>& buffer_envio_cima,
    std::vector<Agente>& buffer_envio_baixo) 
{
    int recv_cima_size = 0, recv_baixo_size = 0;
    int send_cima_size = buffer_envio_cima.size();
    int send_baixo_size = buffer_envio_baixo.size();
    
    MPI_Request reqs_migr[8];
    int num_reqs_migr = 0;
    
    // FASE 1: Troca de metadados (quantos agentes cada um vai mandar?)
    if (rank > 0) {
        // Recebe do vizinho de cima e envia sua borda superior para ele
        MPI_Irecv(&recv_cima_size, 1, MPI_INT, rank - 1, 2, MPI_COMM_WORLD, &reqs_migr[num_reqs_migr++]);
        MPI_Isend(&send_cima_size, 1, MPI_INT, rank - 1, 3, MPI_COMM_WORLD, &reqs_migr[num_reqs_migr++]);
    }
    if (rank < size - 1) {
        // Recebe do vizinho de baixo e envia sua borda inferior para ele
        MPI_Irecv(&recv_baixo_size, 1, MPI_INT, rank + 1, 3, MPI_COMM_WORLD, &reqs_migr[num_reqs_migr++]);
        MPI_Isend(&send_baixo_size, 1, MPI_INT, rank + 1, 2, MPI_COMM_WORLD, &reqs_migr[num_reqs_migr++]);
    }
    if (num_reqs_migr > 0) {
        // Aguarda todas as comunicações não-bloqueantes terminarem antes de prosseguir
        MPI_Waitall(num_reqs_migr, reqs_migr, MPI_STATUSES_IGNORE);
    } 
    
    // FASE 2: Troca dos agentes reais
    std::vector<Agente> recv_buffer_cima(recv_cima_size);
    std::vector<Agente> recv_buffer_baixo(recv_baixo_size);
    num_reqs_migr = 0;
    
    if (rank > 0) {
        if (recv_cima_size > 0) {
            // Recebe do vizinho de cima e envia sua borda superior para ele
            MPI_Irecv(recv_buffer_cima.data(), recv_cima_size, mpi_agente, rank - 1, 4, MPI_COMM_WORLD, &reqs_migr[num_reqs_migr++]);
        }
        if (send_cima_size > 0) {
            // Recebe do vizinho de cima e envia sua borda superior para ele
            MPI_Isend(buffer_envio_cima.data(), send_cima_size, mpi_agente, rank - 1, 5, MPI_COMM_WORLD, &reqs_migr[num_reqs_migr++]);
        }
    }
    if (rank < size - 1) {
        if (recv_baixo_size > 0) {
            // Recebe do vizinho de baixo e envia sua borda inferior para ele
            MPI_Irecv(recv_buffer_baixo.data(), recv_baixo_size, mpi_agente, rank + 1, 5, MPI_COMM_WORLD, &reqs_migr[num_reqs_migr++]);
        }
        if (send_baixo_size > 0) {
            // Recebe do vizinho de baixo e envia sua borda inferior para ele
            MPI_Isend(buffer_envio_baixo.data(), send_baixo_size, mpi_agente, rank + 1, 4, MPI_COMM_WORLD, &reqs_migr[num_reqs_migr++]);
        }
    }
    if (num_reqs_migr > 0) MPI_Waitall(num_reqs_migr, reqs_migr, MPI_STATUSES_IGNORE);
    
    // FASE 3: Consolidação (Move os dados para a lista oficial)
    agentes_locais = std::move(nova_lista_local);
    if (!recv_buffer_cima.empty()) {
        // Recebe do vizinho de cima e envia sua borda superior para ele
        agentes_locais.insert(agentes_locais.end(), recv_buffer_cima.begin(), recv_buffer_cima.end());
    }
    if (!recv_buffer_baixo.empty()) {
        // Recebe do vizinho de baixo e envia sua borda inferior para ele
        agentes_locais.insert(agentes_locais.end(), recv_buffer_baixo.begin(), recv_buffer_baixo.end());
    }
}
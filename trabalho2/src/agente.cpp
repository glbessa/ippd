#include "agente.hpp"
#include <cmath>
#include <cstdlib>

#define MAX_CUSTO 10000
#define CUSTO_METABOLICO 2.0f
#define TAXA_CUSTO_ESFORCO 0.002f
#define EFICIENCIA_REABASTECIMENTO 0.4f

Agente::Agente(int id, Posicao inicial, float energia_inicial)
    : id(id), pos(inicial), energia(energia_inicial) {}

void Agente::executar_carga(float recurso_local) {
    // O custo é proporcional ao recurso local (quanto mais recurso, mais trabalho para processar/decidir)
    int custo = static_cast<int>(recurso_local * 100.0f);
    if (custo > MAX_CUSTO) {
        custo = MAX_CUSTO;
    }

    // Processamento computacional arbitrário (útil para analisar OpenMP overhead)
    volatile double dummy = 0.0;
    for (int i = 0; i < custo; ++i) {
        dummy += std::sin(static_cast<double>(i)) * std::cos(static_cast<double>(i));
    }

    // Gasto de energia: 
    // 1. Custo metabólico fixo - Aumentado para maior rigor
    // 2. Gasto proporcional ao esforço da carga sintética - Peso aumentado
    float custo_esforco = custo * TAXA_CUSTO_ESFORCO;
    
    this->energia -= (CUSTO_METABOLICO + custo_esforco);
}

void Agente::decidir(const Territorio& grid_local, Posicao& dest) const {
    // Algoritmo local de decisão:
    // Agente verifica células vizinhas acessíveis e com maior recurso disponível.
    // O destino padrão inicialmente é a própria posição.
    dest = pos;

    // Converte sua posição global atual para o referencial local da grade
    int local_x = pos.x - grid_local.get_offset().x;
    int local_y = pos.y - grid_local.get_offset().y;

    // Uma heurística inicial simplista visando a máxima quantidade de recursos (vizinhança Moore)
    int dx[] = {-1,  0,  1, -1, 1, -1, 0, 1};
    int dy[] = {-1, -1, -1,  0, 0,  1, 1, 1};
    
    // Identificação do melhor recurso atual (célula onde está no momento)
    float melhor_recurso = -1.0f;
    if (local_x >= 0 && local_x < grid_local.get_largura() && 
        local_y >= 0 && local_y < grid_local.get_altura()) {
        melhor_recurso = grid_local.get_celula(Posicao(local_x, local_y)).recurso;
    }

    for (int i = 0; i < 8; ++i) {
        int nx = pos.x + dx[i]; // Global adjacente X
        int ny = pos.y + dy[i]; // Global adjacente Y
        
        // Relacionado ao grid e ao offset
        int lnx = nx - grid_local.get_offset().x;
        int lny = ny - grid_local.get_offset().y;

        bool is_valid = false;
        Celula vizinha;

        if (lnx >= 0 && lnx < grid_local.get_largura()) {
            if (lny >= 0 && lny < grid_local.get_altura()) {
                vizinha = grid_local.get_celula(Posicao(lnx, lny));
                is_valid = true;
            } else if (lny == -1 && grid_local.tem_halo_sup()) {
                vizinha = grid_local.get_halo_sup(lnx);
                is_valid = true;
            } else if (lny == grid_local.get_altura() && grid_local.tem_halo_inf()) {
                vizinha = grid_local.get_halo_inf(lnx);
                is_valid = true;
            }
        }

        if (is_valid && vizinha.acessivel && vizinha.recurso > melhor_recurso) {
            melhor_recurso = vizinha.recurso;
            dest.x = nx;
            dest.y = ny;
        }
    }
}

float Agente::consumir_recurso(Territorio& grid_local) {
    // Quantidade fixa que um grupo indígena retira em um ciclo
    float recurso_requerido = 5.0f;
    
    int local_x = pos.x - grid_local.get_offset().x;
    int local_y = pos.y - grid_local.get_offset().y;

    // Garante que tentará consumir dentro do grid que reside
    if (local_x >= 0 && local_x < grid_local.get_largura() &&
        local_y >= 0 && local_y < grid_local.get_altura()) {

        // Consome limitando à quantidade total que a célula possui no momento
        float recurso_disponivel = grid_local.get_celula(Posicao(local_x, local_y)).recurso;
        float consumo_real = (recurso_disponivel >= recurso_requerido) ? recurso_requerido : recurso_disponivel;

        // Avisa à grade local que aquele conteúdo foi removido.
        // O `registrar_consumo` deverá tratar a atomicidade do OpenMP
        grid_local.registrar_consumo(Posicao(local_x, local_y), consumo_real);
        
        // Reabastece as energias do Agente - Eficiência reduzida
        this->energia += consumo_real * 0.4f;
        
        return consumo_real;
    }

    return 0.0f; // Se fora do grid não tem acesso ao recurso para este processo
}

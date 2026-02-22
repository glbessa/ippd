#include "territorio.hpp"
#include "config.hpp"
#include <omp.h>
#include <stdexcept>
#include <cmath>

Territorio::Territorio(int w, int h, Posicao offset_inicial)
    : largura(w), altura(h), offset(offset_inicial) {
    
    // Aloca continuamente na memória - melhor para cache misses (L1, L2)
    // E permite buffer contíguo ao passar para o MPI
    grid.resize(largura * altura);
}

// Funções determinísticas espaciais
// Implementação exata depende da formulação do problema.
TipoCelula Territorio::f_tipo(Posicao global) const {
    // Exemplo de mapeamento determinístico. Intercale ou utilize uma função hash sobre gx/gy.
    if ((global.x % Config::MODULO_ALDEIA == 0) && (global.y % Config::MODULO_ALDEIA == 0)) return TipoCelula::ALDEIA;
    if (global.x % Config::MODULO_PESCA == 0) return TipoCelula::PESCA; // rios passam nesses eixos
    if (global.y % Config::MODULO_ROCADO == 0) return TipoCelula::ROCADO;
    return TipoCelula::COLETA;
}

float Territorio::f_recurso(TipoCelula tipo) const {
    switch (tipo) {
        case TipoCelula::ALDEIA: return Config::RECURSO_MAX_ALDEIA;
        case TipoCelula::PESCA: return Config::RECURSO_MAX_PESCA;
        case TipoCelula::ROCADO: return Config::RECURSO_MAX_ROCADO;
        case TipoCelula::COLETA: return Config::RECURSO_MAX_COLETA;
        case TipoCelula::INTERDITA: return 0.0f;
        default: return 0.0f;
    }
}

bool Territorio::f_acesso(TipoCelula tipo, Estacao estacao) const {
    // Exemplo: na estação cheia áreas de pesca podem expandir, ou áreas interditadas mudarem
    if (tipo == TipoCelula::INTERDITA) return false;
    if (estacao == Estacao::CHEIA && tipo == TipoCelula::COLETA) return false; // Inundou a coleta
    return true;
}

float Territorio::f_regeneracao(Estacao estacao) const {
    // Retorna a taxa de regeneração baseada na estação
    return estacao == Estacao::CHEIA ? Config::TAXA_REGENERACAO_CHEIA : Config::TAXA_REGENERACAO_SECA;
}

void Territorio::inicializar(Estacao estacao_inicial) {
    // Utilização de OpenMP para inicialização distribuída no multicore (first-touch policy p/ NUMA)
    // collapse(2) para "juntar" os dois loops
    // schedule(static) para eficiência na distribuição de trabalho
    #pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < altura; ++y) {
        for (int x = 0; x < largura; ++x) {
            Posicao global(offset.x + x, offset.y + y);
            
            TipoCelula tipo = f_tipo(global);
            float recurso = f_recurso(tipo);
            bool acessivel = f_acesso(tipo, estacao_inicial);

            int index = y * largura + x;
            grid[index].tipo = tipo;
            grid[index].recurso = recurso;
            grid[index].consumo_acumulado_na_celula = 0.0f;
            grid[index].acessivel = acessivel;
        }
    }
}

void Territorio::atualizar_acessibilidade(Estacao nova_estacao) {
    // Apenas recalcula o status de acesso nas células, de acordo com a nova estação
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < (int)grid.size(); ++i) {
        grid[i].acessivel = f_acesso(grid[i].tipo, nova_estacao);
    }
}

void Territorio::atualizar_recursos(Estacao estacao_atual) {
    float regeneracao_base = f_regeneracao(estacao_atual);
    
    // Atualização de dados da área local. Essa rotina é computacionalmente intensiva?
    // Operações em array contíguo: ótimo uso de prefetching!
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < (int)grid.size(); ++i) {
        // Recurso += regeneracao - consumo_acumulado (limitado ao máx possível de recursos)
        float maximo_capacidade = f_recurso(grid[i].tipo);
        float novo_recurso = grid[i].recurso + regeneracao_base - grid[i].consumo_acumulado_na_celula;
        
        // Clamping manual
        if (novo_recurso > maximo_capacidade) novo_recurso = maximo_capacidade;
        if (novo_recurso < 0.0f) novo_recurso = 0.0f;
        
        grid[i].recurso = novo_recurso;
        
        // Zera o consumo para o próximo ciclo
        grid[i].consumo_acumulado_na_celula = 0.0f;
    }
}

void Territorio::registrar_consumo(Posicao local, float quantidade) {
    int index = local.y * largura + local.x;
    
    // Como os agentes são processados em paralelo (via threads OpenMP),
    // vários agentes podem tentar consumir na MESMA célula simultaneamente!
    // A soma deve ser atômica.
    #pragma omp atomic
    grid[index].consumo_acumulado_na_celula += quantidade;
}

float Territorio::get_recursos_totais() const {
    float total = 0.0f;
    #pragma omp parallel for reduction(+:total)
    for (int i = 0; i < (int)grid.size(); ++i) {
        total += grid[i].recurso;
    }
    return total;
}

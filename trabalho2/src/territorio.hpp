#ifndef TERRITORIO_HPP
#define TERRITORIO_HPP

#include <vector>
#include "posicao.hpp"

// Representa os tipos de células possíveis no território
enum class TipoCelula {
    ALDEIA,
    PESCA,
    COLETA,
    ROCADO,
    INTERDITA
};

// Representa a sazonalidade no sistema
enum class Estacao {
    SECA,
    CHEIA
};

// Estrutura principal da célula
// Utilizando atributos básicos para garantir localidade de cache e permitir que seja fácil
// realizar a comunicação via MPI futuramente.
struct Celula {
    TipoCelula tipo;
    float recurso;
    float consumo_acumulado_na_celula; // Útil para abater do recurso final na fase da atualização
    bool acessivel;
};

class Territorio {
private:
    int largura;
    int altura;
    Posicao offset; // Posição global de início do subgrid
    
    // Matriz 1D contínua é muito mais eficiente computacionalmente para OpenMP (cache friendly) e MPI (facilita envio de blocos/halos)
    std::vector<Celula> grid;
    std::vector<Celula> halo_sup;
    std::vector<Celula> halo_inf;

    // Funções auxiliares (de acordo com as regras de negócio abstratas)
    TipoCelula f_tipo(Posicao global) const;
    float f_recurso(TipoCelula tipo) const;
    bool f_acesso(TipoCelula tipo, Estacao estacao) const;
    float f_regeneracao(Estacao estacao) const;

public:
    // Construtor: Inicializa a grade baseada na divisão espacial
    Territorio(int w, int h, Posicao offset_inicial);

    // Inicializa os atributos da célula baseado na posição global
    void inicializar(Estacao estacao_inicial);

    // Métricas principais para OpenMP parallel for
    void atualizar_acessibilidade(Estacao nova_estacao);
    void atualizar_recursos(Estacao estacao_atual);

    // Consumo de recurso por um agente localmente (precisa ser atômico dependendo do de como os agentes operam)
    void registrar_consumo(Posicao local, float quantidade);

    // Acessos à célula usando mapeamento de 2D para 1D
    inline Celula& get_celula(Posicao local) {
        return grid[local.y * largura + local.x];
    }
    
    inline const Celula& get_celula(Posicao local) const {
        return grid[local.y * largura + local.x];
    }

    void alocar_halos(bool tem_sup, bool tem_inf) {
        if (tem_sup) halo_sup.resize(largura);
        if (tem_inf) halo_inf.resize(largura);
    }

    bool tem_halo_sup() const { return !halo_sup.empty(); }
    bool tem_halo_inf() const { return !halo_inf.empty(); }

    const Celula& get_halo_sup(int x) const { return halo_sup[x]; }
    const Celula& get_halo_inf(int x) const { return halo_inf[x]; }

    Celula* ptr_linha_sup() { return grid.data(); }
    Celula* ptr_linha_inf() { return grid.data() + (altura - 1) * largura; }
    Celula* ptr_halo_sup() { return halo_sup.data(); }
    Celula* ptr_halo_inf() { return halo_inf.data(); }

    // Getters úteis
    int get_largura() const { return largura; }
    int get_altura() const { return altura; }
    Posicao get_offset() const { return offset; }
    int get_tamanho_total() const { return largura * altura; }
    
    // Retorna ponteiro bruto se precisar para operações de MPI
    Celula* data() { return grid.data(); }
    const Celula* data() const { return grid.data(); }
};

#endif // TERRITORIO_HPP

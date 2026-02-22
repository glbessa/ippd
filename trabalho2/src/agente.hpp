#ifndef AGENTE_HPP
#define AGENTE_HPP

#include "territorio.hpp"
#include "posicao.hpp"

// Classe para abstrair os agentes no sistema (grupos familiares indígenas)
class Agente {
private:
    Posicao pos; // Posição global
    float energia; // Estado interno do agente (necessidade/energia)

public:
    // Construtor do agente
    Agente() : energia(0.0f) {}
    Agente(Posicao inicial, float energia_inicial);

    // Getters para os atributos
    Posicao get_posicao() const { return pos; }
    float get_energia() const { return energia; }

    // Atualiza a posição do agente
    void set_posicao(Posicao nova) { pos = nova; }

    // Atualiza a energia do agente (usado na reprodução para ceder energia ao filho)
    void set_energia(float e) { energia = e; }

    // Tenta reproduzir o agente caso sua energia supere THRESHOLD_REPRODUCAO.
    // O filho nasce na célula adjacente acessível com mais recurso.
    // O pai transfere FATOR_ENERGIA_REPRODUCAO * sua_energia ao filho e perde esse valor.
    // Retorna true e preenche `filho` se a reprodução ocorreu, false caso contrário.
    bool reproduzir(const Territorio& grid_local, Agente& filho);

    // Executa uma carga computacional sintética proporcional ao recurso disponível
    // Essa função servirá para stress test do OpenMP e reduzirá a energia do agente
    void executar_carga(float recurso_local);

    // Regras locais de decisão para o deslocamento do agente.
    // Avalia o territorio e decide qual posição (dest_x, dest_y) o agente deseja ir
    void decidir(const Territorio& grid_local, Posicao& dest) const;

    // Tenta consumir recursos no grid local e converte em energia.
    // Retorna o total de recursos consumidos na iteração 
    float consumir_recurso(Territorio& grid_local);
};

#endif // AGENTE_HPP

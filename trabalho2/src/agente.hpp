#ifndef AGENTE_HPP
#define AGENTE_HPP

#include "territorio.hpp"
#include "posicao.hpp"

// Classe para abstrair os agentes no sistema (grupos familiares indígenas)
class Agente {
private:
    int id;
    Posicao pos; // Posição global
    float energia; // Estado interno do agente (necessidade/energia)

public:
    // Construtor do agente
    Agente(int id, Posicao inicial, float energia_inicial);

    // Getters para os atributos
    int get_id() const { return id; }
    Posicao get_posicao() const { return pos; }
    float get_energia() const { return energia; }

    // Atualiza a posição do agente
    void set_posicao(Posicao nova) { pos = nova; }

    // Executa uma carga computacional sintética proporcional ao recurso disponível
    // Essa função servirá para stress test do OpenMP
    void executar_carga(float recurso_local) const;

    // Regras locais de decisão para o deslocamento do agente.
    // Avalia o territorio e decide qual posição (dest_x, dest_y) o agente deseja ir
    void decidir(const Territorio& grid_local, Posicao& dest) const;

    // Tenta consumir recursos no grid local e converte em energia.
    // Retorna o total de recursos consumidos na iteração 
    float consumir_recurso(Territorio& grid_local);
};

#endif // AGENTE_HPP

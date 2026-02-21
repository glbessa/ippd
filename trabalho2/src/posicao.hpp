#ifndef POSICAO_HPP
#define POSICAO_HPP

// Estrutura para abstrair uma posição 2D no grid (ou subgrid)
struct Posicao {
    int x;
    int y;

    // Construtor padrão e paramétrico
    Posicao(int x = 0, int y = 0) : x(x), y(y) {}

    // Sobrecarga de operadores básicos de comparação que são úteis
    bool operator==(const Posicao& outra) const {
        return x == outra.x && y == outra.y;
    }

    bool operator!=(const Posicao& outra) const {
        return !(*this == outra);
    }
};

#endif // POSICAO_HPP

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Definição da estrutura de um nó da árvore binária
typedef struct Node {
    int data;             // Valor armazenado no nó
    struct Node *left;    // Ponteiro para o filho esquerdo
    struct Node *right;   // Ponteiro para o filho direito
} Node;

// Função para criar um novo nó na árvore
Node* createNode(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node)); // Aloca memória para o novo nó
    if (newNode == NULL) {
        printf("Erro ao alocar memória!\n");
        exit(1);
    }
    newNode->data = data;  // Atribui o valor ao nó
    newNode->left = NULL;  // Inicializa o filho esquerdo como NULL
    newNode->right = NULL; // Inicializa o filho direito como NULL
    return newNode;
}

// Função recursiva para encontrar e imprimir os nós com valores maiores que um dado valor
// Retorna o número de nós maiores que "value" a partir do nó corrente
int findNodes(Node* root, int value) {
    int r = 0; // contadores: local, nó da esquerda, nó da direita
    if (root == NULL) {
        return 0; // Se o nó for NULL, retorna sem fazer nada
    }

    // Se o valor do nó atual for maior que o valor fornecido, imprime o nó
    if (root->data > value) {
	    r = 1;
        printf("Nó encontrado: %d\n", root->data);
    }

    // Chamada recursiva para percorrer os filhos esquerdo e direito
    #pragma omp task shared(r)
    {
        int rp = findNodes(root->left, value);
        #pragma omp atomic
        r += rp;
    }

    #pragma omp task shared(r)
    {
        int rp = findNodes(root->right, value);
        #pragma omp atomic
        r += rp;
    }

    return r;
}

int main() {
    int r;
    // Criação manual da árvore binária de busca
    // Estrutura da árvore:
    //        10
    //       /  \
    //      5    15
    //     / \   /  \
    //    3   7 12  18
    Node* root = createNode(10);
    root->left = createNode(5);
    root->right = createNode(15);
    root->left->left = createNode(3);
    root->left->right = createNode(7);
    root->right->left = createNode(12);
    root->right->right = createNode(18);

    // Definição do valor de referência para busca
    int value = 8;
    
    // Chamada da função de busca
    r = findNodes(root, value);
    printf("Existem %d nós maiores que %d\n", r, value);

    return 0; // Indica que o programa foi executado com sucesso
}


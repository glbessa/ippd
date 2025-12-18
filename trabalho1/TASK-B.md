A Tarefa B, intitulada **Seção crítica vs atomic e agregação por thread**, foca na implementação e comparação de diferentes métodos para gerenciar a concorrência na construção de histogramas.

O **kernel** central a ser implementado envolve o cálculo de histogramas, onde a entrada $A$ está no intervalo $[0, B)$ e a saída é o vetor $H[B]$.

A implementação do código deve cobrir três variantes principais:

### V1: Uso de Seção Crítica

Esta variante implementa a atualização do histograma utilizando uma seção crítica OpenMP para garantir a exclusão mútua:

*   O código deve atualizar o contador do histograma para o elemento $A[i]$ dentro de uma região crítica:
    ```c
    // Atualizar H[A[i]]++
    #pragma omp critical
    {
        H[A[i]]++; 
    }
    ```

### V2: Substituição por Operação Atômica

Nesta variante, a seção crítica deve ser substituída pela diretiva atômica, sempre que for válido. A diretiva `#pragma omp atomic` é aplicável para atualizações atômicas sobre um único escalar endereçável e quando não há efeitos colaterais na expressão de atualização.

*   O código deve utilizar a operação atômica para o incremento:
    ```c
    #pragma omp atomic
    H[A[i]]++;
    ```

### V3: Agregação Local por Thread

Esta variante visa reduzir a contenção, que é alta nas V1 e V2. O método exige o uso de **histogramas locais (privados) por thread**, seguidos por uma **redução manual** (soma dos histogramas locais no histograma global).

### Objetivo e Medições

O objetivo da Tarefa B é **comparar os tempos de execução e a escalabilidade** das três variantes. Os parâmetros de teste definidos para o tamanho do *bucket* do histograma são $B \in \{32, 256, 4096\}$.

É fundamental lembrar os critérios para escolher *atomic* sobre *critical*: a atualização deve ser sobre **um único escalar endereçável** e **sem efeitos colaterais**. Caso a substituição não seja possível, deve-se justificar o uso de `critical` ou a agregação por thread.
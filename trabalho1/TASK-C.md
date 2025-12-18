A Tarefa C, focada em **Vetorização com simd**, exige a implementação do kernel SAXPY em três variantes distintas para análise.

O **kernel** a ser implementado é a operação **SAXPY**, definida como:
$$y[i] = a \cdot x[i] + y[i]$$.

As **três variantes** de implementação que devem ser comparadas são:

1.  **V1 (Sequencial):** A versão base, implementada de forma sequencial.
2.  **V2 (Simd Puro):** A implementação que utiliza a diretiva **`#pragma omp simd`**. O objetivo desta diretiva é ativar a vetorização do laço pelo compilador, quando possível.
3.  **V3 (Paralelismo e Simd):** A implementação que combina paralelismo e vetorização usando a diretiva **`#pragma omp parallel for simd`**.

Após a implementação, a tarefa exige a **análise dos ganhos e limitações** do uso de SIMD. As medições obrigatórias incluem reportar o **Ganho de simd sobre a versão base**. A análise deve ser feita com base em dados, e não em suposições.
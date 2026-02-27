# Simulação Distribuída e Paralela de Mobilidade Territorial Sazonal (MPI + OpenMP)

Implementação de uma simulação **multiagente** em um território 2D (grid) com **sazonalidade** (seca/cheia), explorando paralelismo **híbrido**:

- **MPI** para distribuir o território entre processos (decomposição espacial)
- **OpenMP** para paralelizar, dentro de cada processo, o processamento massivo de agentes e a atualização do subgrid

Este projeto foi desenvolvido para a disciplina **Introdução ao Processamento Paralelo e Distribuído** (2025/2).

---

## Visão geral do modelo

### Território
O território global é uma grade 2D de células. Cada célula possui:

- **Tipo**: aldeia, pesca, coleta, roçado (e suporte a área interditada)
- **Recurso**: valor escalar finito que pode ser consumido por agentes
- **Acessibilidade**: depende da estação (seca/cheia)

O tipo de terreno e o recurso máximo por célula são definidos de forma **determinística** a partir da posição global, visando **reprodutibilidade**.

### Agentes
Cada agente representa um grupo familiar e tem:

- **Posição global** (no grid)
- **Energia** (estado interno)

Por ciclo, cada agente:

1. Executa uma **carga computacional sintética** proporcional ao recurso local (controla o custo computacional e estressa OpenMP)
2. Perde energia por um custo metabólico + custo proporcional ao esforço
3. Decide deslocamento para a vizinhança (Moore) buscando células acessíveis com mais recurso
4. Se permanecer no subgrid local, **consome recurso** da célula e converte parte disso em energia
5. Pode **morrer** (energia <= 0) ou **reproduzir** (energia acima de um limiar)

### Tempo e sazonalidade
A simulação evolui em ciclos discretos. A cada `TAMANHO_CICLO_SAZONAL` ciclos, alterna entre:

- **SECA**
- **CHEIA**

Essa troca afeta a **acessibilidade** e a **regeneração** de recursos no território.

---

## Paralelismo e distribuição

### MPI (distribuição espacial)
O grid global é particionado por **faixas horizontais** (decomposição 1D no eixo Y):

- Cada rank MPI recebe um subgrid com `local_height = ALTURA_GRID / size` e largura total
- A simulação realiza **troca de halos** (linha acima/abaixo) entre ranks vizinhos a cada ciclo
- Agentes que cruzam a fronteira superior/inferior são **migrados** entre ranks via MPI (topologia linear)

**Observação importante:** a implementação assume que `ALTURA_GRID` é múltiplo do número de processos MPI (`size`), pois o particionamento usa divisão inteira.

### OpenMP (paralelismo intra-processo)
Dentro de cada rank:

- O processamento dos agentes é feito com `#pragma omp parallel` + `#pragma omp for`
- A atualização das células do território é paralelizada por varredura do vetor contíguo
- O consumo acumulado por célula usa `#pragma omp atomic` para evitar condições de corrida

---

## Saída e métricas

Em cada ciclo (rank 0), o programa imprime um painel com:

- ciclo e estação (seca/cheia)
- número total de agentes e **energia média**
- distribuição de agentes por processo (mínimo/máximo)
- nascimentos e mortes no ciclo
- migração no ciclo e migração acumulada
- recursos totais e recursos médios por célula
- indicação de “sustentabilidade” (regeneração >= consumo)

---

## Estrutura do projeto

- [src/main.cpp](src/main.cpp): laço principal, troca de halos, migração de agentes e coleta de métricas
- [src/territorio.hpp](src/territorio.hpp) / [src/territorio.cpp](src/territorio.cpp): grid local, halos, acesso/regeneração e consumo atômico
- [src/agente.hpp](src/agente.hpp) / [src/agente.cpp](src/agente.cpp): regras do agente (decisão, carga sintética, consumo, reprodução)
- [src/config.hpp](src/config.hpp): parâmetros da simulação (tamanho do grid, nº de agentes, taxas, limites)
- [bin/trabalho2](bin/trabalho2): binário (se já estiver compilado no ambiente)

---

## Como compilar

Requisitos:

- Compilador C++ com suporte a **C++17**
- Implementação MPI (ex.: OpenMPI ou MPICH)
- Suporte a OpenMP

Com `mpic++` (recomendado; em algumas distros o wrapper pode se chamar `mpicxx` ou `mpiCC`):

```bash
cd ippd/trabalho2
mkdir -p bin
mpic++ -O3 -std=c++17 -fopenmp src/*.cpp -o bin/trabalho2
```

---

## Como executar

Defina o número de threads OpenMP por processo e execute com MPI:

```bash
cd ippd/trabalho2
export OMP_NUM_THREADS=4
mpirun -np 4 ./bin/trabalho2
```

Se o seu MPI não expõe `mpirun`, use `mpiexec`.

Notas:

- Ajuste `-np` e `OMP_NUM_THREADS` conforme sua máquina.
- A execução é **determinística** para um mesmo número de processos MPI e os mesmos parâmetros em [src/config.hpp](src/config.hpp).

---

## Parâmetros (configuração)

Os principais parâmetros podem ser alterados em [src/config.hpp](src/config.hpp), por exemplo:

- dimensões do grid (`LARGURA_GRID`, `ALTURA_GRID`)
- número total de agentes (`N_AGENTS`)
- ciclos e tamanho do ciclo sazonal (`TOTAL_CICLOS`, `TAMANHO_CICLO_SAZONAL`)
- carga sintética (`FATOR_CARGA_TRABALHO`, `MAX_CUSTO`) e custo energético (`CUSTO_METABOLICO`, `TAXA_CUSTO_ESFORCO`)
- consumo/regeneração de recursos



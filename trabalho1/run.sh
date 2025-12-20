#!/usr/bin/env bash

set -euo pipefail

# Script de experimentos para a disciplina de Programação Paralela
#
# Função:
#   - varrer a matriz de experimentos descrita em INSTRUCTIONS.md
#   - executar cada ponto 5 vezes
#   - calcular média e desvio-padrão dos tempos
#   - gravar tudo em um único CSV para uso pelo plot.py
#
# CONVENÇÕES ESPERADAS DOS PROGRAMAS C/C++ (ajuste se necessário):
#   - Haverá um executável para cada tarefa, todos em bin/:
#       Tarefa A (laço irregular):   ./bin/task_a_omp
#       Tarefa B (histogramas):      ./bin/task_b_omp
#       Tarefa C (SAXPY):            ./bin/task_c_omp
#       Tarefa D (região paralela):  ./bin/task_d_omp
#   - Os programas devem ler o número de threads via variável de ambiente
#       OMP_NUM_THREADS
#   - Formato de linha de comando (sugestão):
#       Tarefa A: task_a_omp N K schedule chunk
#           schedule ∈ {static, dynamic, guided}
#           chunk    ∈ {0, 1, 4, 16, 64} (0 pode ser ignorado para static)
#       Tarefa B: task_b_omp N B variant
#           variant ∈ {critical, atomic, local}
#       Tarefa C: task_c_omp N K variant
#           variant ∈ {seq, simd, parallel_simd}
#       Tarefa D: task_d_omp N variant
#           variant ∈ {two_parallel_for, single_parallel}
#   - Cada execução deve imprimir pelo menos duas linhas no stdout:
#       TOTAL_SECONDS=<tempo_total_em_segundos>
#       KERNEL_SECONDS=<tempo_kernel_em_segundos>
#     (em ponto flutuante; o script extrai esses valores por awk)

REPS=5   # número de repetições por ponto da matriz

Ns=(100000 500000 1000000)
Ks=(20 24 28)           # usado em Tarefas A e C
Bs=(32 256 4096)        # usado na Tarefa B
THREADS=(1 2 4 8)

OUTPUT_DIR="results"
mkdir -p "${OUTPUT_DIR}"
CSV_FILE="${OUTPUT_DIR}/results.csv"

# Cabeçalho do CSV (só escreve se o arquivo ainda não existir ou estiver vazio)
if [[ ! -f "${CSV_FILE}" || ! -s "${CSV_FILE}" ]]; then
    echo "task,variant,N,param_name,param_value,threads,mean_total_s,std_total_s,mean_kernel_s,std_kernel_s" >"${CSV_FILE}"
fi

###############################################################################
# Funções auxiliares: média e desvio-padrão
###############################################################################

compute_mean() {
    local values=("$@")
    local n=${#values[@]}

    if (( n == 0 )); then
        echo "0.0"
        return
    fi

    local mean
    mean=$(for v in "${values[@]}"; do echo "${v}"; done | awk '{s += $1} END {if (NR > 0) printf "%.9f", s/NR; else print "0.0"}')
    echo "${mean}"
}

compute_std() {
    local mean="$1"; shift
    local values=("$@")
    local n=${#values[@]}

    if (( n <= 1 )); then
        echo "0.0"
        return
    fi

    local std
    std=$(for v in "${values[@]}"; do echo "${v}"; done | awk -v m="${mean}" -v n="${n}" '{d = $1 - m; sq += d * d} END {if (n > 1) printf "%.9f", sqrt(sq/(n-1)); else print "0.0"}')
    echo "${std}"
}

###############################################################################
# Função genérica: roda um ponto da matriz e grava no CSV
###############################################################################

run_experiment_point() {
    local task="$1"       # A, B, C ou D
    local variant="$2"    # nome da variante (static, dynamic_1, ...)
    local N="$3"          # tamanho do problema
    local param_name="$4" # "K", "B" ou "-"
    local param_value="$5"
    local threads="$6"
    shift 6

    # Comando da aplicação (binário + argumentos restantes)
    local cmd=("$@")

    local total_values=()
    local kernel_values=()

    for ((rep = 1; rep <= REPS; rep++)); do
        export OMP_NUM_THREADS="${threads}"

        local output
        if ! output="$(${cmd[@]})"; then
            echo "[ERRO] Execução falhou: task=${task} variant=${variant} N=${N} ${param_name}=${param_value} threads=${threads}" >&2
            exit 1
        fi

        local total kernel
        total=$(echo "${output}" | awk -F= '/^TOTAL_SECONDS=/ {print $2}' | head -n1)
        kernel=$(echo "${output}" | awk -F= '/^KERNEL_SECONDS=/ {print $2}' | head -n1)

        if [[ -z "${total}" || -z "${kernel}" ]]; then
            echo "[ERRO] Não consegui extrair TOTAL_SECONDS/KERNEL_SECONDS do output da tarefa ${task}, variante ${variant}." >&2
            echo "Saída completa:" >&2
            echo "${output}" >&2
            exit 1
        fi

        total_values+=("${total}")
        kernel_values+=("${kernel}")
    done

    local mean_total std_total mean_kernel std_kernel
    mean_total=$(compute_mean "${total_values[@]}")
    std_total=$(compute_std "${mean_total}" "${total_values[@]}")
    mean_kernel=$(compute_mean "${kernel_values[@]}")
    std_kernel=$(compute_std "${mean_kernel}" "${kernel_values[@]}")

    printf "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n" \
        "${task}" "${variant}" "${N}" "${param_name}" "${param_value}" "${threads}" \
        "${mean_total}" "${std_total}" "${mean_kernel}" "${std_kernel}" \
        >>"${CSV_FILE}"
}

###############################################################################
# Tarefa A — Controle de reexecução (retomar de onde parou)
###############################################################################

declare -A DONE_POINTS_A=()

load_done_points_A() {
    # Lê o CSV existente (se houver) e marca quais pontos da Tarefa A
    # já foram medidos, para evitar repetir experimentos.
    if [[ ! -f "${CSV_FILE}" ]]; then
        return
    fi

    # Lê o arquivo linha a linha em shell atual (sem subshell),
    # ignorando o cabeçalho manualmente.
    local first=1
    while IFS=, read -r task variant N param_name param_value threads _; do
        if (( first )); then
            first=0
            continue
        fi
        [[ "${task}" != "A" ]] && continue
        local key="${variant}|${N}|${param_value}|${threads}"
        DONE_POINTS_A["${key}"]=1
    done < "${CSV_FILE}"
}


###############################################################################
# Tarefa A — Laço irregular e políticas de schedule
###############################################################################

run_task_A() {
    # Carrega pontos já presentes no CSV para evitar repetições
    load_done_points_A
    local bin_static="./bin/task_a_omp_static"
    local bin_dynamic="./bin/task_a_omp_dynamic"
    local bin_guided="./bin/task_a_omp_guided"

    if [[ ! -x "${bin_static}" || ! -x "${bin_dynamic}" || ! -x "${bin_guided}" ]]; then
        echo "[AVISO] Binários da Tarefa A (static/dynamic/guided) não encontrados; pulando Tarefa A." >&2
        return
    fi

    local chunks=(1 4 16 64)

    for N in "${Ns[@]}"; do
        for K in "${Ks[@]}"; do
            for threads in "${THREADS[@]}"; do
                # Variante 1: schedule(static)
                variant="static"
                key="${variant}|${N}|${K}|${threads}"
                if [[ ${DONE_POINTS_A["${key}"]+_} ]]; then
                    echo "[INFO] Pulando Tarefa A ${variant} N=${N} K=${K} threads=${threads} (já no CSV)" >&2
                else
                    run_experiment_point "A" "${variant}" "${N}" "K" "${K}" "${threads}" \
                        "${bin_static}" "${N}" "${K}" "static" 0
                fi

                # Variante 2: schedule(dynamic,chunk)
                for chunk in "${chunks[@]}"; do
                    variant="dynamic_${chunk}"
                    key="${variant}|${N}|${K}|${threads}"
                    if [[ ${DONE_POINTS_A["${key}"]+_} ]]; then
                        echo "[INFO] Pulando Tarefa A ${variant} N=${N} K=${K} threads=${threads} (já no CSV)" >&2
                        continue
                    fi
                    run_experiment_point "A" "${variant}" "${N}" "K" "${K}" "${threads}" \
                        "${bin_dynamic}" "${N}" "${K}" "dynamic" "${chunk}"
                done

                # Variante 3: schedule(guided,chunk)
                for chunk in "${chunks[@]}"; do
                    variant="guided_${chunk}"
                    key="${variant}|${N}|${K}|${threads}"
                    if [[ ${DONE_POINTS_A["${key}"]+_} ]]; then
                        echo "[INFO] Pulando Tarefa A ${variant} N=${N} K=${K} threads=${threads} (já no CSV)" >&2
                        continue
                    fi
                    run_experiment_point "A" "${variant}" "${N}" "K" "${K}" "${threads}" \
                        "${bin_guided}" "${N}" "${K}" "guided" "${chunk}"
                done
            done
        done
    done
}

###############################################################################
# Tarefa B — Histogramas: critical vs atomic vs local
###############################################################################

run_task_B() {
    local bin="./bin/task_b_omp"

    if [[ ! -x "${bin}" ]]; then
        echo "[AVISO] Binário ${bin} não encontrado; pulando Tarefa B." >&2
        return
    fi

    for N in "${Ns[@]}"; do
        for B in "${Bs[@]}"; do
            for threads in "${THREADS[@]}"; do
                run_experiment_point "B" "critical" "${N}" "B" "${B}" "${threads}" \
                    "${bin}" "${N}" "${B}" "critical"

                run_experiment_point "B" "atomic" "${N}" "B" "${B}" "${threads}" \
                    "${bin}" "${N}" "${B}" "atomic"

                run_experiment_point "B" "local" "${N}" "B" "${B}" "${threads}" \
                    "${bin}" "${N}" "${B}" "local"
            done
        done
    done
}

###############################################################################
# Tarefa C — SAXPY: seq, simd, parallel for simd
###############################################################################

run_task_C() {
    local bin="./bin/task_c_omp"

    if [[ ! -x "${bin}" ]]; then
        echo "[AVISO] Binário ${bin} não encontrado; pulando Tarefa C." >&2
        return
    fi

    for N in "${Ns[@]}"; do
        for K in "${Ks[@]}"; do
            # V1: sequencial (idealmente sem paralelismo; threads fixo em 1)
            run_experiment_point "C" "seq" "${N}" "K" "${K}" 1 \
                "${bin}" "${N}" "${K}" "seq"

            for threads in "${THREADS[@]}"; do
                # V2: #pragma omp simd
                run_experiment_point "C" "simd" "${N}" "K" "${K}" "${threads}" \
                    "${bin}" "${N}" "${K}" "simd"

                # V3: #pragma omp parallel for simd
                run_experiment_point "C" "parallel_simd" "${N}" "K" "${K}" "${threads}" \
                    "${bin}" "${N}" "${K}" "parallel_simd"
            done
        done
    done
}

###############################################################################
# Tarefa D — Organização de região paralela
###############################################################################

run_task_D() {
    local bin="./bin/task_d_omp"

    if [[ ! -x "${bin}" ]]; then
        echo "[AVISO] Binário ${bin} não encontrado; pulando Tarefa D." >&2
        return
    fi

    for N in "${Ns[@]}"; do
        for threads in "${THREADS[@]}"; do
            # Variante ingênua: dois parallel for consecutivos
            run_experiment_point "D" "two_parallel_for" "${N}" "-" "-" "${threads}" \
                "${bin}" "${N}" "two_parallel_for"

            # Variante arrumada: uma parallel englobando dois for internos
            run_experiment_point "D" "single_parallel" "${N}" "-" "-" "${threads}" \
                "${bin}" "${N}" "single_parallel"
        done
    done
}

###############################################################################
# Ponto de entrada
###############################################################################

main() {
    # Se o usuário passar argumentos, interpretamos como lista de tarefas
    # a serem executadas (por ex.: ./run.sh A C). Caso contrário, rodamos
    # todas as tarefas.

    local tasks=()
    if (( $# > 0 )); then
        tasks=("$@")
    else
        tasks=(A B C D)
    fi

    for t in "${tasks[@]}"; do
        case "${t}" in
            A|a) run_task_A ;;
            B|b) run_task_B ;;
            C|c) run_task_C ;;
            D|d) run_task_D ;;
            *)
                echo "[AVISO] Tarefa desconhecida: ${t} (use A, B, C ou D)" >&2
                ;;
        esac
    done

    echo "Resultados salvos em ${CSV_FILE}"
}

main "$@"

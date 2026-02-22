#ifndef CONFIG_HPP
#define CONFIG_HPP

namespace Config {
    // Configurações do Grid Global
    constexpr int LARGURA_GRID = 1000;
    constexpr int ALTURA_GRID = 1000;
    
    // Configurações da Simulação
    constexpr int SEED = 42;
    constexpr int TOTAL_CICLOS = 20;
    constexpr int TAMANHO_CICLO_SAZONAL = 4;
    
    // Configurações dos Agentes
    constexpr int N_AGENTS = 10000;
    constexpr float ENERGIA_INICIAL_AGENTE = 20.0f;
    constexpr float RECURSO_REQUERIDO_AGENTE = 5.0f;
    constexpr float EFICIENCIA_REABASTECIMENTO = 0.4f;
    
    // Configurações de Carga de Trabalho e Custo de Energia
    constexpr int MAX_CUSTO = 10000;
    constexpr float CUSTO_METABOLICO = 2.0f;
    constexpr float TAXA_CUSTO_ESFORCO = 0.002f;
    constexpr float FATOR_CARGA_TRABALHO = 100.0f;
    
    // Configurações de Território (Recursos Máximos)
    constexpr float RECURSO_MAX_ALDEIA = 25.0f;
    constexpr float RECURSO_MAX_PESCA = 15.0f;
    constexpr float RECURSO_MAX_COLETA = 10.0f;
    constexpr float RECURSO_MAX_ROCADO = 20.0f;
    
    // Taxas de Regeneração
    constexpr float TAXA_REGENERACAO_CHEIA = 0.1f;
    constexpr float TAXA_REGENERACAO_SECA = 0.05f;
    
    // Fatores de tipos de terreno (f_tipo)
    constexpr int MODULO_ALDEIA = 10;
    constexpr int MODULO_PESCA = 5;
    constexpr int MODULO_ROCADO = 4;
}

#endif // CONFIG_HPP

// ============================================================================ 
//         PROJETO WAR ESTRUTURADO - DESAFIO DE CÓDIGO
// ============================================================================
//        
// ============================================================================
//
// OBJETIVOS:
// - Modularizar completamente o código em funções especializadas.
// - Implementar um sistema de missões para um jogador.
// - Criar uma função para verificar se a missão foi cumprida.
// - Utilizar passagem por referência (ponteiros) para modificar dados e
//   passagem por valor/referência constante (const) para apenas ler.
// - Foco em: Design de software, modularização, const correctness, lógica de jogo.
//
// ============================================================================

// Inclusão das bibliotecas padrão necessárias para entrada/saída, alocação de memória, manipulação de strings e tempo.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

// --- Constantes Globais ---
// Definem valores fixos para o número de territórios, missões e tamanho máximo de strings, facilitando a manutenção.
#define TOTAL_TERRITORIOS 5
#define TAM_NOME 50
#define TAM_COR 20
#define MISS_DESC_TAM 128

// --- Estrutura de Dados ---
// Define a estrutura para um território, contendo seu nome, a cor do exército que o domina e o número de tropas.
typedef struct {
    char nome[TAM_NOME];
    char corExercito[TAM_COR];
    int tropas;
} Territorio;

// Códigos ANSI para cores no terminal (uso opcional em terminais compatíveis)
static const char *coresANSI[] = {"\033[32m", "\033[34m", "\033[31m", "\033[33m", "\033[35m"};
static const char *resetANSI = "\033[0m";

// --- Protótipos das Funções ---
// Declarações antecipadas de todas as funções que serão usadas no programa, organizadas por categoria.

// Funções de setup e gerenciamento de memória:
Territorio *alocarMapa(size_t total);
void inicializarTerritorios(Territorio *territorios, size_t total);
void liberarMemoria(Territorio *territorios);

// Funções de interface com o usuário:
void exibirMenuPrincipal(void);
void exibirMapa(const Territorio *territorios, size_t total);
void exibirMissao(int idMissao, const char *alvoMissao);

// Funções de lógica principal do jogo:
void faseDeAtaque(Territorio *territorios, size_t total, const char *corJogador);
void simularAtaque(Territorio *atacante, Territorio *defensor, const char *corJogador);
int sortearMissao(char *descricao, size_t descSize, char *alvoMissao, size_t alvoSize, const char *corJogador);
int verificarVitoria(const Territorio *territorios, size_t total, int idMissao, const char *alvoMissao, const char *corJogador);

// Função utilitária:
void limparBufferEntrada(void);
int indiceCorParaANSI(const char *cor); // ajuda para cor no terminal (retorna índice ou -1)

// --- Função Principal (main) ---
// Função principal que orquestra o fluxo do jogo, chamando as outras funções em ordem.
int main(void) {
    // 1. Configuração Inicial (Setup):
    // - Define o locale para português.
    // - Inicializa a semente para geração de números aleatórios com base no tempo atual.
    // - Aloca a memória para o mapa do mundo e verifica se a alocação foi bem-sucedida.
    // - Preenche os territórios com seus dados iniciais (tropas, donos, etc.).
    // - Define a cor do jogador e sorteia sua missão secreta.

    setlocale(LC_ALL, "");      // define locale (ajuda em ambientes que usam acentuação)
    srand((unsigned int)time(NULL)); // inicializa aleatoriedade

    // cor do jogador (pode ser parametrizada)
    const char corJogador[TAM_COR] = "Azul";

    // aloca e inicializa mapa
    Territorio *mapa = alocarMapa(TOTAL_TERRITORIOS);
    if (mapa == NULL) {
        fprintf(stderr, "Erro: falha ao alocar memória para o mapa.\n");
        return EXIT_FAILURE;
    }
    inicializarTerritorios(mapa, TOTAL_TERRITORIOS);

    // sorteia missão
    char descricaoMissao[MISS_DESC_TAM] = {0};
    char alvoMissao[TAM_COR] = {0}; // por exemplo: "Verde" se missão for destruir exército Verde
    int idMissao = sortearMissao(descricaoMissao, sizeof(descricaoMissao), alvoMissao, sizeof(alvoMissao), corJogador);

    // 2. Laço Principal do Jogo (Game Loop):
    // - Roda em um loop 'do-while' que continua até o jogador sair (opção 0) ou vencer.
    // - A cada iteração, exibe o mapa, a missão e o menu de ações.
    // - Lê a escolha do jogador e usa um 'switch' para chamar a função apropriada:
    //   - Opção 1: Inicia a fase de ataque.
    //   - Opção 2: Verifica se a condição de vitória foi alcançada e informa o jogador.
    //   - Opção 0: Encerra o jogo.
    // - Pausa a execução para que o jogador possa ler os resultados antes da próxima rodada.

    int opcao;
    int venceu = 0;
    do {
        exibirMapa((const Territorio *)mapa, TOTAL_TERRITORIOS);
        exibirMissao(idMissao, alvoMissao);

        exibirMenuPrincipal();
        if (scanf("%d", &opcao) != 1) { limparBufferEntrada(); opcao = -1; }
        limparBufferEntrada();

        switch (opcao) {
            case 1:
                faseDeAtaque(mapa, TOTAL_TERRITORIOS, corJogador);
                break;
            case 2:
                if (verificarVitoria((const Territorio *)mapa, TOTAL_TERRITORIOS, idMissao, alvoMissao, corJogador)) {
                    printf("\nParabéns! Você cumpriu a missão: %s\n", descricaoMissao);
                    venceu = 1;
                } else {
                    printf("\nMissão NÃO cumprida ainda: %s\n", descricaoMissao);
                }
                break;
            case 0:
                printf("\nSaindo do jogo...\n");
                break;
            default:
                printf("\nOpção inválida. Tente novamente.\n");
        }

        if (!venceu && opcao != 0) {
            printf("\nPressione Enter para continuar...");
            getchar(); // pausa para o jogador ler
        }

    } while (opcao != 0 && !venceu);

    // 3. Limpeza:
    // - Ao final do jogo, libera a memória alocada para o mapa para evitar vazamentos de memória.
    liberarMemoria(mapa);

    return EXIT_SUCCESS;
}

// --- Implementação das Funções ---

// alocarMapa():
// Aloca dinamicamente a memória para o vetor de territórios usando calloc.
// Retorna um ponteiro para a memória alocada ou NULL em caso de falha.
Territorio *alocarMapa(size_t total) {
    Territorio *m = (Territorio *)calloc(total, sizeof(Territorio));
    return m;
}

// inicializarTerritorios():
// Preenche os dados iniciais de cada território no mapa (nome, cor do exército, número de tropas).
// Esta função modifica o mapa passado por referência (ponteiro).
void inicializarTerritorios(Territorio *territorios, size_t total) {
    // valores padrão iniciais (poderiam ser lidos de arquivo ou gerados aleatoriamente)
    const char *nomes[TOTAL_TERRITORIOS] = {"Amazonas", "Cerrado", "Pantanal", "Caatinga", "Mata Atlantica"};
    const char *cores[TOTAL_TERRITORIOS] = {"Verde", "Azul", "Vermelho", "Amarelo", "Roxo"};
    const int tropas[TOTAL_TERRITORIOS] = {5, 4, 6, 3, 5};

    for (size_t i = 0; i < total; ++i) {
        strncpy(territorios[i].nome, nomes[i], TAM_NOME - 1);
        territorios[i].nome[TAM_NOME - 1] = '\0';
        strncpy(territorios[i].corExercito, cores[i], TAM_COR - 1);
        territorios[i].corExercito[TAM_COR - 1] = '\0';
        territorios[i].tropas = tropas[i];
    }
}

// liberarMemoria():
// Libera a memória previamente alocada para o mapa usando free.
void liberarMemoria(Territorio *territorios) {
    free(territorios);
}

// exibirMenuPrincipal():
// Imprime na tela o menu de ações disponíveis para o jogador.
void exibirMenuPrincipal(void) {
    printf("Menu:\n");
    printf("  1 - Atacar\n");
    printf("  2 - Verificar Missão\n");
    printf("  0 - Sair\n");
    printf("Escolha uma opção: ");
}

// exibirMapa():
// Mostra o estado atual de todos os territórios no mapa, formatado como uma tabela.
// Usa 'const' para garantir que a função apenas leia os dados do mapa, sem modificá-los.
void exibirMapa(const Territorio *territorios, size_t total) {
    printf("\n=== Estado Atual do Mapa ===\n");
    printf("Idx | Território               | Exército    | Tropas\n");
    printf("----+---------------------------+-------------+--------\n");
    for (size_t i = 0; i < total; ++i) {
        int idxCor = indiceCorParaANSI(territorios[i].corExercito);
        if (idxCor >= 0) {
            printf("%3zu | %-25s | %s%-11s%s | %6d\n",
                   i + 1,
                   territorios[i].nome,
                   coresANSI[idxCor],
                   territorios[i].corExercito,
                   resetANSI,
                   territorios[i].tropas);
        } else {
            printf("%3zu | %-25s | %-11s | %6d\n",
                   i + 1,
                   territorios[i].nome,
                   territorios[i].corExercito,
                   territorios[i].tropas);
        }
    }
    printf("\n");
}

// exibirMissao():
// Exibe a descrição da missão atual do jogador com base no ID da missão sorteada.
void exibirMissao(int idMissao, const char *alvoMissao) {
    printf("=== Missão Atual ===\n");
    if (idMissao == 0) {
        printf("  Objetivo: Destruir o exército %s\n", alvoMissao);
    } else if (idMissao == 1) {
        printf("  Objetivo: Conquistar 3 territórios (ser dono de pelo menos 3 territórios)\n");
    } else {
        printf("  Missão desconhecida\n");
    }
    printf("\n");
}

// faseDeAtaque():
// Gerencia a interface para a ação de ataque, solicitando ao jogador os territórios de origem e destino.
// Chama a função simularAtaque() para executar a lógica da batalha.
void faseDeAtaque(Territorio *territorios, size_t total, const char *corJogador) {
    int nAtaques = 1;
    printf("Quantos ataques deseja realizar neste turno? ");
    if (scanf("%d", &nAtaques) != 1) { limparBufferEntrada(); printf("Entrada inválida. Voltando ao menu.\n"); return; }
    limparBufferEntrada();

    for (int i = 0; i < nAtaques; ++i) {
        printf("\n>>> Ataque %d de %d <<<\n", i + 1, nAtaques);
        int atk = 0, def = 0;
        printf("Escolha o território atacante (1 - %zu): ", total);
        if (scanf("%d", &atk) != 1) { limparBufferEntrada(); printf("Entrada inválida. Pulando ataque.\n"); continue; }
        limparBufferEntrada();
        printf("Escolha o território defensor (1 - %zu): ", total);
        if (scanf("%d", &def) != 1) { limparBufferEntrada(); printf("Entrada inválida. Pulando ataque.\n"); continue; }
        limparBufferEntrada();

        if (atk < 1 || atk > (int)total || def < 1 || def > (int)total || atk == def) {
            printf("Opção inválida (índices fora de intervalo ou territórios iguais). Ataque cancelado.\n");
            continue;
        }

        // executa ataque
        simularAtaque(&territorios[atk - 1], &territorios[def - 1], corJogador);
    }
}

// simularAtaque():
// Executa a lógica de uma batalha entre dois territórios.
// Realiza validações, rola os dados, compara os resultados e atualiza o número de tropas.
// Se um território for conquistado, atualiza seu dono e move uma tropa.
void simularAtaque(Territorio *atacante, Territorio *defensor, const char *corJogador) {
    if (atacante->tropas <= 0) {
        printf("Território atacante '%s' não tem tropas suficientes.\n", atacante->nome);
        return;
    }
    if (defensor->tropas <= 0) {
        printf("Território defensor '%s' já está vazio.\n", defensor->nome);
        return;
    }

    // Rolar dados (1..6)
    int dadoAtaque = rand() % 6 + 1;
    int dadoDefesa  = rand() % 6 + 1;

    printf("%s (tropas: %d, exército: %s) ataca %s (tropas: %d, exército: %s)\n",
           atacante->nome, atacante->tropas, atacante->corExercito,
           defensor->nome, defensor->tropas, defensor->corExercito);
    printf("Rolagem: atacante %d vs defensor %d\n", dadoAtaque, dadoDefesa);

    if (dadoAtaque >= dadoDefesa) {
        // atacante vence (empates favorecem atacante)
        defensor->tropas -= 1;
        printf("Resultado: %s perde 1 tropa (agora %d).\n", defensor->nome, defensor->tropas);
        if (defensor->tropas <= 0) {
            // conquista: mudar dono e mover 1 tropa do atacante (mínimo)
            printf("Território %s foi conquistado por %s!\n", defensor->nome, atacante->corExercito);
            // atualiza cor para cor do atacante
            strncpy(defensor->corExercito, atacante->corExercito, TAM_COR - 1);
            defensor->corExercito[TAM_COR - 1] = '\0';
            // mover 1 tropa do atacante para o defensor (se possível)
            if (atacante->tropas > 1) {
                atacante->tropas -= 1;
                defensor->tropas = 1;
                printf("Uma tropa foi movida de %s para %s.\n", atacante->nome, defensor->nome);
            } else {
                // se atacante só tinha 1 tropa, defender fica com 1 (já está) e atacante fica 0
                defensor->tropas = 1;
                atacante->tropas = 0;
            }
        }
    } else {
        // defensor vence
        printf("Resultado: defesa bem sucedida. Nenhuma perda do defensor.\n");
    }

    printf("\n");
}

// sortearMissao():
// Sorteia e retorna um ID de missão aleatório para o jogador.
// Além disso preenche a descrição e o alvo (quando aplicável).
// Retorna 0 para tipo 'destruir exército X' e 1 para 'conquistar 3 territórios'.
int sortearMissao(char *descricao, size_t descSize, char *alvoMissao, size_t alvoSize, const char *corJogador) {
    // tipos de missões possíveis:
    // 0 -> Destruir exército <COR_ALVO> (escolhida aleatoriamente, diferente do jogador)
    // 1 -> Conquistar 3 territórios (ser dono de >= 3 territórios)

    int tipo = rand() % 2;
    if (tipo == 0) {
        // escolher uma cor alvo diferente da do jogador
        const char *possiveis[] = {"Verde", "Azul", "Vermelho", "Amarelo", "Roxo"};
        size_t n = sizeof(possiveis) / sizeof(possiveis[0]);
        // escolhe aleatoriamente até que não seja a cor do jogador
        const char *escolhida = NULL;
        for (int tent = 0; tent < 10 && escolhida == NULL; ++tent) {
            const char *c = possiveis[rand() % n];
            if (strcmp(c, corJogador) != 0) escolhida = c;
        }
        if (escolhida == NULL) {
            // fallback: escolher a primeira que não seja do jogador
            for (size_t i = 0; i < n; ++i) if (strcmp(possiveis[i], corJogador) != 0) { escolhida = possiveis[i]; break; }
        }
        strncpy(alvoMissao, escolhida, alvoSize - 1);
        alvoMissao[alvoSize - 1] = '\0';
        snprintf(descricao, descSize, "Destruir o exército %s", alvoMissao);
    } else {
        strncpy(alvoMissao, "3Territorios", alvoSize - 1);
        alvoMissao[alvoSize - 1] = '\0';
        snprintf(descricao, descSize, "Conquistar 3 territórios");
    }
    return tipo;
}

// verificarVitoria():
// Verifica se o jogador cumpriu os requisitos de sua missão atual.
// Implementa a lógica para cada tipo de missão (destruir um exército ou conquistar um número de territórios).
// Retorna 1 (verdadeiro) se a missão foi cumprida, e 0 (falso) caso contrário.
int verificarVitoria(const Territorio *territorios, size_t total, int idMissao, const char *alvoMissao, const char *corJogador) {
    if (idMissao == 0) {
        // destruir exército alvo: verificar se ainda existe algum território com exército alvo e tropas > 0
        for (size_t i = 0; i < total; ++i) {
            if (strcmp(territorios[i].corExercito, alvoMissao) == 0 && territorios[i].tropas > 0) {
                return 0; // ainda existe exército alvo
            }
        }
        return 1; // não existe mais exército alvo => missão cumprida
    } else if (idMissao == 1) {
        // conquistar 3 territórios: contar quantos territórios são do jogador
        int contador = 0;
        for (size_t i = 0; i < total; ++i) {
            if (strcmp(territorios[i].corExercito, corJogador) == 0) contador++;
        }
        return (contador >= 3) ? 1 : 0;
    }
    return 0;
}

// limparBufferEntrada():
// Função utilitária para limpar o buffer de entrada do teclado (stdin), evitando problemas com leituras consecutivas de scanf e getchar.
void limparBufferEntrada(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* descarta */ }
}

// indiceCorParaANSI():
// Retorna o índice do array coresANSI para uma cor conhecida (ou -1 caso não encontre).
int indiceCorParaANSI(const char *cor) {
    if (strcmp(cor, "Verde") == 0) return 0;
    if (strcmp(cor, "Azul") == 0) return 1;
    if (strcmp(cor, "Vermelho") == 0) return 2;
    if (strcmp(cor, "Amarelo") == 0) return 3;
    if (strcmp(cor, "Roxo") == 0) return 4;
    return -1;
}
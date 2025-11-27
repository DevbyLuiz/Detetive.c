#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 53   // tabela hash simples (primo pequeno)
#define MAX_COLETADAS 100

/* ------------------ STRUCTS ------------------ */

/* Nó da árvore binária que representa uma sala da mansão */
typedef struct Sala {
    char *nome;         // nome da sala
    char *pista;        // texto da pista (pode ser NULL)
    struct Sala *esq;   // sala à esquerda
    struct Sala *dir;   // sala à direita
} Sala;

/* Nó da BST que guarda pistas coletadas (ordenadas por string) */
typedef struct PistaNode {
    char *pista;
    struct PistaNode *esq;
    struct PistaNode *dir;
} PistaNode;

/* Nó para encadeamento na tabela hash (mapeia pista -> suspeito) */
typedef struct HashNode {
    char *chave;            // pista
    char *suspeito;         // suspeito associado
    struct HashNode *proximo;
} HashNode;

/* ------------------ VARIÁVEIS GLOBAIS ------------------ */

// BST root para pistas coletadas
PistaNode *rootPistas = NULL;

// Tabela hash (vetor de ponteiros para listas encadeadas)
HashNode *hashTable[HASH_SIZE];

// Lista dinâmica de pistas coletadas (cópias alocadas)
char *coletadas[MAX_COLETADAS];
int totalColetadas = 0;

/* ------------------ FUNÇÕES AUXILIARES ------------------ */

/* Aloca e duplica string (substitui strdup para portabilidade) */
char *strdup_safe(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (!p) {
        fprintf(stderr, "Erro: sem memoria\n");
        exit(1);
    }
    memcpy(p, s, n);
    return p;
}

/* Função de hash simples para strings */
unsigned int hash_func(const char *s) {
    unsigned int h = 0;
    while (*s) {
        h = (h * 31 + (unsigned char)*s) % HASH_SIZE;
        s++;
    }
    return h;
}

/* ------------------ MANIPULAÇÃO DA TABELA HASH ------------------ */

/* Insere associação (pista -> suspeito) na tabela hash */
void inserirNaHash(const char *pista, const char *suspeito) {
    unsigned int idx = hash_func(pista);
    HashNode *node = malloc(sizeof(HashNode));
    node->chave = strdup_safe(pista);
    node->suspeito = strdup_safe(suspeito);
    node->proximo = hashTable[idx];
    hashTable[idx] = node;
}

/* Busca suspeito associado a uma pista; retorna NULL se não achar */
char *encontrarSuspeito(const char *pista) {
    unsigned int idx = hash_func(pista);
    HashNode *cur = hashTable[idx];
    while (cur) {
        if (strcmp(cur->chave, pista) == 0) return cur->suspeito;
        cur = cur->proximo;
    }
    return NULL;
}

/* ------------------ BST DE PISTAS (coleção) ------------------ */

/* Cria um novo nó de pista */
PistaNode *criarPistaNode(const char *pista) {
    PistaNode *n = malloc(sizeof(PistaNode));
    n->pista = strdup_safe(pista);
    n->esq = n->dir = NULL;
    return n;
}

/* Insere pista na BST se ainda não existir (evita duplicatas) */
PistaNode *inserirPistaBST(PistaNode *root, const char *pista, int *inseriu) {
    if (!root) {
        *inseriu = 1;
        return criarPistaNode(pista);
    }
    int cmp = strcmp(pista, root->pista);
    if (cmp == 0) {
        *inseriu = 0;
        return root; // já existe
    } else if (cmp < 0) {
        root->esq = inserirPistaBST(root->esq, pista, inseriu);
    } else {
        root->dir = inserirPistaBST(root->dir, pista, inseriu);
    }
    return root;
}

/* Percorre em ordem e imprime pistas (alfabético) */
void exibirPistasInOrder(PistaNode *root) {
    if (!root) return;
    exibirPistasInOrder(root->esq);
    printf("- %s\n", root->pista);
    exibirPistasInOrder(root->dir);
}

/* Libera memória da BST de pistas */
void liberarPistasBST(PistaNode *root) {
    if (!root) return;
    liberarPistasBST(root->esq);
    liberarPistasBST(root->dir);
    free(root->pista);
    free(root);
}

/* ------------------ ÁRVORE BINÁRIA DE SALAS ------------------ */

/* criarSala: cria dinamicamente uma sala com nome e pista (pista pode ser NULL) */
Sala *criarSala(const char *nome, const char *pista) {
    Sala *s = malloc(sizeof(Sala));
    s->nome = strdup_safe(nome);
    s->pista = pista ? strdup_safe(pista) : NULL;
    s->esq = s->dir = NULL;
    return s;
}

/* liberar toda a árvore de salas (recursivo) */
void liberarSalas(Sala *root) {
    if (!root) return;
    liberarSalas(root->esq);
    liberarSalas(root->dir);
    free(root->nome);
    if (root->pista) free(root->pista);
    free(root);
}

/* ------------------ COLETA: adicionar pista à BST e à lista de coletadas ------------------ */

void coletarPista(const char *pista) {
    if (!pista) return;
    // evita exceder limite
    if (totalColetadas >= MAX_COLETADAS) return;

    // inserir na BST (evita duplicatas)
    int inseriu = 0;
    rootPistas = inserirPistaBST(rootPistas, pista, &inseriu);
    if (inseriu) {
        // adicionar cópia ao vetor de coletadas para posterior contagem
        coletadas[totalColetadas++] = strdup_safe(pista);
        printf("Pista coletada: \"%s\"\n", pista);
    } else {
        printf("Pista já coletada anteriormente: \"%s\"\n", pista);
    }
}

/* ------------------ EXPLORAÇÃO INTERATIVA (com pistas) ------------------ */

/*
 explorarSalasComPistas:
 Navega a árvore de salas a partir do nó atual.
 Opções: 'e' para esquerda, 'd' para direita, 's' para sair.
 Ao entrar em uma sala, se houver pista, ela é automaticamente coletada.
 As visitas continuam até o jogador escolher 's' ou chegar em nó-folha
 (sem filhos).
*/
void explorarSalasComPistas(Sala *atual) {
    if (!atual) return;
    char comando[8];
    Sala *cur = atual;

    while (1) {
        printf("\nVocê está na sala: %s\n", cur->nome);
        if (cur->pista) {
            printf("Há uma pista aqui: \"%s\"\n", cur->pista);
            coletarPista(cur->pista);
        } else {
            printf("Nenhuma pista nesta sala.\n");
        }

        // se for nó-folha, avisar e permitir saída ou retroceder não implementado
        if (!cur->esq && !cur->dir) {
            printf("Esta sala não tem caminhos. Digite 's' para sair ou volte para o início do programa.\n");
        } else {
            printf("Escolha: (e) esquerda | (d) direita | (s) sair: ");
        }

        // ler opção
        if (!fgets(comando, sizeof(comando), stdin)) break;
        // remover newline
        comando[strcspn(comando, "\n")] = 0;

        if (strcmp(comando, "s") == 0) {
            printf("Saindo da exploração...\n");
            break;
        } else if (strcmp(comando, "e") == 0) {
            if (cur->esq) cur = cur->esq;
            else printf("Não há caminho à esquerda.\n");
        } else if (strcmp(comando, "d") == 0) {
            if (cur->dir) cur = cur->dir;
            else printf("Não há caminho à direita.\n");
        } else {
            printf("Comando inválido. Use 'e', 'd' ou 's'.\n");
        }
    }
}

/* ------------------ FINAL: verificar acusado ------------------ */

/* Conta quantas pistas coletadas apontam para o suspeito acusado */
int contarPistasParaSuspeito(const char *acusado) {
    int cont = 0;
    for (int i = 0; i < totalColetadas; i++) {
        char *s = encontrarSuspeito(coletadas[i]);
        if (s && strcmp(s, acusado) == 0) cont++;
    }
    return cont;
}

/* Libera memória da tabela hash */
void liberarHash() {
    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode *cur = hashTable[i];
        while (cur) {
            HashNode *next = cur->proximo;
            free(cur->chave);
            free(cur->suspeito);
            free(cur);
            cur = next;
        }
        hashTable[i] = NULL;
    }
}

/* ------------------ MONTAGEM DA MANSÃO E MAPA DE PISTAS/SUSPEITOS ------------------ */

int main() {
    // construir mapa fixo (manual, simples)
    // Hall (raiz)
    Sala *hall = criarSala("Hall de Entrada", NULL);

    // nível 1
    hall->esq = criarSala("Sala de Estar", "pegada no tapete");            // pista A
    hall->dir = criarSala("Biblioteca", "página arrancada do livro");     // pista B

    // nível 2 (esquerda)
    hall->esq->esq = criarSala("Cozinha", "faca com digitais");           // pista C
    hall->esq->dir = criarSala("Jardim", NULL);

    // nível 2 (direita)
    hall->dir->esq = criarSala("Sala de Jantar", "vidro quebrado");       // pista D
    hall->dir->dir = criarSala("Escritório", "bilhete rasgado");         // pista E

    // nível 3 (algumas folhas)
    hall->esq->dir->esq = criarSala("Estufa", "pegada molhada");         // pista F
    hall->dir->dir->dir = criarSala("Quarto do Dono", "chave perdida");  // pista G

    // definir mapa de pistas -> suspeitos (tabela hash)
    inserirNaHash("pegada no tapete", "Carlos");
    inserirNaHash("página arrancada do livro", "Mariana");
    inserirNaHash("faca com digitais", "Carlos");
    inserirNaHash("vidro quebrado", "Luisa");
    inserirNaHash("bilhete rasgado", "Mariana");
    inserirNaHash("pegada molhada", "Carlos");
    inserirNaHash("chave perdida", "Luisa");

    // Mensagem inicial
    printf("Bem-vindo a Detective Quest!\n");
    printf("Explore a mansão e colete pistas. Ao final, acuse o suspeito.\n");
    printf("Comandos de navegação: 'e' = esquerda, 'd' = direita, 's' = sair\n");
    printf("Pressione Enter para começar...\n");
    // limpar buffer e aguardar ENTER
    getchar();

    // explorar mansão a partir do Hall (coleta automática de pistas)
    explorarSalasComPistas(hall);

    // exibir pistas coletadas em ordem alfabética (BST)
    printf("\nPistas coletadas (ordenadas):\n");
    if (rootPistas) exibirPistasInOrder(rootPistas);
    else printf("Nenhuma pista coletada.\n");

    // julgamento: o jogador acusa um suspeito
    char acusado[100];
    printf("\nQuem você acusa? Digite o nome do suspeito: ");
    if (!fgets(acusado, sizeof(acusado), stdin)) acusado[0] = '\0';
    acusado[strcspn(acusado, "\n")] = 0; // remove newline

    int cont = contarPistasParaSuspeito(acusado);
    printf("\nPistas que apontam para %s: %d\n", acusado, cont);

    if (cont >= 2) {
        printf("Acusação aceita: há evidências suficientes. Parabéns, Detetive!\n");
    } else {
        printf("Acusação rejeitada: não há evidências suficientes.\n");
    }

    // liberar tudo
    liberarPistasBST(rootPistas);
    for (int i = 0; i < totalColetadas; i++) free(coletadas[i]);
    liberarHash();
    liberarSalas(hall);

    return 0;
}


# Detective Quest - Mapa e Pistas (Árvore + BST + Hash)

Trabalho de Estrutura de Dados (ADS). Simula uma mansão como árvore binária, coleta pistas em uma BST e associa pistas a suspeitos via tabela hash.

## O que faz
- Explora salas (árvore binária) a partir do Hall de Entrada.
- Coleta pistas automaticamente em cada sala visitada.
- Armazena pistas em uma BST (ordenadas alfabeticamente).
- Associação pista -> suspeito em tabela hash.
- Ao final, o jogador acusa um suspeito; o programa verifica se há ≥2 pistas contra ele.

## Como compilar
```bash
gcc detective.c -o detective

/*OBJETIVO PRINCIPAL:
Sempre conseguir pegar rapidamente o nó com menor frequência para construir a árvore de Huffman.*/

#ifndef PQUEUE_HEAP_H
#define PQUEUE_HEAP_H

/*Problema: Inclusão Duplicada
Imagine se você incluir o mesmo arquivo duas vezes:

#include "pqueue_heap.h"
#include "pqueue_heap.h"  // ← OPS! Duplicado!
O compilador veria as mesmas funções duas vezes → ERRO!

Solução: Guardas de Inclusão

#ifndef PQUEUE_HEAP_H    // "Se PQUEUE_HEAP_H NÃO está definido"
#define PQUEUE_HEAP_H    // "Defina PQUEUE_HEAP_H"

// Todo o código do arquivo aqui...

#endif  // Fim do bloco condicional

Como funciona:

Primeira inclusão:
#ifndef PQUEUE_HEAP_H → VERDADEIRO (nunca foi definido)
#define PQUEUE_HEAP_H → Define o "marcador"
// Compila todo o código

Segunda inclusão:  
#ifndef PQUEUE_HEAP_H → FALSO (já foi definido)
// PULA todo o código → Evita duplicação!
É como um "porteiro inteligente" que só deixa entrar uma vez! 

EXEMPLO PRÁTICO NO  CÓDIGO:
O problema da dependência circular:

No huffman.h:
#include "pqueue_heap.h"  ← Entra em pqueue_heap.h

No pqueue_heap.h:  
#include "huffman.h"  ← Tenta entrar em huffman.h DE NOVO!

Como #ifndef resolve:

1ª inclusão: huffman.h → #ifndef HUFFMAN_H → VERDADEIRO → Define HUFFMAN_H
2ª inclusão: pqueue_heap.h → #include "huffman.h" → #ifndef HUFFMAN_H → FALSO → PULA!

Sem #ifndef: Loop infinito de inclusões! 
Com #ifndef: Cada arquivo é incluído apenas uma vez! 

*/

#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_HEAP 256 //Porque temos 256 caracteres possíveis (0-255)

typedef struct NODE {
    unsigned char character; 
    int frequency; 
    struct NODE *left, *right; // Filhos na árvore
} NODE;

typedef struct {
    NODE* data[MAX_HEAP];  // Array de ponteiros para nós, se comporta como uma árvore heap
    int size;
} PRIORITY_QUEUE;


NODE* create_node(unsigned char c, int freq, NODE* left, NODE* right) { //Cria um novo nó atribuindo os parâmetros fornecidos
    NODE* NODE = malloc(sizeof(*NODE));
    NODE->character = c;
    NODE->frequency = freq;
    NODE->left = left;
    NODE->right = right;
    return NODE;
}

PRIORITY_QUEUE* create_queue() { // Cria uma nova fila de prioridade, menos frequencia primeiro
    PRIORITY_QUEUE* pq = malloc(sizeof(PRIORITY_QUEUE));
    pq->size = 0;
    return pq;
}

// Adicione isso antes de qualquer uso da função
void free_huffman_tree(NODE* root);

/*
O problema:
O pqueue_heap.h precisa liberar memória dos nós, mas a função free_huffman_tree() está definida no huffman.h!

A solução:

void free_huffman_tree(NODE* root); <- Declaração ANTECIPADA

Isso diz ao compilador: "Confia em mim, essa função existe em outro lugar!"

Por que é necessário?
Sem essa declaração, quando o compilador vê:

free_huffman_tree(pq->data[i]);  ← no free_priority_queue()
Ele pensa: "Que função é essa? Nunca vi!" → ERRO!

Fluxo correto:

Compilação:
1. Vê void free_huffman_tree(NODE* root); → "Ok, existe em algum lugar"
2. Vê free_huffman_tree(pq->data[i]) → "Ah, é aquela que prometeram!"
3. Na ligação (linking): Encontra a implementação real no huffman.h

Essa declaração antecipada resolve o problema da dependência circular entre os arquivos!
*/

int is_empty(PRIORITY_QUEUE* pq) {
    return pq->size == 0; //Fila vazia
}

//Quando inserimos ou removemos elementos, precisamos trocar posições no array para manter a ordem do heap.
void swap(NODE** a, NODE** b) { //Troca dois nós
    NODE* temp = *a; // Guarda o valor de a temporariamente
    *a = *b; // Coloca o valor de b em a
    *b = temp; // Coloca o valor de b em a
}

// Sempre que inserimos um novo elemento no final do heap, compara o elemento com seu pai e sobe até encontrar a posição correta.
void heapify_up(PRIORITY_QUEUE* pq, int idx) {
    int dad = (idx - 1) / 2; // Fórmula para encontrar o pai

    // Se não é a raiz E é menor que o pai
    if (idx > 0 && pq->data[idx]->frequency < pq->data[dad]->frequency) {
        swap(&pq->data[idx], &pq->data[dad]); // Troca com o pai
        heapify_up(pq, dad); // Continua subindo
    }
}

//Sempre que removemos o elemento do topo do heap. Pega o último elemento (colocado no topo) e desce até encontrar a posição correta.
void heapify_down(PRIORITY_QUEUE* pq, int idx) {
    int lower = idx; // Assume que atual é o menor
    int left = 2 * idx + 1; // Filho esquerdo
    int right = 2 * idx + 2; // Filho direito

    // Verifica se filho esquerdo é menor
    if (left < pq->size && pq->data[left]->frequency < pq->data[lower]->frequency)
        lower = left;

    // Verifica se filho direito é menor
    if (right < pq->size && pq->data[right]->frequency < pq->data[lower]->frequency)
        lower = right;

     // Se encontrou um filho menor, troca e continua descendo
    if (lower != idx) {
        swap(&pq->data[idx], &pq->data[lower]);
        heapify_down(pq, lower);
    }
}

/*
Array: [A, B, C, D, E, F, G]
       ↓
Árvore:
        A(0)
       /    \
     B(1)   C(2)
    /   \   /   \
  D(3) E(4) F(5) G(6)

Fórmulas mágicas:

Pai de i: (i - 1) / 2
Filho esquerdo: 2*i + 1
Filho direito: 2*i + 2

Exemplos:
Nó B(1) → Pai: (1-1)/2 = 0 → A 

Nó A(0) → Filho esq: 2*0+1 = 1 → B, Filho dir: 2*0+2 = 2 → C
*/

void insert(PRIORITY_QUEUE* pq, NODE* node) { //
    if (pq->size >= MAX_HEAP) {
        fprintf(stderr, "Erro: A fila de prioridades está cheia\n");
        return;
    }
    pq->data[pq->size] = node; //Coloca no final do array
    heapify_up(pq, pq->size);//Chama heapify_up para organizar 
    //"Sobe" o nó para cima até encontrar lugar certo
    //Mantém a propriedade do heap
    pq->size++; //Atualiza contador 
}

NODE* remove_lower(PRIORITY_QUEUE* pq) {
    if (pq->size == 0) return NULL;

    NODE* min = pq->data[0]; //Pega o primeiro elemento (sempre o menor)
    pq->size--;
    pq->data[0] = pq->data[pq->size]; //Pega o último e coloca no início  
    heapify_down(pq, 0); //Chama heapify_down para organizar
    //"Afunda" esse elemento até encontrar lugar certo
    return min; //Retorna o elemento
}

//Função de DEBUG, não aparece quando o código (main.c) é compilado.
/*void print_priority_queue(PRIORITY_QUEUE* pq) {
    if (pq == NULL || pq->size == 0) {
        printf("Fila vazia ou inexistente.\n");
        return;
    }

    printf("Conteúdo da fila de prioridade:\n");
    for (int i = 0; i < pq->size; i++) {
        NODE* current = pq->data[i];
        if (current->character >= 32 && current->character <= 126) {
            printf("'%c' (%d): freq = %d\n", current->character, current->character, current->frequency);
        } else {
            printf("'\\x%02x' (%d): freq = %d\n", (unsigned char)current->character, current->character, current->frequency);
        }
    }
}*/

void free_priority_queue(PRIORITY_QUEUE* pq) {
    if (pq == NULL) return;

    for (int i = 0; i < pq->size; i++) {
        free_huffman_tree(pq->data[i]); // Libera cada nó na fila
    }
    free(pq); // Liberte a estrutura da fila
}


#endif // PQUEUE_HEAP_H


/*
Opção 1 - Compactar:


FASE 1: PREPARAÇÃO 

main.c → opção 1
     ↓
PRIORITY_QUEUE* huff_queue1 = create_queue()
     ↓
// create_queue() aloca memória:
PRIORITY_QUEUE* pq = malloc(sizeof(PRIORITY_QUEUE))
pq->size = 0
Retorna: [data: [NULL, NULL, ...], size: 0]
     ↓
PRIORITY_QUEUE* huff_queue2 = create_queue()
// Cria segunda fila IDÊNTICA


FASE 2: ANÁLISE DE FREQUÊNCIA 

main.c → create_huff_queue(original_file, &huff_queue1, &huff_queue2)
     ↓
Para cada caractere no arquivo:
     ↓
NODE* node = create_node(i, freq[i], NULL, NULL)
// create_node() aloca: [character: 'A', frequency: 5, left: NULL, right: NULL]
     ↓
insert(*pq1, node) e insert(*pq2, node)
     ↓
Dentro do insert():
1. pq->data[pq->size] = node  // Coloca no final
2. heapify_up(pq, pq->size)   // Reorganiza
3. pq->size++                 // Aumenta tamanho


FASE 3: CONSTRUÇÃO DA ÁRVORE HUFFMAN 
text
main.c → build_huffman_tree(huff_queue1)
     ↓
Enquanto huff_queue1->size > 1:
     ↓
NODE* left = remove_lower(huff_queue1)
// remove_lower():
// 1. NODE* min = pq->data[0]  // Pega o menor (primeiro)
// 2. pq->data[0] = pq->data[pq->size-1]  // Último vai pro início
// 3. pq->size--
// 4. heapify_down(pq, 0)  // Reorganiza
     ↓
NODE* right = remove_lower(huff_queue1)  // Segundo menor
     ↓
NODE* parent = create_node('\0', left->freq + right->freq, left, right)
     ↓  
insert(huff_queue1, parent)  // Volta para a fila
// REPETE até sobrar 1 nó (a raiz)


FASE 4: USO DA SEGUNDA FILA 

main.c → write_header(huff_queue2, huff_table, new_file, root)
     ↓
Dentro write_header → calculate_bits_trashed(huff_queue2, huff_table)
     ↓
Enquanto huff_queue2->size > 0:
     ↓
NODE* node = remove_lower(huff_queue2)  // Usa a fila INTACTA!
bit_amount += node->frequency * huff_table[node->character].


FASE 5: LIMPEZA 
text
main.c → free_priority_queue(huff_queue1)
     ↓
Dentro free_priority_queue:
for (i = 0; i < pq->size; i++)
    free_huffman_tree(pq->data[i])  // Libera cada nó
free(pq)  // Libera a estrutura da fila
*/

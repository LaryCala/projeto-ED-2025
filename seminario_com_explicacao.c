#include <stdio.h>
#include <stdlib.h>

typedef struct {
    float *dados;     
    float *segmento; 
    int n;         
} ArvoreSegmentos;

ArvoreSegmentos* inicializar(float *valores, int n) {
    ArvoreSegmentos *a = malloc(sizeof(ArvoreSegmentos));
    /*malloc(sizeof(ArvoreSegmentos)): Aloca memória para a estrutura que contém:
    
    dados (ponteiro) 
    segmento (ponteiro)
    n (inteiro)*/

    a->n = n; //Armazena o número de elementos do array original
    a->dados = malloc(n * sizeof(float));
    /*Propósito: Manter uma cópia dos valores originais
    Tamanho: n × sizeof(float) = 6 × 4 bytes = 24 bytes
    Conteúdo: [7.5, 8.0, 6.0, 9.0, 5.5, 8.5]*/

    a->segmento = malloc(4 * n * sizeof(float));
    /*Por que 4×n?
    
    Uma árvore de segmentos binária completa com n folhas precisa de no máximo 4×n nós
    
    Cálculo
    :
    Folhas: n nós
    Níveis internos: ~n-1 nós
    Total: ~2n-1 nós, mas arredondamos para 4n por segurança
    
    No exemplo: 4 × 6 × 4 bytes = 96 bytes
    
    O tamanho 4×n é conservador - garante que não teremos overflow*/


    for (int i = 0; i < n; i++)
        a->dados[i] = valores[i];
        
        /*Copia cada elemento do array de entrada para o array interno
        
        Importante: Mantém uma cópia independente dos dados originais
        
        Dois arrays separados:
        dados: mantém os valores originais (para referência)
        segmento: armazena a árvore de segmentos construída*/


    return a;
    /*Retorna o ponteiro para a estrutura alocada
    A árvore ainda não está construída - só a memória está preparada*/
}

/*Após inicializar, temos:

ArvoreSegmentos *a = {
    n: 6,
    dados: [7.5, 8.0, 6.0, 9.0, 5.5, 8.5],  // Cópia dos valores
    segmento: [?, ?, ?, ?, ?, ?, ?, ?, ...]  // 24 posições vazias
};*/

void montar(ArvoreSegmentos *a, int indice, int inicio, int fim) {
    /*
    Parâmetros:
    
    a: Ponteiro para a estrutura da árvore
    indice: Posição atual no array segmento
    inicio, fim: Intervalo do array original que este nó representa*/

    if (inicio == fim) {
        a->segmento[indice] = a->dados[inicio];
        return;
        /*
        Quando acontece: Quando o intervalo contém apenas um elemento
        O que faz: Copia o valor do array original para a árvore*/
    }

    int meio = (inicio + fim) / 2; //Divide o intervalo atual em duas metades
    montar(a, 2 * indice + 1, inicio, meio);
    montar(a, 2 * indice + 2, meio + 1, fim);
    /*Cálculo dos filhos:
    
    Filho esquerdo: 2 * indice + 1
    Filho direito: 2 * indice + 2*/

    a->segmento[indice] = a->segmento[2 * indice + 1] + a->segmento[2 * indice + 2]; //O nó pai armazena a soma dos filhos
} 
/*void montar(ArvoreSegmentos *a, int indice, int inicio, int fim)

Abordagem "Bottom-Up": Começa nas folhas e sobe calculando os pais
Divisão e Conquista: Divide o problema em subproblemas menores
Completa a árvore em O(n): Cada nó é processado exatamente uma vez*/

float soma_intervalo(ArvoreSegmentos *a, int indice, int inicio, int fim, int l, int r) {
    /*Parâmetros:
    
    a: Ponteiro para a estrutura da árvore
    indice: Posição atual no array segmento
    inicio, fim: Intervalo que este nó representa
    l, r: Intervalo desejado da consulta*/

    if (r < inicio || l > fim) //O nó atual está completamente fora do intervalo desejado
        return 0;
    if (l <= inicio && fim <= r) //O nó atual está completamente dentro do intervalo desejado
        return a->segmento[indice];

    int meio = (inicio + fim) / 2; // O nó atual está parcialmente dentro da consulta, então divide e busca nos dois filhos
    float esquerda = soma_intervalo(a, 2 * indice + 1, inicio, meio, l, r);
    float direita = soma_intervalo(a, 2 * indice + 2, meio + 1, fim, l, r);

    return esquerda + direita;
}

/*Comparação com Força Bruta:

Força bruta: Sempre percorre todos os elementos do intervalo → O(k) onde k = tamanho do intervalo
Árvore de segmentos: No máximo 4 nós por nível da árvore → O(log n)

Estratégia Inteligente:
Evita navegar em folhas individuais quando possível
Reutiliza cálculos prévios dos nós internos  
Sempre para quando encontra um nó completamente dentro do intervalo

Complexidade: O(log n)
A função visita no máximo 4 nós por nível da árvore, e a altura é log n!*/

void modificar(ArvoreSegmentos *a, int indice, int inicio, int fim, int pos, float novo_valor) {

    /*Parâmetros:
    
    a: Ponteiro para a estrutura da árvore
    indice: Posição atual no array segmento
    inicio, fim: Intervalo que este nó representa
    pos: Posição no array original a ser modificada
    novo_valor: Novo valor a ser atribuído
    */

    if (inicio == fim) { //Chegamos na folha que corresponde à posição pos
        a->segmento[indice] = novo_valor; //Atualiza o valor na árvore 
        a->dados[pos] = novo_valor; //Atualiza o valor no array original 
        //Importante: Atualiza ambos para manter consistência!
        return; 
    }

    int meio = (inicio + fim) / 2; //Decide para qual filho navegar baseado na posição pos
    if (pos <= meio)  //Filho esquerdo: Se pos está na primeira metade do intervalo
        modificar(a, 2 * indice + 1, inicio, meio, pos, novo_valor);
    else //Filho direito: Se pos está na segunda metade
        modificar(a, 2 * indice + 2, meio + 1, fim, pos, novo_valor);

    a->segmento[indice] = a->segmento[2 * indice + 1] + a->segmento[2 * indice + 2]; //Recalcula o valor do nó atual após a atualização dos filhos
    //Importante: Isso acontece na volta da recursão, garantindo que todos os ancestais sejam atualizados
}

/*Por que é Eficiente?

Comparação com Força Bruta:

Força bruta: Atualiza o array, depois recalcula todas as somas → O(n)
Árvore de segmentos: Atualiza apenas os nós no caminho da folha até a raiz → O(log n)

Apenas 3 nós foram atualizados em vez de recalcular toda a árvore!

Características Importantes

Bottom-Up: Começa na folha e sobe atualizando os pais
Caminho Único: Só visita os nós no caminho da raiz até a folha desejada
Consistência: Mantém tanto dados quanto segmento atualizados
Propagação Automática: A soma é recalculada para todos os ancestais

Complexidade: O(log n)

A função visita exatamente um nó por nível da árvore!
*/

int main() {
    
    float notas[] = {7.5, 8.0, 6.0, 9.0, 5.5, 8.5};
    int n = 6;

    ArvoreSegmentos *a = inicializar(notas, n);
    montar(a, 0, 0, n - 1);

    float soma = soma_intervalo(a, 0, 0, n - 1, 1, 3);
    printf("Soma das notas dos alunos 1 a 3: %.1f\n", soma);
    printf("Media das notas [1..3]: %.2f\n", soma / 3.0);

    modificar(a, 0, 0, n - 1, 2, 9.5);
    printf("\nApos atualizar a nota do aluno 3:\n");
    soma = soma_intervalo(a, 0, 0, n - 1, 1, 3);
    printf("Soma das notas [1..3]: %.1f\n", soma);
    printf("Media das notas [1..3]: %.2f\n", soma / 3.0);

    free(a->dados);
    free(a->segmento);
    free(a);


    return 0;
}

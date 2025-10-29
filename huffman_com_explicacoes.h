#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "pqueue_heap.h"

#define BUFFER_SIZE 1024

//Lê o arquivo e conta quantas vezes cada byte aparece, cria nós e insere nas DUAS filas
void create_huff_queue(FILE *input_file, PRIORITY_QUEUE** pq1, PRIORITY_QUEUE** pq2) {
  /*
  FILE *input_file - Onde aponta?

  input_file aponta para o arquivo que está sendo compactado
  É o mesmo original_file do main.c que você abriu com fopen() 
  Exemplo: Se compactando "foto.jpg", aponta para esse arquivo.

  Ponteiros duplos em PRIORITY_QUEUE** pq1 e PRIORITY_QUEUE** pq2, pq são importantes?

  Sem eles só muda a copia local, no main a fica contiuaria NULL. 
  Usando dois ponteiros mudamos também o ponteiro original no main.
  SÓ MUDA A CÓPIA LOCAL!
  */

    int freq[256] = {0}; //array para salvar as frequencias de bytes que aparece no arquivo lido, inicializa TODOS os elementos com ZERO evitando lixo de memória e contagens erradas.
    //Pq tamanho 256? Pq o unsigned char vai de 0 a 255 → 256 valores

    unsigned char c; 
    /*Pq usar um "caractere sem sinal"?
    
    O char "normal":
    char c;  Pode ser -128 a 127 (COM sinal)
    Se ler byte 255 → vira -1 → freq[-1] ← ACESSO INVÁLIDO, fora do intervalo 0 - 255 de freq, tenta acessar uma memoria ANTES do inicio do array

    Com o unsigned char:
    unsigned char c; SEMPRE 0 a 255 (SEM sinal)
    Qualquer byte lido fica entre 0-255 → freq[c] sempre válido 
    */

    //Conta a frequencia de caracteres
    while (fread(&c, 1, 1, input_file) == 1) { 
      
      /* Função fread(&c <- Onde guardar (endereço de c), 1 <- Tamanho de CADA elemento (1 byte), 1 <- Quantos elementos ler (1 elemento), input_file <- Arquivo de onde ler)
      
      Condição: == 1) <- Enquanto conseguir ler 1 elemento
      Retorna 1: Leu 1 elemento com sucesso → continua
      Retorna 0: Fim do arquivo ou erro → para
      */

        freq[c]++; /*quando um caracter é mandado como indice, esse caracter é transformado em decimal da tabela ASCII, assim toda vez que chega o mesmo caracter é incrementado um na posição correspondente
        
        Exemplo:
        Quando leio byte 65 ('A'):
        freq[65]++  → Incrementa a posição 65 do array
        
        Quando leio byte 66 ('B'):  
        freq[66]++  → Incrementa a posição 66
        
        Quando leio byte 65 ('A') de novo:
        freq[65]++  → Agora freq[65] = 2
        
        Assim:
        freq[65] = 2  // 'A' apareceu 2 vezes
        freq[66] = 1  // 'B' apareceu 1 vez
        freq[67] = 0  // 'C' nunca apareceu
        ...etc
        */

    }

    // Cria nós para caracteres com frequência diferente de zero e os insere em ambas as filas
      for (int i = 0; i < 256; i++) {
          if (freq[i] > 0) { //enquanto a frequencia for maior que zero, continua .
              NODE* node = create_node(i, freq[i], NULL, NULL); /*Cria um ponteiro do tipo node que nele vai ter a frequencia, e seus filhos apontam pra nulo. Essa função está em pqueue_heap.h

              // Exemplo: i=65, freq[65]=2
              NODE* node = create_node(65, 2, NULL, NULL);
              
              // Dentro de create_node (Em pqueue_heap.h):
              NODE* node = (NODE*)malloc(sizeof(NODE));
              node->character = 65;     // Byte 'A'
              node->frequency = 2;      // Apareceu 2 vezes  
              node->left = NULL;        // Ainda não tem filhos
              node->right = NULL;       // Ainda não tem filhos
              */

              insert(*pq1, node); //insere esse nó na fila de prioridade 1 (heap 1)
              insert(*pq2, node); //insere esse nó na fila de prioridade 2 (heap 2)
          }
      }
}

// Constrói a árvore de Huffman a partir da fila de prioridade
NODE* build_huffman_tree(PRIORITY_QUEUE* pq) {
    while (pq->size > 1) { //enquanto o tamanho da minha heap for > 1  continuo 
        NODE* left = remove_lower(pq); // removo o menor nó da heap e esse sera o no filho a esquerda 
        NODE* right = remove_lower(pq); // removo o  segundo menor nó da heap e esse sera o no filho a direita 

        // Create a parent node with the left and right children
        NODE* parent = create_node('\0', left->frequency + right->frequency, left, right); /*cria um no "pai" com caracter "nulo" , sua freq sera a soma das duas menores frequencias dos respectivos no removido acima.
        Seus filhos será o nó com menor freq e o de segunda menor freq*/

        insert(pq, parent); //Insere o novo nó pai na fila de prioridade (heap) agora com sua frequencia sendo a soma das frequencia dos filhos (isso mudará sua posição na heap)
    }

    //O ultimo nó que sobra é a raiz da arvore de huffman
    return remove_lower(pq); //Agora o size é = 1 , significa o ultimo nó, o nó pai com menor frequencia, retornamos ele para ser usado na proxima etapa - criação da tabela de codigo.
}

// Define a estrutura da tabela de Huffman com os códigos binários dos caracteres
typedef struct {
    uint32_t code; 
    /*uint32_t = unsigned int de 32 bits (0 a 4.294.967.295)
    code = O código Huffman em binário
    
    Exemplo: Se código for 101, code = 5 (em decimal)
    */
    int length; //comprimento 
} HuffmanCode;

void create_huffman_table(NODE* root, uint32_t code, int depth, HuffmanCode huff_table[256]) {

  /*
  De onde vêm na PRIMEIRA chamada (do main):

  No main.c:
  HuffmanCode huff_table[256] = {0};  // Tabela vazia
  uint32_t code = 0;                  // Código inicial: 0
  create_huffman_table(root, code, 0, huff_table);
  //                ↑      ↑     ↑        ↑
  //               raiz   code=0 depth=0 tabela_vazia
 
  O que cada parâmetro significa:
  root: Raiz da árvore de Huffman (vem do build_huffman_tree())

  No main.c:
  NODE* root = build_huffman_tree(huff_queue1);
  // ↑ root AGORA aponta para a raiz da árvore

  O que build_huffman_tree() retorna:
  return remove_lower(pq);  ← Retorna a RAIZ da árvore

  code: Código binário atual (começa com 0)
  depth: Profundidade atual na árvore (começa com 0)
  huff_table: Array onde salvar os códigos finais
  */

    if (!root) return; //O mesmo que root == NULL, evita de ta lendo no vazio (arvore vazia)

    if (!root->left && !root->right) {   //verifica se o nó  atual é uma folha. 
        huff_table[root->character].code = code; //salva o numero em binario do caracter correspondente no array 
        huff_table[root->character].length = depth; //a profundidade serve para mostrar quantos bits a esquerda sao mais significativo (quanto maior a profundidade mais bit para representar o caracter)
        //depth = Quantos bits tem o código desse caractere

        /*
        Exemplo, temos a árvore:
  
            Raiz
           /    \
         A(0)   Pai
               /    \
             B(10)  R(11)

        Para 'A': depth = 1 (código: 0 → 1 bit)
        Para 'B': depth = 2 (código: 10 → 2 bits)   
        Para 'R': depth = 2 (código: 11 → 2 bits)*/
        return;
    }
    //Atraves da arvore de huffman criada (build_huffman_tree), ela vai andando por ela e criando a tabela que seria nosso novo ASCII 


    create_huffman_table(root->left, (code << 1), depth + 1, huff_table); //chama recurssivamente para sub arvore a esquerda, setando 0 no caminho da profundidade
    create_huffman_table(root->right, (code << 1) | 1, depth + 1, huff_table); //chama recurssivamente para sub arvore a direita, setando 1 no caminho da profundidade 

    //Quem guarda os 0's e 1's? A tabela Huffman (huff_table) guarda!

}

//Calcula quantos bits totais serão escritos no corpo compactado
int calculate_bits_trashed(PRIORITY_QUEUE* pq, HuffmanCode huff_table[256]) {
    int bit_amount = 0; //quantidade de bit
    //vai removendo nó a nó e multiplicando a frequencia de caracter no arquivo pela quantidade de bit mais significativo (quantas vezes se repete)
    while (pq->size > 0) {
        NODE* node = remove_lower(pq); 
        if (node && huff_table[node->character].length > 0) { 
          //verifica se node não é NULL e se este caractere tem um código Huffman válido
          // length > 0 significa: tem um código gerado na tabela
          // length == 0 significa: caractere não apareceu ou não tem código
            bit_amount += node->frequency * huff_table[node->character].length;
            //bits_totais = soma( frequência × tamanho_do_código )
            /*Para saber quantos bits de "lixo" teremos no último byte:
            
            Exemplo:
            int total_bits = 6;
            int trash = (8 - (6 % 8)) % 8;  // 8 - 6 = 2 bits de lixo*/
            
        }
    }

    return bit_amount; //retorna a quantidade de bit
}

// Conta o número de nós da árvore (para o tamanho da árvore no cabeçalho)
int count_tree_size(NODE* root) {
    if (!root) return 0; //checa se o nó é vazio
    return count_tree_size(root->left) + count_tree_size(root->right) + 1; // chamada recussiva para nos da esquerda e direta, fazendo a soma dos mesmo

    /*
    Exemplo:
      Raiz
    /    \
    A      Pai
      /    \
      B     R

    count_tree_size(Raiz)
    = count_tree_size(A) + count_tree_size(Pai) + 1
    = (0+0+1) + (count_tree_size(B) + count_tree_size(R) + 1) + 1  
    = 1 + ((0+0+1) + (0+0+1) + 1) + 1
    = 1 + (1 + 1 + 1) + 1
    = 1 + 3 + 1
    = 5 nós no total!
    
    No cabeçalho do arquivo .huff, precisamos saber quantos bytes a árvore serializada vai ocupar.
    */
}

//Verifica se os dois filhos são nulos → se sim, é uma folha.
int is_leaf(NODE* node) {
  return node && !node->left && !node->right; // se for folha retorna 1 (true) se nao , 0(false)
    /*node → Verifica se o ponteiro não é NULL, assim não dá erro se tentar acessar node->left ou node->right de um ponteiro NUll
    !node->left → Verifica se o filho esquerdo é NULL
    !node->right → Verifica se o filho direito é NULL
    */
} 

// Escreve a árvore de Huffman no arquivo de saída em PRE ORDEM = Raiz → Esquerda → Direita 
void write_tree(NODE* root, FILE* output_file) {
  if (is_leaf(root)) { //verifica se é folha (nao tem filhos)
    //Nó folha foi encontrado
    fputc('1', output_file); // Marca folha
    //fputc: Escreve UM caractere em um arquivo, sendo '1' caracter a ser escrito e output_file o arquivo onde escrever
  
    if (root->character == '*' || root->character == '\\') { //se o caracter lido foi * ou \\ 
      fputc('\\', output_file);
      /*
      Caractere * → escreve \*
      Caractere \ → escreve \\
      
      Na descompactação: \ significa "o próximo caractere é literal"
      */
    }
    fputc(root->character, output_file); //escreve o caracter no arquivo de saida 
    // FOLHA: escreve e PARA (sem recursão)
    // Porque folhas não têm filhos para visitar!

  } else {
    //Nao é um nó folha, é um nó interno 
    fputc('0', output_file); 
    // NÓ INTERNO: escreve marcação E continua recursão
    // Porque precisa visitar os filhos!
    write_tree(root->left, output_file); //chamada recursiva para o filho a esquerda 
    write_tree(root->right, output_file);  //chamada recursiva para o filho a direita 
  }
}

// Escreve o cabeçalho no novo arquivo (lixo, tamanho da árvore, árvore)
// Escreve a árvore de Huffman codificada no cabeçalho do arquivo compactado, bit a bit
void write_header(PRIORITY_QUEUE* pq, HuffmanCode huff_table[256], FILE *output_file, NODE* root) {

    int total_bits = calculate_bits_trashed(pq, huff_table); //Calcula o total de bits que serão usados para escrever os dados compactados. 
    int trash = ((8 - (total_bits % 8)) % 8); //Calcula quantos bits finais do último byte vão ser "lixo"
    // Se sobrar, digamos, 5 bits, precisamos preencher 3 bits com 0s → então trash = 3.
    /*
    1. total_bits % 8 = 13 % 8 = 5
   → Precisamos de 5 bits no último byte
   
   2. 8 - 5 = 3  
      → Faltam 3 bits para completar o byte
   
   3. (8 - 5) % 8 = 3 % 8 = 3
      → trash = 3 bits de preenchimento*/

    int tree_size = count_tree_size(root);//Tamanho da árvore de Huffman (em bytes): necessário para a descompactação.

    /*O cabeçalho tem 16 bits com a seguinte estrutura:
    [3 bits de lixo][13 bits de tamanho da árvore]*/
    unsigned short header = (trash << 13) | tree_size; //short = Tipo de dado menor que int (normalmente 16 bits)
    /*Exemplo: trash=3, tree_size=25
    trash = 3      <- binário: 011
    tree_size = 25 <- binário: 0000000011001
    
    (trash << 13) = 0110000000000000  // Lixo nos 3 primeiros bits
    tree_size     = 0000000000011001  // Tamanho nos 13 últimos
    
    (trash << 13) | tree_size = 0110000000011001  // Junta tudo!
    */

    //Precisamos escrever 2 bytes no arquivo, não 16 bits direto. Por isso separamos em dois:
    unsigned char byte1 = header >> 8; // pega os 8 a esquerda, Shift Right (deslocamento para direita)
    /*header:    01100000 00011001
      >> 8:      00000000 01100000  ← byte1 agora está na posição certa*/

    unsigned char byte2 = header & 0xFF; // pega os 8 a direita 
    /* header & 0xFF - AND bit a bit

    O que acontece:
    
    header: 01100000 00011001
    & 0xFF: 00000000 11111111
    result: 00000000 00011001  ← Só passa os 8 bits da direita
    byte2 = 00011001 = 25
    */

    //os dois bytes lidos acima serão os primeiros dois bytes do arquivo compactado
    fwrite(&byte1, 1, 1, output_file); //fwrite: Escrever dados em binários
    fwrite(&byte2, 1, 1, output_file); 

    /*fwrite(&byte1,    // Onde estão os dados (endereço de byte1)
       1,         // Tamanho de CADA elemento (1 byte)
       1,         // Quantos elementos escrever (1 elemento)
       output_file) // Arquivo onde escrever
    */

    write_tree(root, output_file); //Depois de escrever os dois bytes agora chama write_tree para escrever a arvore em PRE ORDEM 
}

// Estrutura para armazenar bits até completar um byte
//vai acumulando bits um a um até formar um byte completo (8 bits)
//É para a COMPACTAÇÃO dos DADOS do arquivo original
typedef struct {
    unsigned char byte;
    int bits_used;
} BitBuffer;

// adiciona um novo bit ao buffer.
void bit_buffer_add(BitBuffer *bit_buffer, int bit) {
    bit_buffer->byte <<= 1; /*Move todos os bits atuais do buffer uma posição à esquerda.
Isso abre espaço no final (bit menos significativo) para o novo bit. -> 00000001 -> 00000010*/

    bit_buffer->byte |= (bit & 1); //OR bit a bit com o novo bit, garantindo que ele entre no final.
    /*(bit & 1):
    Isso isola apenas o bit menos significativo (o último) do valor bit.
    Se bit = 0, então 0 & 1 = 0.*/
    //depois faz um OR 

    bit_buffer->bits_used++; //Aumenta o contador de bits já utilizados no byte.
}

//Escreve o conteúdo parcial do BitBuffer no arquivo, preenchendo com zeros os bits restantes até formar 8 bits.
void write_buffer(FILE *f, BitBuffer *bit_buffer) { 
    if (bit_buffer->bits_used == 0) return;//Se o buffer estiver vazio, não há nada para escrever.

    //caso em que o byte nao tem 8 bits 
    unsigned char temp_byte = bit_buffer->byte << (8 - bit_buffer->bits_used); //ele move os bits usados para a esquerda, para “encostar” os bits no início do byte.
    /*bit_buffer->byte = 000101 (apenas 5 bits usados).
    bit_buffer->bits_used = 5

O deslocamento será 8 - 5 = 3
    Resultado: 000101 << 3 = 101000 → os bits ocupam a parte mais significativa do byte.
    Isso garante que a estrutura do cabeçalho fique correta, pois o "lixo" (bits não usados) está na parte menos significativa.*/

    fwrite(&temp_byte, 1, 1, f); //fwrite : escreve 1 byte no arquivo.
    //usa a variável temporária temp_byte, que contém o conteúdo alinhado.

    //zera para a proxima vez que chamar 
    bit_buffer->byte = 0;   
    bit_buffer->bits_used = 0;
}

// Escreve os dados compactados no novo arquivo
void compactor(FILE *input_file, FILE *output_file, HuffmanCode huff_table[256]) {
    unsigned char c;
    BitBuffer bit_buffer = {0, 0}; //Inicializa bit_buffer

    // Lê cada caractere do arquivo de entrada
    while (fread(&c, 1, 1, input_file) == 1) {
        // 1. Pega o código Huffman desse byte
        HuffmanCode code = huff_table[c];

        // 2. Adiciona cada bit do código Huffman ao buffer (do mais significativo pro menos)
        for (int i = code.length - 1; i >= 0; i--) {
        // Pega o bit MAIS SIGNIFICATIVO primeiro
        // e vai até o bit MENOS SIGNIFICATIVO

        /*Exemplo Prático:
        Se temos um código Huffman:
        
        code.code = 5 (binário: 101)
        code.length = 3 (3 bits)
        
        O loop vai:
        i = 2: pega o bit na posição 2 (mais à esquerda)
        i = 1: pega o bit na posição 1  
        i = 0: pega o bit na posição 0 (mais à direita)
        */

            // 3. Pega o bit atual: (code.code >> i) & 1
            bit_buffer_add(&bit_buffer, (code.code >> i) & 1);
            
            // 4. Se buffer cheio, escreve byte(8 bits)
            if (bit_buffer.bits_used == 8) { 
                write_buffer(output_file, &bit_buffer);
            }
        }
    }

    // 5. Escreve bits que sobraram (último byte incompleto)
    write_buffer(output_file, &bit_buffer);
}

/*
// 1. PRIMEIRO: Prepara tudo
create_huff_queue(original_file, &huff_queue1, &huff_queue2);
NODE* root = build_huffman_tree(huff_queue1);
create_huffman_table(root, code, 0, huff_table);

// 2. DEPOIS: Escreve cabeçalho + árvore
write_header(huff_queue2, huff_table, new_file, root);

// 3. FINALMENTE: Compacta os dados (ESSA função!)
rewind(original_file);  // Volta ao início do arquivo
compactor(original_file, new_file, huff_table);  // ← AQUI!
*/

void free_huffman_tree(NODE* root) {
    if (root == NULL) return;

    free_huffman_tree(root->left);
    free_huffman_tree(root->right);
    free(root);
}



/*
    FUNÇÕES PARA DESCOMPACTAR O ARQUIVO
*/

// Lê os dois primeiros bytes do cabeçalho e extrai lixo e tamanho da árvore
void read_header(FILE *file, int *trash, int *tree_size) { //recebe o ponteiro para o arquivo , o tamanho do lixo , e o tamanho da arvore 
    unsigned char byte1, byte2;
    //Lê os dois primeiros bytes do arquivo:
    // fread (ponteiro para o buffer , tamanho de cada elemento a ser lido em byte, numero de elementos a ser lido , ponteiro para o arquivo )
    fread(&byte1, 1, 1, file); //fread : ler arquivo em bytes 
    fread(&byte2, 1, 1, file);

    //usigned é um inteiro sem sinal e short é uma variavel menor que int (tamanho)
    /*calculando a soma dos bytes 
    byte 1 =  0000000011111111 ; byte 2 = 0000000010101010
    (byte << 8) = 1111111100000000 -> quando faz o | (fazendo o OR) junta byte 1 com o byte 2 e tem o os 16bytes do cabeçalho*/
    unsigned short header = (byte1 << 8) | byte2; //'|' é OR (ou)
    /*byte1 = 01100000, byte2 = 00011001
    
    byte1 << 8:  01100000 00000000  ← byte1 vai para esquerda
    byte2:     01100000 00000000 | 00000000 00011001 = 01100000 00011001*/

    *trash = header >> 13; // faz os shitf pra direita e pega somente o lixo
    *tree_size = header & 0x1FFF; 
    //0x1FFF é um número em hexadecimal, e corresponde a 8191 em decimal, ou 0001 1111 1111 1111 em binário (13 bits ligados em 1).OS 13 bits de tamanho da árvore
    /*Exemplo:
    header:   0110000000011001
    & (AND)
    0x1FFF:   0001111111111111
    result:   0000000000011001 = 25  ← Só passou os 13 bits da direita!*/
}

// Lê a árvore codificada no arquivo compactado e reconstrói a árvore de Huffman a partir da representação em pré-ordem que está gravada no arquivo logo após os 2 bytes do cabeçalho.
NODE* read_tree(FILE *file, int *bytes_read) { 
    // c sera 1 ou 0 (tipo do NO)
    int c= fgetc(file);//fgetc : ler um único caractere de um arquivo
    (*bytes_read)++; //conta quantos bytes da árvore já foram lidos

    if (c == '1') { //Se for '1': é uma folha (chegamos a um caracter da arvore)
        int next = fgetc(file);  //Ler o proximo caracter do arquivo 
        (*bytes_read)++; //Conta +1 byte lido 

        if (next == '\\') { //Considerando o caso em que o caracter de "escape"
            next = fgetc(file);
            (*bytes_read)++;
        }

        return create_node((unsigned char)next, 0, NULL, NULL); //Cria o nó, nosso caracter 

    } else if (c == '0') { //Se for '0': é um nó interno ( um no com nada que aponta pro filhos)
        //chamada recurssiva para filhos 
        NODE *left = read_tree(file, bytes_read); // chama filho pra esquerda 
        NODE *right = read_tree(file, bytes_read); // chama filho para direita 
        return create_node('\0', 0, left, right); // cria no interno 
    }

    return NULL; //quando fgetc nao retorna um caracter (evita dados invalidos)
    /*O que acontece se fgetc falhar?

    int c = fgetc(file);  // Pode retornar EOF (End Of File) em caso de erro
    Situações onde fgetc retorna EOF:

    Fim do arquivo inesperado (arquivo corrompido)
    Erro de leitura (disco com problema)
    Dados inválidos na serialização

    Com return NULL: A função retorna NULL indicando "não consegui reconstruir"
    */
}

// Percorre os bits do corpo compactado e escreve os caracteres no arquivo de saída
void decompress(FILE *input, FILE *output, NODE* root, int trash_size, int header_bytes) {
    // input: arquivo compactado (.huff) para ler
    // output: arquivo descompactado para escrever  
    // root: raiz da árvore de Huffman reconstruída
    // trash_size: quantos bits ignorar no último byte (0-7)
    // header_bytes: quantos bytes pular (cabeçalho + árvore)

    fseek(input, 0, SEEK_END); // Vai para o FINAL do arquivo
    //fseek(arquivo, posição, origem -> SEEK_END = final do arquivo); 
    //fseek : Move o "cursor" de leitura/escrita dentro do arquivo, como se você escolhesse onde começar a ler ou escrever
    long file_size = ftell(input); // Pega quantos bytes tem o arquivo
    //ftell : Retorna a posição atual do cursor (em bytes a partir do início do arquivo). É um jeito de descobrir o tamanho total do arquivo.
    long data_size = file_size - header_bytes; // Calcula tamanho dos dados
    //data_size = tamanho do arquivo - tamanho do cabeçalho 
    fseek(input, header_bytes, SEEK_SET); // Volta para INÍCIO dos dados
     // Vai para header_bytes depois do INÍCIO -> SEEK_SET = início do arquivo

    NODE* current = root; // Começa na raiz da árvore
    unsigned char byte; //Começa a ler byte a byte da parte compactada.

    for (long i = 0; i < data_size; i++) { //cada byte nos dados
        fread(&byte, 1, 1, input);//ler o byte atual
        /* Função fread(&byte <- Onde guardar (endereço de byte), 1 <- Tamanho de CADA elemento (1 byte), 1 <- Quantos elementos ler (1 elemento), input <- Arquivo de onde ler)*/
        

        for (int bit = 7; bit >= 0; bit--) { //cada bit no byte (do mais pro menos significativo)

            int current_bit = (byte >> bit) & 1; // bit_atual = shift rigth do byte, bit vezes, e seta ele fazendo um AND 1

            /*Exemplo:
            byte = 173 (10101101):

            bit=7: 10101101 >> 7 = 00000001 & 1 = 1
            bit=6: 10101101 >> 6 = 00000010 & 1 = 0  
            bit=5: 10101101 >> 5 = 00000101 & 1 = 1
            bit=4: 10101101 >> 4 = 00001010 & 1 = 0
            bit=3: 10101101 >> 3 = 00010101 & 1 = 1
            bit=2: 10101101 >> 2 = 00101011 & 1 = 1
            bit=1: 10101101 >> 1 = 01010110 & 1 = 0
            bit=0: 10101101 >> 0 = 10101101 & 1 = 1

            Resultado: 1 0 1 0 1 1 0 1 (exatamente os bits do byte!) 
            
            Assim cada bit é analisado, um de cada vez.
            */

            if (current_bit == 0) // para cada bit , vai ser 0 para o filho a esquerda 
                current = current->left;
            else
                current = current->right; // 1 para o filho a direita 

            if (is_leaf(current)) { //checa se chegamos numa folha (se chegou numa folha )
                // (caracter a ser escrito, ponteiro para o arquivo)
                fputc(current->character, output); //fputc : escreve um único caractere em um arquivo (escreve o caractere no arquivo de saída.)
                current = root; //Reinicia o ponteiro na raiz da árvore para continuar.
            }

            // Evita processar bits de "lixo" no último byte
            // Para no ÚLTIMO byte (i == data_size - 1) 
            // no ÚLTIMO bit válido (bit == trash_size - 1)
            if (i == data_size - 1 && bit == trash_size - 1) { //os ultimos bits é o lixo e nao deve ser processado 
                return;//A função termina assim que os bits válidos acabam, evitando decodificar lixo.
            }
        }
    }
}

// Função principal chamada na main
void decompact(const char* compressed_filename, char final_format[]) {
    // "const char*" = string constante (não pode ser modificada)
    // compressed_filename = nome do arquivo .huff para descompactar
    // final_format = extensão desejada para o arquivo final

    /*Quem fornece esses valores?
    Do main.c:
    
    printf("\nInsira o nome do arquivo compactado (.huff):\n");
    scanf("%s", compressed_filename);  // ← usuário digita
    
    printf("\nInsira o formato da extensao final (ex: jpg, txt, etc):\n");
    scanf("%s", final_format);         // ← usuário digita
    
    decompact(compressed_filename, final_format);  // ← Passa para a função
    */


    FILE *input_file = fopen(compressed_filename, "rb"); //abre o arquivo compactado 
    // fopen : abre arquivo em modo de leitura em binario"rb"
    
    if (!input_file) { //se falhar 
        perror("Erro ao abrir o arquivo compactado");
        return;
    }

    // Cria nome para o arquivo de saída
    char output_filename[BUFFER_SIZE]; 
    strcpy(output_filename, compressed_filename); 
    //strcpy(destino, origem);  <- Copia string origem para destino, de compressed_filename para output_filename
    char* dot = strrchr(output_filename, '.');  // strrchr : procura a última ocorrência do caractere '.' na string.
    if (dot) *dot = '\0';  // Remove a extensão atual
    /*dot → aponta para o caractere `'.'` antes de `"huff"`
     substitui esse ponto por '\0', o terminador nulo de string em C.
     Isso corta a string naquele ponto, removendo a extensão final.
     output_filename = "texto.txt";  // extensão .huff foi removida*/


    // Acrescenta o sufixo e a nova extensão ex.:descompactado.txt
   snprintf(output_filename, sizeof(output_filename), "%.*s_descompactado.%s", (int)(sizeof(output_filename) - strlen("_descompactado.") - strlen(final_format) - 1), output_filename,  final_format);
    //snprinf: Formata dados e os escreve em um buffer de caractere, similar à função printf, mas com a adição de um parâmetro de tamanho máximo para evitar buffer overflows. 
   //strlen - CONTADOR DE LETRAS

   /*Por que esse calculo?
   Evitar overflow!

   Exemplo:
   256 (tamanho máximo)
   - 15 ("_descompactado.") 
   - 3 ("jpg")
   - 1 ('\0')
   = 237 caracteres máximos para o nome base*/

    FILE *output_file = fopen(output_filename, "wb"); //abre arquivo em modo de escrita em binario"wb"
    if (!output_file) {
        perror("Erro ao criar arquivo de saída");
        fclose(input_file);
        return;
    }
    
    //Lê o cabeçalho     
    int trash_size = 0, tree_size = 0, bytes_read = 0;
    //tam lixo , tam arvore , byte lido :conta quantos bytes da árvore foram lidos (para saber onde começa o corpo compactado)
    read_header(input_file, &trash_size, &tree_size); //Lê o lixo e o tamanho da árvore
    NODE* root = read_tree(input_file, &bytes_read); //Reconstrói a árvore de Huffman a partir dos próximos tree_size bytes


    decompress(input_file, output_file, root, trash_size, 2 + bytes_read); //Descompacta o corpo usando a árvore
    /*
    O que 2 + bytes_read realmente significa:

    2 = bytes do CABEÇALHO
    Byte 1 + Byte 2 = 2 bytes com informações do lixo e tamanho da árvore
    
    bytes_read = bytes da ÁRVORE
    Quantos bytes foram lidos para reconstruir a árvore
    
    2 + bytes_read = TOTAL de bytes para PULAR

    [2 bytes: cabeçalho][X bytes: árvore][dados...]
     ↑                   ↑                ↑
    cabeçalho          árvore           dados começam AQUI

    = posição onde os dados compactados começam
    */

    printf("Arquivo descompactado com sucesso: %s\n", output_filename);

    free_huffman_tree(root);
    fclose(input_file);
    fclose(output_file);
}



#endif // HUFFMAN_H

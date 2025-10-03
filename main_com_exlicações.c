/*  Toda a implementação no está .h, o que é comum quando:
    Querem compilar rápido sem fazer Makefile
    São poucos arquivos
    É um projeto pequeno/educacional

    COMO ISSO FUNCIONA ATUALMENTE?
    O main.c faz:
    #include "huffman.h"    <- Pega TODO o código do Huffman
    #include "pqueue_heap.h"  <- Pega TODO o código da fila de prioridade
    
    Na compilação:
    gcc main.c -o programa  <- Só precisa do main.c!
    ↑ Como tudo está nos .h, o compilador vê tudo como um arquivo só!

    Isso permite compilar fácil, porém é mais lenta pq sempre recompila tudo a cada uso.
*/

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <stdint.h>  // Para uint32_t
// No huffamn os códigos podem ser longos, até 256 no pior caso, e o uint32_t garante que o código tenha bits suficientes para armazenar qualquer código e evita problemas entre sistemas (32-bit vs 64-bit)

#include "huffman.h"
#include "pqueue_heap.h"

#define BUFFER_SIZE 1024
//Buffer = Área de memória temporária para armazenar strings
//1024 significa 1024 caracteres de espaço. Isso permite nomes de arquivo longos
//1024 bytes = 1KB

int main() {
    int option;
    
    printf("====== Compactador Huffman ======\n");
    printf("Escolha uma opção:\n");
    printf("1 - Compactar arquivo\n");
    printf("2 - Descompactar arquivo\n");
    printf("Opção: ");
    scanf("%d", &option);
    getchar(); // Limpa o buffer do ENTER
    /*Usuário digita: "1" + [ENTER]
      scanf pega: "1"
      Buffer fica com: '\n' (ENTER)
      
      scanf("%s", filename); -> Iria ler o '\n' como string vazia!

      Solução:
      getchar(); -> Remove o '\n' do buffer
      Agora o próximo scanf funciona corretamente
    */


    if (option == 1) {
        printf("\nInsira o nome do arquivo a ser compactado, com a extensao:\n");

        char filename[BUFFER_SIZE];
        /* ↑ Cria um ARRAY de caracteres com 1024 posições
        É como: [ ][ ][ ][ ][ ][ ][ ]... (1024 espaços vazios)*/

        scanf("%s", filename);
        /*↑ Quando usuário digita "arquivo.txt"
           O buffer fica: ['a']['r']['q']['u']['i']['v']['o']['.']['t']['x']['t']['\0'][ ][ ][ ]...                                                                   ↑    ↑
           //                                                                      Fim  Espaços sobrando
           //
        */

        FILE* original_file = fopen(filename, "rb");
        //FILE é uma estrutura que representa um arquivo aberto
        //FILE* = "Controle remoto" do arquivo
        /*"rb" - Modo de Abertura
            "r" = Read (Leitura)
            "b" = Binary (Binário)
            "rb" = Abrir para leitura em modo binário  
            
            Por que binário no Huffman?

            Para TEXTO:
            FILE* f = fopen("arquivo.txt", "r"); -> Modo texto

            Para QUALQUER arquivo (imagens, zip, exe, etc):
            FILE* f = fopen("foto.jpg", "rb"); -> Modo binário ← NOSSO CASO!

            No Huffman: Como compactamos QUALQUER tipo de arquivo (imagens, vídeos, etc.), usamos "rb" para ler os bytes exatamente como estão, sem conversões.
        */

        if (original_file == NULL) {
            perror("Erro ao abrir o arquivo");
            return 1;
        }

        // Remove a extensão do nome do arquivo
        char base_name[BUFFER_SIZE];
        strcpy(base_name, filename);  // Copia, por exemplo, "foto.jpg" para base_name

        char* dot = strrchr(base_name, '.');  // Encontra o ÚLTIMO ponto
        if (dot != NULL) {
            *dot = '\0'; // Coloca fim de string no lugar do ponto
            /*dot é um ponteiro para a posição do ponto 
            \0 = caractere de fim de string em C
            A string agora termina no \0, ignorando tudo depois
            
            */
        }

        // Cria um novo nome com extensão .huff
        char new_file_name[1024];
        snprintf(new_file_name, sizeof(new_file_name), "%.*s.huff", 
        (int)(sizeof(new_file_name) - strlen(".huff") - 1), 
        base_name); 
        /*%.*s = Imprime uma string com limite de tamanho

        O cálculo (int)(sizeof(new_file_name) - strlen(".huff") - 1) tenta evitar overflow*/

        FILE *new_file = fopen(new_file_name, "wb"); //"wb" = Write Binary (Escrita Binária) → Criar novo arquivo em binário

        /*
        Se usasse "w" (texto):
        fputc(0x0A, arquivo); -> No Windows, viraria 0x0D 0x0A ← CORROMPERIA!

         Com "wb" (binário):
        fputc(0x0A, arquivo); -> Mantém 0x0A ← CORRETO!

        No Huffman: Escrevemos bits, bytes crus, árvore serializada - dados binários puros!
        */

        if (new_file == NULL) {
            perror("Erro ao criar o arquivo de saída");
            fclose(original_file);
            return 1;
        }

        // Cria as duas filas de prioridade
        PRIORITY_QUEUE* huff_queue1 = create_queue(); // huff_queue1 → Usada para CONSTRUIR a árvore (é destruída)
        PRIORITY_QUEUE* huff_queue2 = create_queue(); // huff_queue2 → Cópia intacta para usar DEPOIS

        // Preenche as filas com as frequências dos caracteres do arquivo
        create_huff_queue(original_file, &huff_queue1, &huff_queue2);

        // Constrói a árvore de Huffman
        NODE* root = build_huffman_tree(huff_queue1);

        // Cria a tabela de códigos de Huffman
        HuffmanCode huff_table[256] = {0}; // Inicializa a tabela de Huffman
        uint32_t code = 0; // Inicializa o código como um inteiro, garante que ele tenha exatamente 32 bits (4bytes) (#inclue <stdint.h)
        create_huffman_table(root, code, 0, huff_table); // Passa o código como ponteiro

        // Escreve o cabeçalho e a árvore no novo arquivo
        write_header(huff_queue2, huff_table, new_file, root);

        // Libera memória usada
        free_huffman_tree(root);
        free_priority_queue(huff_queue1);
        free_priority_queue(huff_queue2);

        // Reposiciona o ponteiro do arquivo original para o início
        rewind(original_file);

        // Compacta os dados do arquivo original usando a tabela de Huffman
        compactor(original_file, new_file, huff_table);

        // Fecha os arquivos
        fclose(original_file);
        fclose(new_file);

        printf("Arquivo compactado com sucesso: %s\n", new_file_name);

    } else if (option == 2) {
        char compressed_filename[BUFFER_SIZE];
        char final_format[10];
        printf("\nInsira o nome do arquivo compactado (.huff):\n");
        scanf("%s", compressed_filename);

        printf("\nInsira o formato da extensao final (ex: jpg, txt, etc):\n");
        scanf("%s", final_format);

        decompact(compressed_filename, final_format);

    } else {
        printf("Opção inválida.\n");
    }

    return 0;
}




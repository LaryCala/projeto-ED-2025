#include <stdio.h> 
#include <string.h> //Usado para funções como manipulação de linhas do arquivo
#include <stdbool.h>
#include <math.h> //Usado principalmente para abs() (valor absoluto) ao trabalhar com literais positivos/negativos
#include <stdlib.h> 

typedef struct {
    int* literais;      
    int num_literais;  
} Clausula;

typedef struct {
    Clausula* clausulas;
    int num_clausulas;
    int num_variaveis;
} Formula;

typedef struct {
    int* valores;
    int num_variaveis;
} Interpretacao;

// Função para ler fórmula no formato DIMACS
Formula ler_dimacs(const char* nome_arquivo) {
    FILE* arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo %s\n", nome_arquivo);
        exit(1);
    }

    Formula F; //Cria uma variável F do tipo Formula
    F.num_clausulas = 0; //Contadores = 0
    F.num_variaveis = 0;
    F.clausulas = NULL; //Indica que memória não foi alocada ainda para as cláusulas
    
    char linha[1024]; //Lê até 1023 caracteres
    
    // Ler cabeçalho
    while (fgets(linha, sizeof(linha), arquivo)) {
        if (linha[0] == 'c') { //Se a linha começa com 'c'
            continue;  //É é comentário, ignora
        }
        else if (linha[0] == 'p') { //Se a linha começa com 'p', é parâmetros, lê
            sscanf(linha, "p cnf %d %d", &F.num_variaveis, &F.num_clausulas); //sscanf extrai os números da linha 
            F.clausulas = (Clausula*)malloc(F.num_clausulas * sizeof(Clausula)); //malloc aloca um array de F.num_clausulas estruturas Clausula
            for (int i = 0; i < F.num_clausulas; i++) { //O loop inicializa cada cláusula:
                F.clausulas[i].num_literais = 0; //nenhum literal ainda
                F.clausulas[i].literais = NULL; //array de literais ainda não alocado
            }
            break;
        }
    }
    
    int clausula_atual = 0; //Controla qual cláusula estamos preenchendo, de 0 a num_clausulas-1
    int capacidade_atual = 0;
    int literal;
    
    // Ler cláusulas
    while (fscanf(arquivo, "%d", &literal) != EOF && clausula_atual < F.num_clausulas) { //fscanf lê um número por vez do arquivo
        if (literal == 0) { //No formato DIMACS, 0 marca o fim de uma cláusula
            clausula_atual++; //Quando encontra 0, passa para a próxima cláusula
            continue;
        }
        
        // Realocar se necessário
        if (F.clausulas[clausula_atual].num_literais == 0) {
            F.clausulas[clausula_atual].literais = (int*)malloc(10 * sizeof(int));
            capacidade_atual = 10; //Primeira alocação: 10 espaços 
        }
        else if (F.clausulas[clausula_atual].num_literais >= capacidade_atual) {
            capacidade_atual *= 2;
            F.clausulas[clausula_atual].literais = (int*)realloc( //Se encher: dobra a capacidade 
                F.clausulas[clausula_atual].literais, 
                capacidade_atual * sizeof(int)
            );
        }
        
        F.clausulas[clausula_atual].literais[F.clausulas[clausula_atual].num_literais++] = literal; //Adiciona o literal no array
        //num_literais++ incrementa o contador após usar o índice

    }
    
    fclose(arquivo);
    return F;

    /*Exemplo:
    p cnf 3 2
    1 -2 0
    3 -1 0

    Execução:
    1. Detecta p cnf 3 2 → 3 variáveis, 2 cláusulas
    2. Aloca array para 2 cláusulas
    3. Lê 1 → adiciona à cláusula 0
    4. Lê -2 → adiciona à cláusula 0
    5. Lê 0 → passa para cláusula 1
    6. Lê 3 → adiciona à cláusula 1
    7. Lê -1 → adiciona à cláusula 1
    8. Lê 0 → terminou

    Resultado:
    F.clausulas[0] = {[1, -2], 2}
    F.clausulas[1] = {[3, -1], 2}
    */
}

// Verifica se uma cláusula é satisfeita pela interpretação atual
bool clausula_satisfeita(const Clausula* clausula, const Interpretacao* interpretacao) {
    for (int i = 0; i < clausula->num_literais; i++) {
        int literal = clausula->literais[i];
        int index = abs(literal);
        int valor = interpretacao->valores[index];

        if ((literal > 0 && valor == 1) || (literal < 0 && valor == 0)) {
            return true;
        }
    }
    return false;
}

// Verifica se todas as variáveis da cláusula estão atribuídas
bool todas_variaveis_atribuidas_na_clausula(const Clausula* clausula, const Interpretacao* interpretacao) {
    for (int i = 0; i < clausula->num_literais; i++) {
        int index = abs(clausula->literais[i]);
        if (interpretacao->valores[index] == -1) {
            return false;
        }
    }
    return true;
}

// Encontra a próxima variável não atribuída
int encontrar_variavel_livre(const Interpretacao* interpretacao) {
    for (int i = 1; i <= interpretacao->num_variaveis; i++) {
        if (interpretacao->valores[i] == -1) {
            return i;
        }
    }
    return -1;
}

// Função principal do resolvedor SAT (backtracking)
bool resolver_sat(Formula* formula, Interpretacao* interpretacao) {
    for (int i = 0; i < formula->num_clausulas; i++) {
        if (!clausula_satisfeita(&formula->clausulas[i], interpretacao)) {
            if (todas_variaveis_atribuidas_na_clausula(&formula->clausulas[i], interpretacao)) {
                return false;
            }
        }
    }

    int var_livre = encontrar_variavel_livre(interpretacao);

    if (var_livre == -1) {
        return true;
    }

    interpretacao->valores[var_livre] = 1;
    if (resolver_sat(formula, interpretacao)) return true;

    interpretacao->valores[var_livre] = 0;
    if (resolver_sat(formula, interpretacao)) return true;

    interpretacao->valores[var_livre] = -1;
    return false;
}

int main() {
    const char* arquivo_cnf = "input.txt"; //O código vai ler deste arquivo, e define um nome pra ele
    //'const' significa que não pode ser modificado
    
    Formula F = ler_dimacs(arquivo_cnf); //Chama a a função, lê o arquivo e retorna uma estrutura 'Formula' completamente preenchida
    /*F agora contém:
      Numero toral de váriaveis e cláusulas
      Array com todas as cláusulas e seus literais
    */
    
    Interpretacao I;
    I.num_variaveis = F.num_variaveis;
    I.valores = (int*)malloc((F.num_variaveis + 1) * sizeof(int));

    /*Por que +1? Porque as variáveis são numeradas de 1 a N, mas arrays em C começam em 0

    Estrutura resultante:

    I.valores[0] = não usado (lixo)
    I.valores[1] = valor da variável 1
    I.valores[2] = valor da variável 2
    ...
    I.valores[F.num_variaveis] = valor da última variável

    Inicialização com -1:
    -1 = não atribuído (indeterminado)
    0 = falso
    1 = verdadeiro*/

    for (int i = 0; i <= F.num_variaveis; i++) {
        I.valores[i] = -1;
    }
    
    if (resolver_sat(&F, &I)) {
        printf("SAT\n");
        for (int i = 1; i <= F.num_variaveis; i++) {
            printf("%d = %s\n", i, I.valores[i] ? "1" : "0");
        }
    } else {
        printf("UNSAT\n");
    }
    
    // Liberar memória
    for (int i = 0; i < F.num_clausulas; i++) {
        free(F.clausulas[i].literais);
    }
    free(F.clausulas);
    free(I.valores);

    /*Por que essa sequência específica?

    Primeiro: Liberar os arrays de literais de cada cláusula. 
    Cada F.clausulas[i].literais foi alocado com malloc

    Depois: Liberar o array de cláusulas
    F.clausulas foi alocado com malloc

    Finalmente: Liberar o array de valores
    I.valores foi alocado com malloc

    ⚠️ ORDEM IMPORTANTE: Se fizéssemos free(F.clausulas) antes, perderíamos o acesso aos literais individuais e teríamos memory leaks!*/
    
    return 0;
}

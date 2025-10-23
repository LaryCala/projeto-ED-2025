#include <stdio.h> 
#include <string.h> 
#include <stdbool.h>
#include <math.h> 
#include <stdlib.h> 

//int* - PONTEIRO PARA ARRAY DINÂMICO 
/*Armazena ENDEREÇO DE MEMÓRIA de um array
Pode acessar MÚLTIPLOS valores
Tamanho variável: pode crescer/diminuir*/

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
Formula ler_dimacs(const char* nome_arquivo) { //Essa função retorna uma estrutura complexa, não apenas um número ou verdadeiro/falso.
    FILE* arquivo = fopen(nome_arquivo, "r"); //Abre o arquivo para leitura
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
    
    int clausula_atual = 0; //Indica qual cláusula estamos preenchendo agora, de 0 até num_clausulas-1.
    int capacidade_atual = 0; //Armazena quantos literais já cabem na memória alocada do vetor atual; usado pra realocação dinâmica.
    int literal; //Vai guardar temporariamente cada número lido do arquivo.
    
    // Ler cláusulas
    while (fscanf(arquivo, "%d", &literal) != EOF && clausula_atual < F.num_clausulas) { //fscanf lê um número por vez do arquivo 
        /*O loop continua até chegar no final do arquivo (EOF) ou até termos lido todas as cláusulas.
        
        Cada número do arquivo representa:
        Um literal (positivo ou negativo) 
        Ou 0, que indica fim da cláusula (padrão do DIMACS).
        */
        if (literal == 0) { //Se literal for 0, significa que a cláusula atual acabou.
            clausula_atual++; //Então incrementa clausula_atual para começar a preencher a próxima cláusula. "move o foco para a próxima cláusula no vetor F.clausulas"
            continue; //'continue' pula o resto do loop, porque não há literal 0 de verdade para guardar.
        }
        
        // Realocar se necessário
        if (F.clausulas[clausula_atual].num_literais == 0) { //num_literais == 0 significa: esta cláusula está vazia
            //É o primeiro literal que estamos adicionando
            F.clausulas[clausula_atual].literais = (int*)malloc(10 * sizeof(int)); //malloc(10 * sizeof(int)): aloca espaço para 10 inteiros inicialmente
            capacidade_atual = 10; //Primeira alocação: 10 espaços 
        }
        else if (F.clausulas[clausula_atual].num_literais >= capacidade_atual) { //num_literais >= capacidade_atual: o array está cheio!
            capacidade_atual *= 2; //capacidade_atual *= 2: dobra a capacidade (10 → 20 → 40 → 80...)
            F.clausulas[clausula_atual].literais = (int*)realloc( //realloc(): redimensiona o array mantendo os dados existentes
                F.clausulas[clausula_atual].literais, //<-Diz ao realloc QUAL array redimensionar
                capacidade_atual * sizeof(int) //Tradução: "Redimensiona para caber 20 inteiros"
                /*Cálculo:
                
                capacidade_atual = 20 (após dobrar de 10)
                sizeof(int) = 4 bytes (normalmente)
                20 * 4 = 80 bytes*/
            );
        }
        
        F.clausulas[clausula_atual].literais[F.clausulas[clausula_atual].num_literais++] = literal; //Coloca o literal lido na próxima posição livre da cláusula atual, e aumenta o contador de quantos literais essa cláusula já tem
        //num_literais++ incrementa o contador após usar o índice


        /*
        F.clausulas[clausula_atual]
        → acessa a cláusula atual, ou seja, o espaço da fórmula onde estamos guardando os literais no momento.
        Exemplo: se clausula_atual = 0, estamos mexendo na primeira cláusula.
        
        .literais[...]
        → dentro dessa cláusula, acessamos o vetor que contém seus literais.
        
        F.clausulas[clausula_atual].num_literais
        → indica quantos literais já foram adicionados até agora.
        Esse valor é usado como índice (posição no vetor) para colocar o novo literal.
        
        F.clausulas[clausula_atual].num_literais++
        → o ++ pós-fixado (depois da variável) significa:
        
        primeiro use o valor atual de num_literais no índice,
        
        depois some 1 nele.
        
        Exemplo prático:
        Se num_literais era 0, o literal será colocado em literais[0],
        e depois num_literais passa a valer 1.
        
        = literal
        → aqui é onde o literal recém-lido do arquivo (fscanf) é finalmente armazenado na posição certa.*/

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
    for (int i = 0; i < clausula->num_literais; i++) { //Percorre todos os literais que estão dentro da cláusula atual.
        int literal = clausula->literais[i]; //pega o literal da posição i.             
        int index = abs(literal); //pega o número da variável, ignorando o sinal.
        int valor = interpretacao->valores[index];//consulta na interpretação atual se essa variável está atribuída a: 1 → verdadeiro, 0 → falso, -1 → indefinido (ainda não atribuído).

        if ((literal > 0 && valor == 1) || (literal < 0 && valor == 0)) {
            //literal = -1 < 0, valor = 1 <-Falso
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
        if (!clausula_satisfeita(&formula->clausulas[i], interpretacao)) { //Testa as clausulas, uma não deu certo, para e vai para o if
            if (todas_variaveis_atribuidas_na_clausula(&formula->clausulas[i], interpretacao)) { //Uma clausula não deu certo e todas as atribuições foram feitas? Não é a configuração certa. Para e muda
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
    
    Formula F = ler_dimacs(arquivo_cnf); //Chama a função, lê o arquivo e retorna uma estrutura 'Formula' completamente preenchida, é declarada e preenchida ao mesmo tempo, por causa do '=' com a função.
    
    Interpretacao I; //Cria uma variável I do tipo Interpretacao. Ela vai guardar os valores (atribuições) para cada variável da fórmula.
    I.num_variaveis = F.num_variaveis; //A quantidade de variáveis que podem receber valores (I.num_variaveis) é a mesma da fórmula (F.num_variaveis). Porque se a fórmula tem, digamos, 3 variáveis (x1, x2, x3), a interpretação também precisa ter espaço pra guardar o valor de cada uma delas (0 ou 1).
    I.valores = (int*)malloc((F.num_variaveis + 1) * sizeof(int)); //(int*)malloc(...) → aloca esse espaço e transforma o ponteiro retornado em um int*. sizeof(int) → diz quantos bytes precisa reservar pra cada valor (4 bytes normalmente).
    for (int i = 0; i <= F.num_variaveis; i++) { //Esse loop inicializa todas as variáveis com -1, que significa:“essa variável ainda não recebeu valor”.
        I.valores[i] = -1;
    }
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

    ORDEM IMPORTANTE: Se fizéssemos free(F.clausulas) antes, perderíamos o acesso aos literais individuais e teríamos memory leaks!*/
    
    return 0;
}

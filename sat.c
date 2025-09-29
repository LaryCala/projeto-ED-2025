#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
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

    Formula F;
    F.num_clausulas = 0;
    F.num_variaveis = 0;
    F.clausulas = NULL;
    
    char linha[1024];
    
    // Ler cabeçalho
    while (fgets(linha, sizeof(linha), arquivo)) {
        if (linha[0] == 'c') {
            continue;  // Ignorar comentários
        }
        else if (linha[0] == 'p') {
            sscanf(linha, "p cnf %d %d", &F.num_variaveis, &F.num_clausulas);
            F.clausulas = (Clausula*)malloc(F.num_clausulas * sizeof(Clausula));
            for (int i = 0; i < F.num_clausulas; i++) {
                F.clausulas[i].num_literais = 0;
                F.clausulas[i].literais = NULL;
            }
            break;
        }
    }
    
    int clausula_atual = 0;
    int capacidade_atual = 0;
    int literal;
    
    // Ler cláusulas
    while (fscanf(arquivo, "%d", &literal) != EOF && clausula_atual < F.num_clausulas) {
        if (literal == 0) {
            clausula_atual++;
            continue;
        }
        
        // Realocar se necessário
        if (F.clausulas[clausula_atual].num_literais == 0) {
            F.clausulas[clausula_atual].literais = (int*)malloc(10 * sizeof(int));
            capacidade_atual = 10;
        }
        else if (F.clausulas[clausula_atual].num_literais >= capacidade_atual) {
            capacidade_atual *= 2;
            F.clausulas[clausula_atual].literais = (int*)realloc(
                F.clausulas[clausula_atual].literais, 
                capacidade_atual * sizeof(int)
            );
        }
        
        F.clausulas[clausula_atual].literais[F.clausulas[clausula_atual].num_literais++] = literal;
    }
    
    fclose(arquivo);
    return F;
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
    const char* arquivo_cnf = "input.txt";
    
    Formula F = ler_dimacs(arquivo_cnf);
    
    Interpretacao I;
    I.num_variaveis = F.num_variaveis;
    I.valores = (int*)malloc((F.num_variaveis + 1) * sizeof(int));
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
    
    return 0;
}

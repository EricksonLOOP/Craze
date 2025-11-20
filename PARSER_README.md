# Craze Parser - Documentação da API

## 📋 Visão Geral

O **Parser da Linguagem Craze v0.1** implementa um analisador sintático recursivo descendente que converte uma sequência de tokens (produzida pelo lexer) em uma **Abstract Syntax Tree (AST)** navegável, seguindo rigorosamente a gramática EBNF especificada.

## 🏗️ Estrutura da AST

### NodeType (Tipos de Nó)

```c
typedef enum {
    // Declarações
    NODE_VAR_DECL,        // Declaração de variável
    NODE_FUNC_DECL,       // Declaração de função
    NODE_PARAM,           // Parâmetro de função

    // Instruções
    NODE_EXPR_STMT,       // Expressão como instrução
    NODE_IF_STMT,         // Instrução if-else
    NODE_WHILE_STMT,      // Loop while
    NODE_RETURN_STMT,     // Instrução return
    NODE_BLOCK,           // Bloco de código

    // Expressões
    NODE_ASSIGN_EXPR,     // Atribuição
    NODE_BINARY_EXPR,     // Operação binária
    NODE_UNARY_EXPR,      // Operação unária
    NODE_CALL_EXPR,       // Chamada de função
    NODE_VAR_EXPR,        // Referência a variável
    NODE_LITERAL_EXPR,    // Literal

    // Tipos
    NODE_TYPE,            // Tipo de dados
    NODE_LITERAL          // Valor literal
} NodeType;
```

### DataType (Tipos de Dados)

```c
typedef enum {
    TYPE_VOID,      // void
    TYPE_INT,       // int
    TYPE_FLOAT,     // float
    TYPE_STRING,    // string
    TYPE_BOOL,      // bool
    TYPE_INVALID    // tipo inválido/erro
} DataType;
```

### ASTNode (Estrutura Principal)

```c
typedef struct ASTNode {
    NodeType node_type;    // Tipo do nó
    DataType data_type;    // Tipo de dados do resultado
    int line;              // Linha no código fonte
    int column;            // Coluna no código fonte
    
    union {
        // Dados específicos de cada tipo de nó
        struct { /* ... */ } var_decl;
        struct { /* ... */ } func_decl;
        // ... outros tipos
    } data;
} ASTNode;
```

## 🔧 API Principal

### Funções de Controle

#### parser_init
```c
void parser_init(Parser* parser, Lexer* lexer);
```
**Descrição:** Inicializa o parser com um lexer já configurado.

**Parâmetros:**
- `parser`: Ponteiro para estrutura Parser a ser inicializada
- `lexer`: Ponteiro para lexer já inicializado com código fonte

**Exemplo:**
```c
Lexer lexer;
Parser parser;
const char* codigo = "let x: int = 42;";

lexer_init(&lexer, codigo);
parser_init(&parser, &lexer);
```

#### parse_program
```c
ASTNode* parse_program(Parser* parser);
```
**Descrição:** Função principal que faz o parsing completo do programa.

**Retorno:** Ponteiro para nó raiz da AST (tipo `NODE_BLOCK`) ou `NULL` em caso de erro.

**Exemplo:**
```c
ASTNode* program = parse_program(&parser);
if (program && !parser.had_error) {
    // Parse bem-sucedido
    ast_print(program, 0);  // Debug da AST
    ast_free(program);      // Liberar memória
} else {
    printf("Erro: %s\n", parser.error_msg);
}
```

### Funções de Utilidade

#### ast_print
```c
void ast_print(ASTNode* node, int indent);
```
**Descrição:** Imprime a AST de forma hierárquica para debugging.

#### ast_free
```c
void ast_free(ASTNode* node);
```
**Descrição:** Libera recursivamente toda a memória de uma AST.

**CRÍTICO:** Deve ser chamada para evitar vazamentos de memória.

#### node_type_to_string / data_type_to_string
```c
const char* node_type_to_string(NodeType type);
const char* data_type_to_string(DataType type);
```
**Descrição:** Convertem enums para strings (útil para debugging).

## 📐 Gramática e Precedência

### Hierarquia de Precedência (alta → baixa)

```
primary       → literal | IDENTIFIER | "(" expression ")" | function_call
unary         → ("-") unary | primary
factor        → unary (("*" | "/") unary)*
term          → factor (("+" | "-") factor)*
comparison    → term ((">" | "<" | ">=" | "<=") term)*
equality      → comparison (("==" | "!=") comparison)*
assignment    → IDENTIFIER "=" assignment | equality
```

### Regras Sintáticas

```
program       → declaration*
declaration   → var_decl | func_decl | statement
var_decl      → "let" IDENTIFIER ":" type "=" expression ";"
func_decl     → "fn" IDENTIFIER "(" parameters ")" ":" type block
statement     → expr_stmt | if_stmt | while_stmt | return_stmt | block
```

## 🎯 Exemplos de Uso

### Exemplo Básico

```c
#include "craze_parser.h"

int main() {
    const char* source = 
        "let x: int = 42;\n"
        "fn soma(a: int, b: int): int {\n"
        "    return a + b;\n"
        "}";
    
    // Inicializar lexer e parser
    Lexer lexer;
    Parser parser;
    
    lexer_init(&lexer, source);
    parser_init(&parser, &lexer);
    
    // Fazer parsing
    ASTNode* program = parse_program(&parser);
    
    if (program && !parser.had_error) {
        printf("✅ Parsing bem-sucedido!\n");
        ast_print(program, 0);
        
        // Analisar AST...
        // Por exemplo, contar declarações:
        printf("Declarações encontradas: %d\n", 
               program->data.block.stmt_count);
        
        ast_free(program);
    } else {
        printf("❌ Erro de sintaxe: %s\n", parser.error_msg);
    }
    
    // Cleanup
    parser_cleanup(&parser);
    lexer_cleanup(&lexer);
    
    return 0;
}
```

### Navegação pela AST

```c
void analyze_program(ASTNode* program) {
    if (program->node_type != NODE_BLOCK) return;
    
    for (int i = 0; i < program->data.block.stmt_count; i++) {
        ASTNode* stmt = program->data.block.statements[i];
        
        switch (stmt->node_type) {
            case NODE_VAR_DECL:
                printf("Variável: %s (tipo %s)\n", 
                       stmt->data.var_decl.name,
                       data_type_to_string(stmt->data.var_decl.type_node->data.type_node.type));
                break;
                
            case NODE_FUNC_DECL:
                printf("Função: %s (%d parâmetros)\n",
                       stmt->data.func_decl.name,
                       stmt->data.func_decl.param_count);
                break;
                
            case NODE_EXPR_STMT:
                printf("Expressão\n");
                break;
        }
    }
}
```

## ⚠️ Tratamento de Erros

### Detecção de Erros
O parser detecta automaticamente:
- **Erros sintáticos**: Tokens inesperados, estruturas malformadas
- **Erros semânticos básicos**: Atribuições inválidas, tipos inconsistentes
- **EOF prematuro**: Código incompleto

### Recuperação de Erros
Implementa **panic mode recovery**:
```c
// O parser se recupera em pontos de sincronização:
// - Após ponto e vírgula (;)
// - Início de declarações (let, fn)
// - Início de instruções (if, while, return)
```

### Exemplo de Tratamento
```c
ASTNode* program = parse_program(&parser);

if (parser.had_error) {
    printf("Erros encontrados:\n");
    printf("  %s\n", parser.error_msg);
    
    // Mesmo com erros, parte da AST pode estar disponível
    if (program) {
        printf("AST parcial:\n");
        ast_print(program, 0);
        ast_free(program);
    }
}
```

## 🔍 Tabela de Símbolos

### API Básica
```c
// Inicializar tabela
SymbolTable table;
symbol_table_init(&table);

// Adicionar símbolos
symbol_table_add(&table, "variavel", TYPE_INT, 0);      // variável
symbol_table_add(&table, "funcao", TYPE_VOID, 1);      // função

// Buscar símbolos
Symbol* sym = symbol_table_lookup(&table, "variavel");
if (sym) {
    printf("Encontrado: %s (tipo %s)\n", 
           sym->name, data_type_to_string(sym->type));
}

// Cleanup
symbol_table_cleanup(&table);
```

## 📊 Saída da AST

### Formato de Debug
```
PROGRAM
├── VAR_DECL: x:int = 
│   └── LITERAL: 42
├── FUNC_DECL: soma(a:int, b:int) -> int
│   └── BLOCK
│       └── RETURN
│           └── BINARY_OP(+)
│               ├── VAR: a
│               └── VAR: b
└── EXPR_STMT
    └── CALL: print
        └── ARG: "Hello"
```

## 🚀 Performance

| Métrica | Valor |
|---------|--------|
| Complexidade temporal | O(n) |
| Uso de memória | Proporcional ao código |
| Profundidade máxima de recursão | ~50 níveis |
| Nós AST por linha de código | ~3-5 |

## 🧪 Casos de Teste Cobertos

✅ **Declarações:** Variáveis e funções  
✅ **Instruções:** if-else, while, return, blocos  
✅ **Expressões:** Aritméticas, relacionais, atribuições  
✅ **Chamadas de função:** Com argumentos múltiplos  
✅ **Precedência:** Operadores corretamente associados  
✅ **Erros sintáticos:** Recuperação e relatório  
✅ **Programas complexos:** Múltiplas funções e estruturas  

## 🔗 Integração

O parser integra-se perfeitamente com:
- **Lexer Craze**: Consome tokens automaticamente
- **Analisador semântico**: AST pronta para verificação de tipos
- **Gerador de código**: AST navegável para compilação
- **Interpretador**: Execução direta da AST

---

**Próximo passo:** Implementar **Analisador Semântico** para verificação de tipos e escopo completo.
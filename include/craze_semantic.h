#ifndef CRAZE_SEMANTIC_H
#define CRAZE_SEMANTIC_H

#include "craze_parser.h"
#include <stdarg.h>

/* --- Tipos de Escopo --- */
typedef enum
{
    SCOPE_GLOBAL,
    SCOPE_FUNCTION,
    SCOPE_BLOCK
} ScopeType;

/* --- Tipos de Entrada na Tabela de Símbolos --- */
typedef enum
{
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_PARAMETER
} SymbolCategory;

/* --- Informações de Tipo Expandidas --- */
typedef struct TypeInfo
{
    DataType base_type;
    int is_array;           // Para futuras versões
    int is_const;           // Para futuras versões
    struct TypeInfo *inner; // Para tipos complexos futuros
} TypeInfo;

/* --- Entrada na Tabela de Símbolos --- */
typedef struct SymbolEntry
{
    char *name;
    SymbolCategory category;
    TypeInfo *type;
    int declared_line;
    int declared_column;
    int scope_depth;
    struct SymbolEntry *next; // Para encadeamento
    union
    {
        struct
        {
            struct SymbolEntry **parameters; // Array de parâmetros para funções
            int param_count;
            TypeInfo *return_type;
            ASTNode *function_node; // Referência para verificação de return
        } func_info;
        struct
        {
            ASTNode *initializer; // Para verificação de inicialização
        } var_info;
    } details;
} SymbolEntry;

/* --- Escopo (Scope) --- */
typedef struct Scope
{
    SymbolEntry *symbols; // Lista de símbolos deste escopo
    struct Scope *parent; // Escopo pai (NULL para global)
    int depth;            // Profundidade do escopo (0 = global)
    ScopeType scope_type; // GLOBAL, FUNCTION, BLOCK
} Scope;

/* --- Tabela de Símbolos Hierárquica --- */
typedef struct SymbolTable
{
    Scope *current_scope;
    Scope *global_scope;
    int scope_count;
} SymbolTable;

/* --- Contexto de Análise Semântica --- */
typedef struct SemanticAnalyzer
{
    SymbolTable *symbol_table;
    ASTNode *ast_root;
    int error_count;
    int warning_count;
    char error_msg[512];

    // Estado atual para verificação
    TypeInfo *current_return_type; // Tipo de retorno da função atual
    int in_function;               // Flag se está dentro de função
    char current_function[64];     // Nome da função atual
    int has_return_statement;      // Para verificar retorno em funções não-void

    // Configurações
    int strict_mode; // Verificações extras
} SemanticAnalyzer;

/* --- Resultado da Verificação de Tipos --- */
typedef struct TypeCheckResult
{
    TypeInfo *type;
    int is_valid;
    char error_msg[256];
    int implicit_conversion; // Flag para conversões implícitas
} TypeCheckResult;

/* --- FUNÇÕES PÚBLICAS --- */

/* Inicializa o analisador semântico */
void semantic_init(SemanticAnalyzer *analyzer, ASTNode *ast_root);

/* Executa toda a análise semântica */
int semantic_analyze(SemanticAnalyzer *analyzer);

/* Libera recursos do analisador */
void semantic_cleanup(SemanticAnalyzer *analyzer);

/* Retorna relatório de erros e warnings */
void semantic_print_report(SemanticAnalyzer *analyzer);

/* Funções de utilidade para tipos */
TypeInfo *typeinfo_create(DataType base_type);
void typeinfo_free(TypeInfo *type_info);
const char *typeinfo_to_string(TypeInfo *type_info);
TypeInfo *typeinfo_copy(TypeInfo *type_info);

/* Função para inserir built-ins */
void semantic_register_builtins(SemanticAnalyzer *analyzer);

/* --- Compatibilidade com Parser (APIs antigas) --- */

// Estrutura antiga para compatibilidade
typedef struct Symbol
{
    char *name;
    DataType type;
    int is_function;
    struct Symbol *next;
} Symbol;

// Tabela simples para compatibilidade
typedef struct SimpleSymbolTable
{
    Symbol *head;
} SimpleSymbolTable;

// Funções de compatibilidade
void symbol_table_init(SimpleSymbolTable *table);
void symbol_table_add(SimpleSymbolTable *table, const char *name, DataType type, int is_function);
Symbol *symbol_table_lookup(SimpleSymbolTable *table, const char *name);
void symbol_table_cleanup(SimpleSymbolTable *table);

#endif /* CRAZE_SEMANTIC_H */
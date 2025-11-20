#ifndef CRAZE_PARSER_H
#define CRAZE_PARSER_H

#include "craze_lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- Tipos de Nó da AST --- */
typedef enum
{
    // Declarações
    NODE_VAR_DECL,
    NODE_FUNC_DECL,
    NODE_PARAM,

    // Instruções
    NODE_EXPR_STMT,
    NODE_IF_STMT,
    NODE_WHILE_STMT,
    NODE_RETURN_STMT,
    NODE_BLOCK,

    // Expressões
    NODE_ASSIGN_EXPR,
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_CALL_EXPR,
    NODE_VAR_EXPR,
    NODE_LITERAL_EXPR,

    // Tipos e literais
    NODE_TYPE,
    NODE_LITERAL
} NodeType;

/* --- Tipos de Dados --- */
typedef enum
{
    TYPE_VOID,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_INVALID
} DataType;

/* --- Estrutura Base do Nó --- */
typedef struct ASTNode
{
    NodeType node_type;
    DataType data_type; // Tipo do resultado da expressão/declaração
    int line;           // Linha no código fonte
    int column;         // Coluna no código fonte

    union
    {
        /* NODE_VAR_DECL */
        struct
        {
            char *name;
            struct ASTNode *type_node;
            struct ASTNode *initializer;
        } var_decl;

        /* NODE_FUNC_DECL */
        struct
        {
            char *name;
            struct ASTNode **params; // Array de parâmetros
            int param_count;
            struct ASTNode *return_type;
            struct ASTNode *body; // Bloco da função
        } func_decl;

        /* NODE_PARAM */
        struct
        {
            char *name;
            struct ASTNode *type_node;
        } param;

        /* NODE_IF_STMT */
        struct
        {
            struct ASTNode *condition;
            struct ASTNode *then_branch;
            struct ASTNode *else_branch; // Pode ser NULL
        } if_stmt;

        /* NODE_WHILE_STMT */
        struct
        {
            struct ASTNode *condition;
            struct ASTNode *body;
        } while_stmt;

        /* NODE_RETURN_STMT */
        struct
        {
            struct ASTNode *value; // Pode ser NULL para void
        } return_stmt;

        /* NODE_EXPR_STMT */
        struct
        {
            struct ASTNode *expression;
        } expr_stmt;

        /* NODE_BLOCK */
        struct
        {
            struct ASTNode **statements; // Array de instruções
            int stmt_count;
        } block;

        /* NODE_BINARY_EXPR */
        struct
        {
            TokenType operator;
            struct ASTNode *left;
            struct ASTNode *right;
        } binary_expr;

        /* NODE_UNARY_EXPR */
        struct
        {
            TokenType operator;
            struct ASTNode *operand;
        } unary_expr;

        /* NODE_ASSIGN_EXPR */
        struct
        {
            char *variable_name;
            struct ASTNode *value;
        } assign_expr;

        /* NODE_CALL_EXPR */
        struct
        {
            char *function_name;
            struct ASTNode **arguments; // Array de argumentos
            int arg_count;
        } call_expr;

        /* NODE_VAR_EXPR */
        struct
        {
            char *name;
        } var_expr;

        /* NODE_LITERAL */
        struct
        {
            TokenType literal_type;
            union
            {
                int int_value;
                double float_value;
                char *string_value;
                int bool_value;
            } value;
        } literal;

        /* NODE_TYPE */
        struct
        {
            DataType type;
        } type_node;
    } data;
} ASTNode;

/* --- Estrutura do Parser --- */
typedef struct
{
    Lexer *lexer;
    Token current_token;
    Token previous_token;
    char error_msg[256];
    int had_error;
    int panic_mode; // Para recuperação de erros
} Parser;

/* --- Estrutura da Tabela de Símbolos Básica --- */
// Definições movidas para craze_semantic.h para evitar conflitos

/* --- FUNÇÕES PÚBLICAS --- */

/* Inicializa o parser com um lexer */
void parser_init(Parser *parser, Lexer *lexer);

/* Libera recursos do parser e AST */
void parser_cleanup(Parser *parser);

/* Parse do programa completo - ponto de entrada */
ASTNode *parse_program(Parser *parser);

/* Funções de utilidade */
void ast_print(ASTNode *node, int indent); // Para debug
void ast_free(ASTNode *node);              // Liberar árvore
const char *node_type_to_string(NodeType type);
const char *data_type_to_string(DataType type);

// Funções da tabela de símbolos movidas para craze_semantic.h

#endif /* CRAZE_PARSER_H */
#ifndef CRAZE_LEXER_H
#define CRAZE_LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* --- Enums para Tipos de Token --- */
typedef enum
{
    // Palavras-chave
    TOKEN_LET,
    TOKEN_FN,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_VOID,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_BOOL,

    // Identificadores e literais
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,

    // Operadores
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_BANG_EQUAL,
    TOKEN_GREATER,
    TOKEN_LESS,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS_EQUAL,

    // Delimitadores
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,

    // Controle
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

/* --- Estrutura do Token --- */
typedef struct
{
    TokenType type;
    char *lexeme; // String do token no fonte
    int length;   // Comprimento do lexeme
    int line;     // Número da linha (para erro)
    int column;   // Coluna inicial (para erro)
} Token;

/* --- Estrutura do Lexer --- */
typedef struct
{
    const char *source;  // Código fonte completo
    const char *start;   // Início do token atual
    const char *current; // Posição atual de análise
    int line;            // Linha atual
    int column;          // Coluna atual
    char error_msg[256]; // Mensagem de erro
} Lexer;

/* --- FUNÇÕES PÚBLICAS --- */

/* Inicializa o lexer com o código fonte */
void lexer_init(Lexer *lexer, const char *source);

/* Libera recursos do lexer */
void lexer_cleanup(Lexer *lexer);

/* Obtém próximo token - função principal */
Token lexer_next_token(Lexer *lexer);

/* Libera recursos de um token */
void token_free(Token *token);

/* Função utilitária para debug - converte tipo p/ string */
const char *token_type_to_string(TokenType type);

#endif /* CRAZE_LEXER_H */
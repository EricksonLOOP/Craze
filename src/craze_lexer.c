#include "../include/craze_lexer.h"

/* --- FUNÇÕES INTERNAS/HELPERS --- */

/* Verifica fim do arquivo */
static int is_at_end(Lexer *lexer)
{
    return *lexer->current == '\0';
}

/* Avança e retorna char anterior */
static char advance(Lexer *lexer)
{
    lexer->column++;
    return *lexer->current++;
}

/* Olha o próximo char sem consumir */
static char peek(Lexer *lexer)
{
    return *lexer->current;
}

/* Olha dois chars à frente */
static char peek_next(Lexer *lexer)
{
    if (is_at_end(lexer))
        return '\0';
    return lexer->current[1];
}

/* Verifica se char combina com esperado e avança */
static int match(Lexer *lexer, char expected)
{
    if (is_at_end(lexer))
        return 0;
    if (*lexer->current != expected)
        return 0;
    lexer->current++;
    lexer->column++;
    return 1;
}

/* Cria token do tipo especificado */
static Token make_token(Lexer *lexer, TokenType type)
{
    Token token;
    token.type = type;
    token.length = (int)(lexer->current - lexer->start);
    token.lexeme = malloc(token.length + 1);
    if (token.lexeme == NULL)
    {
        // Erro de alocação de memória
        token.type = TOKEN_ERROR;
        token.lexeme = malloc(20);
        strcpy(token.lexeme, "Erro de memória");
        token.length = strlen(token.lexeme);
    }
    else
    {
        memcpy(token.lexeme, lexer->start, token.length);
        token.lexeme[token.length] = '\0';
    }
    token.line = lexer->line;
    token.column = lexer->column - token.length;
    return token;
}

/* Cria token de erro */
static Token error_token(Lexer *lexer, const char *message)
{
    Token token;
    token.type = TOKEN_ERROR;
    token.length = strlen(message);
    token.lexeme = malloc(token.length + 1);
    if (token.lexeme != NULL)
    {
        strcpy(token.lexeme, message);
    }
    token.line = lexer->line;
    token.column = lexer->column;
    return token;
}

/* Pula espaços em branco e comentários */
static void skip_whitespace_and_comments(Lexer *lexer)
{
    for (;;)
    {
        char c = peek(lexer);
        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            advance(lexer);
            break;
        case '\n':
            lexer->line++;
            lexer->column = 0;
            advance(lexer);
            break;
        case '#':
            // Comentário até o final da linha
            while (peek(lexer) != '\n' && !is_at_end(lexer))
            {
                advance(lexer);
            }
            break;
        default:
            return;
        }
    }
}

/* Funções de checagem de caracteres */
static int is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static int is_alphanumeric(char c)
{
    return is_alpha(c) || is_digit(c);
}

/* Verificar se é palavra-chave */
static TokenType check_keyword(const char *lexeme, int length)
{
    switch (lexeme[0])
    {
    case 'b':
        if (length == 4 && memcmp(lexeme, "bool", 4) == 0)
            return TOKEN_BOOL;
        break;
    case 'e':
        if (length == 4 && memcmp(lexeme, "else", 4) == 0)
            return TOKEN_ELSE;
        break;
    case 'f':
        if (length == 2 && memcmp(lexeme, "fn", 2) == 0)
            return TOKEN_FN;
        if (length == 5 && memcmp(lexeme, "false", 5) == 0)
            return TOKEN_FALSE;
        if (length == 5 && memcmp(lexeme, "float", 5) == 0)
            return TOKEN_FLOAT;
        break;
    case 'i':
        if (length == 2 && memcmp(lexeme, "if", 2) == 0)
            return TOKEN_IF;
        if (length == 3 && memcmp(lexeme, "int", 3) == 0)
            return TOKEN_INT;
        break;
    case 'l':
        if (length == 3 && memcmp(lexeme, "let", 3) == 0)
            return TOKEN_LET;
        break;
    case 'r':
        if (length == 6 && memcmp(lexeme, "return", 6) == 0)
            return TOKEN_RETURN;
        break;
    case 's':
        if (length == 6 && memcmp(lexeme, "string", 6) == 0)
            return TOKEN_STRING;
        break;
    case 't':
        if (length == 4 && memcmp(lexeme, "true", 4) == 0)
            return TOKEN_TRUE;
        break;
    case 'v':
        if (length == 4 && memcmp(lexeme, "void", 4) == 0)
            return TOKEN_VOID;
        break;
    case 'w':
        if (length == 5 && memcmp(lexeme, "while", 5) == 0)
            return TOKEN_WHILE;
        break;
    }
    return TOKEN_IDENTIFIER;
}

/* Reconhecimento de identificadores */
static Token identifier(Lexer *lexer)
{
    while (is_alphanumeric(peek(lexer)))
    {
        advance(lexer);
    }

    // Verificar se é palavra-chave
    int length = (int)(lexer->current - lexer->start);
    TokenType type = check_keyword(lexer->start, length);
    return make_token(lexer, type);
}

/* Reconhecimento de números */
static Token number(Lexer *lexer)
{
    while (is_digit(peek(lexer)))
    {
        advance(lexer);
    }

    // Verificar se é float
    if (peek(lexer) == '.' && is_digit(peek_next(lexer)))
    {
        advance(lexer); // Consome o ponto
        while (is_digit(peek(lexer)))
        {
            advance(lexer);
        }
        return make_token(lexer, TOKEN_FLOAT_LITERAL);
    }

    return make_token(lexer, TOKEN_INT_LITERAL);
}

/* Reconhecimento de strings */
static Token string(Lexer *lexer)
{
    while (peek(lexer) != '"' && !is_at_end(lexer))
    {
        if (peek(lexer) == '\n')
        {
            return error_token(lexer, "String não pode conter quebra de linha");
        }
        advance(lexer);
    }

    if (is_at_end(lexer))
    {
        return error_token(lexer, "String não fechada");
    }

    advance(lexer); // Fecha aspas
    return make_token(lexer, TOKEN_STRING_LITERAL);
}

/* --- FUNÇÕES PÚBLICAS --- */

/* Inicializa o lexer com o código fonte */
void lexer_init(Lexer *lexer, const char *source)
{
    lexer->source = source;
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
    lexer->column = 1;
    lexer->error_msg[0] = '\0';
}

/* Libera recursos do lexer */
void lexer_cleanup(Lexer *lexer)
{
    // Não há recursos específicos do lexer para liberar
    // Os tokens individuais devem ser liberados pelo usuário
    lexer->source = NULL;
    lexer->start = NULL;
    lexer->current = NULL;
}

/* Libera recursos de um token */
void token_free(Token *token)
{
    if (token->lexeme != NULL)
    {
        free(token->lexeme);
        token->lexeme = NULL;
    }
}

/* Obtém próximo token - função principal */
Token lexer_next_token(Lexer *lexer)
{
    skip_whitespace_and_comments(lexer);
    lexer->start = lexer->current;

    if (is_at_end(lexer))
    {
        return make_token(lexer, TOKEN_EOF);
    }

    char c = advance(lexer);

    // Identificadores e palavras-chave
    if (is_alpha(c))
    {
        return identifier(lexer);
    }

    // Números
    if (is_digit(c))
    {
        return number(lexer);
    }

    // Strings
    if (c == '"')
    {
        return string(lexer);
    }

    // Caracteres simples
    switch (c)
    {
    case '(':
        return make_token(lexer, TOKEN_LEFT_PAREN);
    case ')':
        return make_token(lexer, TOKEN_RIGHT_PAREN);
    case '{':
        return make_token(lexer, TOKEN_LEFT_BRACE);
    case '}':
        return make_token(lexer, TOKEN_RIGHT_BRACE);
    case ':':
        return make_token(lexer, TOKEN_COLON);
    case ',':
        return make_token(lexer, TOKEN_COMMA);
    case ';':
        return make_token(lexer, TOKEN_SEMICOLON);
    case '+':
        return make_token(lexer, TOKEN_PLUS);
    case '-':
        return make_token(lexer, TOKEN_MINUS);
    case '*':
        return make_token(lexer, TOKEN_STAR);
    case '/':
        return make_token(lexer, TOKEN_SLASH);

    // Operadores de 1-2 caracteres
    case '=':
        if (match(lexer, '='))
        {
            return make_token(lexer, TOKEN_EQUAL_EQUAL);
        }
        return make_token(lexer, TOKEN_EQUAL);
    case '!':
        if (match(lexer, '='))
        {
            return make_token(lexer, TOKEN_BANG_EQUAL);
        }
        return error_token(lexer, "Caractere '!' inesperado");
    case '>':
        if (match(lexer, '='))
        {
            return make_token(lexer, TOKEN_GREATER_EQUAL);
        }
        return make_token(lexer, TOKEN_GREATER);
    case '<':
        if (match(lexer, '='))
        {
            return make_token(lexer, TOKEN_LESS_EQUAL);
        }
        return make_token(lexer, TOKEN_LESS);
    }

    return error_token(lexer, "Caractere inesperado");
}

/* Função utilitária para debug - converte tipo p/ string */
const char *token_type_to_string(TokenType type)
{
    switch (type)
    {
    // Palavras-chave
    case TOKEN_LET:
        return "TOKEN_LET";
    case TOKEN_FN:
        return "TOKEN_FN";
    case TOKEN_RETURN:
        return "TOKEN_RETURN";
    case TOKEN_IF:
        return "TOKEN_IF";
    case TOKEN_ELSE:
        return "TOKEN_ELSE";
    case TOKEN_WHILE:
        return "TOKEN_WHILE";
    case TOKEN_TRUE:
        return "TOKEN_TRUE";
    case TOKEN_FALSE:
        return "TOKEN_FALSE";
    case TOKEN_VOID:
        return "TOKEN_VOID";
    case TOKEN_INT:
        return "TOKEN_INT";
    case TOKEN_FLOAT:
        return "TOKEN_FLOAT";
    case TOKEN_STRING:
        return "TOKEN_STRING";
    case TOKEN_BOOL:
        return "TOKEN_BOOL";

    // Identificadores e literais
    case TOKEN_IDENTIFIER:
        return "TOKEN_IDENTIFIER";
    case TOKEN_INT_LITERAL:
        return "TOKEN_INT_LITERAL";
    case TOKEN_FLOAT_LITERAL:
        return "TOKEN_FLOAT_LITERAL";
    case TOKEN_STRING_LITERAL:
        return "TOKEN_STRING_LITERAL";

    // Operadores
    case TOKEN_PLUS:
        return "TOKEN_PLUS";
    case TOKEN_MINUS:
        return "TOKEN_MINUS";
    case TOKEN_STAR:
        return "TOKEN_STAR";
    case TOKEN_SLASH:
        return "TOKEN_SLASH";
    case TOKEN_EQUAL:
        return "TOKEN_EQUAL";
    case TOKEN_EQUAL_EQUAL:
        return "TOKEN_EQUAL_EQUAL";
    case TOKEN_BANG_EQUAL:
        return "TOKEN_BANG_EQUAL";
    case TOKEN_GREATER:
        return "TOKEN_GREATER";
    case TOKEN_LESS:
        return "TOKEN_LESS";
    case TOKEN_GREATER_EQUAL:
        return "TOKEN_GREATER_EQUAL";
    case TOKEN_LESS_EQUAL:
        return "TOKEN_LESS_EQUAL";

    // Delimitadores
    case TOKEN_LEFT_PAREN:
        return "TOKEN_LEFT_PAREN";
    case TOKEN_RIGHT_PAREN:
        return "TOKEN_RIGHT_PAREN";
    case TOKEN_LEFT_BRACE:
        return "TOKEN_LEFT_BRACE";
    case TOKEN_RIGHT_BRACE:
        return "TOKEN_RIGHT_BRACE";
    case TOKEN_COLON:
        return "TOKEN_COLON";
    case TOKEN_COMMA:
        return "TOKEN_COMMA";
    case TOKEN_SEMICOLON:
        return "TOKEN_SEMICOLON";

    // Controle
    case TOKEN_EOF:
        return "TOKEN_EOF";
    case TOKEN_ERROR:
        return "TOKEN_ERROR";

    default:
        return "UNKNOWN_TOKEN";
    }
}
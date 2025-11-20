#include "../include/craze_lexer.h"

void print_token(Token *token)
{
    printf("LINE %d, COL %d: %s \"%s\"\n",
           token->line,
           token->column,
           token_type_to_string(token->type),
           token->lexeme);
}

void test_basic_tokens()
{
    printf("=== TESTE: Tokens Básicos ===\n");

    const char *source = "let x: int = 42;";
    Lexer lexer;
    lexer_init(&lexer, source);

    Token token;
    do
    {
        token = lexer_next_token(&lexer);
        print_token(&token);
        token_free(&token);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    lexer_cleanup(&lexer);
    printf("\n");
}

void test_complete_program()
{
    printf("=== TESTE: Programa Completo ===\n");

    const char *source =
        "# Comentário de teste\n"
        "let x: int = 42;\n"
        "let y: float = 3.14;\n"
        "let nome: string = \"Craze\";\n"
        "let ativo: bool = true;\n"
        "\n"
        "fn soma(a: int, b: int): int {\n"
        "    if (a == b) {\n"
        "        return a + b;\n"
        "    } else {\n"
        "        return a * 2 + b;\n"
        "    }\n"
        "}\n"
        "\n"
        "while (x > 0) {\n"
        "    x = x - 1;\n"
        "}\n";

    Lexer lexer;
    lexer_init(&lexer, source);

    Token token;
    do
    {
        token = lexer_next_token(&lexer);
        print_token(&token);
        token_free(&token);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    lexer_cleanup(&lexer);
    printf("\n");
}

void test_operators()
{
    printf("=== TESTE: Operadores ===\n");

    const char *source = "+ - * / = == != > < >= <=";
    Lexer lexer;
    lexer_init(&lexer, source);

    Token token;
    do
    {
        token = lexer_next_token(&lexer);
        print_token(&token);
        token_free(&token);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    lexer_cleanup(&lexer);
    printf("\n");
}

void test_keywords()
{
    printf("=== TESTE: Palavras-chave ===\n");

    const char *source = "let fn return if else while true false void int float string bool";
    Lexer lexer;
    lexer_init(&lexer, source);

    Token token;
    do
    {
        token = lexer_next_token(&lexer);
        print_token(&token);
        token_free(&token);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    lexer_cleanup(&lexer);
    printf("\n");
}

void test_literals()
{
    printf("=== TESTE: Literais ===\n");

    const char *source = "42 3.14 \"Hello World\" true false identificador _var var2";
    Lexer lexer;
    lexer_init(&lexer, source);

    Token token;
    do
    {
        token = lexer_next_token(&lexer);
        print_token(&token);
        token_free(&token);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    lexer_cleanup(&lexer);
    printf("\n");
}

void test_delimiters()
{
    printf("=== TESTE: Delimitadores ===\n");

    const char *source = "( ) { } : , ;";
    Lexer lexer;
    lexer_init(&lexer, source);

    Token token;
    do
    {
        token = lexer_next_token(&lexer);
        print_token(&token);
        token_free(&token);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    lexer_cleanup(&lexer);
    printf("\n");
}

void test_error_cases()
{
    printf("=== TESTE: Casos de Erro ===\n");

    // String não fechada
    printf("--- String não fechada ---\n");
    const char *source1 = "\"string sem fim";
    Lexer lexer1;
    lexer_init(&lexer1, source1);
    Token token1 = lexer_next_token(&lexer1);
    print_token(&token1);
    token_free(&token1);
    lexer_cleanup(&lexer1);

    // Caractere inválido
    printf("--- Caractere inválido ---\n");
    const char *source2 = "@#$";
    Lexer lexer2;
    lexer_init(&lexer2, source2);
    Token token2 = lexer_next_token(&lexer2);
    print_token(&token2);
    token_free(&token2);
    lexer_cleanup(&lexer2);

    // String com quebra de linha
    printf("--- String com quebra de linha ---\n");
    const char *source3 = "\"string\ncom quebra\"";
    Lexer lexer3;
    lexer_init(&lexer3, source3);
    Token token3 = lexer_next_token(&lexer3);
    print_token(&token3);
    token_free(&token3);
    lexer_cleanup(&lexer3);

    printf("\n");
}

void test_comments()
{
    printf("=== TESTE: Comentários ===\n");

    const char *source =
        "let x: int = 5; # Este é um comentário\n"
        "# Comentário de linha inteira\n"
        "let y: float = 2.0;";

    Lexer lexer;
    lexer_init(&lexer, source);

    Token token;
    do
    {
        token = lexer_next_token(&lexer);
        print_token(&token);
        token_free(&token);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    lexer_cleanup(&lexer);
    printf("\n");
}

int main()
{
    printf("========================================\n");
    printf("       TESTE DO LEXER CRAZE v0.1       \n");
    printf("========================================\n\n");

    test_basic_tokens();
    test_complete_program();
    test_operators();
    test_keywords();
    test_literals();
    test_delimiters();
    test_comments();
    test_error_cases();

    printf("========================================\n");
    printf("       TESTES CONCLUÍDOS               \n");
    printf("========================================\n");

    return 0;
}
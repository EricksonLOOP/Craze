#include "../include/craze_semantic.h"

void test_basic_variable_declaration()
{
    printf("=== TESTE: Declaração Básica de Variável ===\n");

    const char *source = "let x: int = 42;";
    Lexer lexer;
    Parser parser;
    SemanticAnalyzer analyzer;

    lexer_init(&lexer, source);
    parser_init(&parser, &lexer);

    ASTNode *program = parse_program(&parser);

    if (program && !parser.had_error)
    {
        semantic_init(&analyzer, program);
        int success = semantic_analyze(&analyzer);

        if (success)
        {
            printf("✅ Análise semântica bem-sucedida!\n");
        }
        else
        {
            printf("❌ Erro na análise semântica\n");
        }

        semantic_print_report(&analyzer);
        semantic_cleanup(&analyzer);
        ast_free(program);
    }
    else
    {
        printf("❌ Erro no parsing\n");
    }

    parser_cleanup(&parser);
    lexer_cleanup(&lexer);
    printf("\n");
}

void test_type_compatibility()
{
    printf("=== TESTE: Compatibilidade de Tipos ===\n");

    const char *source =
        "let a: int = 10;\n"
        "let b: float = 3.14;\n"
        "let c: float = a;\n" // OK: int -> float
        "let d: string = \"hello\";\n";

    Lexer lexer;
    Parser parser;
    SemanticAnalyzer analyzer;

    lexer_init(&lexer, source);
    parser_init(&parser, &lexer);

    ASTNode *program = parse_program(&parser);

    if (program && !parser.had_error)
    {
        semantic_init(&analyzer, program);
        int success = semantic_analyze(&analyzer);

        printf("Resultado: %s\n", success ? "Sucesso" : "Erro");
        semantic_print_report(&analyzer);
        semantic_cleanup(&analyzer);
        ast_free(program);
    }

    parser_cleanup(&parser);
    lexer_cleanup(&lexer);
    printf("\n");
}

void test_function_declaration()
{
    printf("=== TESTE: Declaração de Função ===\n");

    const char *source =
        "fn soma(a: int, b: int): int {\n"
        "    return a + b;\n"
        "}";

    Lexer lexer;
    Parser parser;
    SemanticAnalyzer analyzer;

    lexer_init(&lexer, source);
    parser_init(&parser, &lexer);

    ASTNode *program = parse_program(&parser);

    if (program && !parser.had_error)
    {
        semantic_init(&analyzer, program);
        int success = semantic_analyze(&analyzer);

        printf("Resultado: %s\n", success ? "Sucesso" : "Erro");
        semantic_print_report(&analyzer);
        semantic_cleanup(&analyzer);
        ast_free(program);
    }

    parser_cleanup(&parser);
    lexer_cleanup(&lexer);
    printf("\n");
}

void test_function_call()
{
    printf("=== TESTE: Chamada de Função ===\n");

    const char *source =
        "fn soma(a: int, b: int): int {\n"
        "    return a + b;\n"
        "}\n"
        "let resultado: int = soma(10, 20);";

    Lexer lexer;
    Parser parser;
    SemanticAnalyzer analyzer;

    lexer_init(&lexer, source);
    parser_init(&parser, &lexer);

    ASTNode *program = parse_program(&parser);

    if (program && !parser.had_error)
    {
        semantic_init(&analyzer, program);
        int success = semantic_analyze(&analyzer);

        printf("Resultado: %s\n", success ? "Sucesso" : "Erro");
        semantic_print_report(&analyzer);
        semantic_cleanup(&analyzer);
        ast_free(program);
    }

    parser_cleanup(&parser);
    lexer_cleanup(&lexer);
    printf("\n");
}

void test_scope_resolution()
{
    printf("=== TESTE: Resolução de Escopo ===\n");

    const char *source =
        "let x: int = 10;\n"
        "fn teste(): void {\n"
        "    let y: int = x;\n" // OK: x está no escopo global
        "    let z: int = 20;\n"
        "}\n"
        "let a: int = z;"; // ERRO: z não está disponível aqui

    Lexer lexer;
    Parser parser;
    SemanticAnalyzer analyzer;

    lexer_init(&lexer, source);
    parser_init(&parser, &lexer);

    ASTNode *program = parse_program(&parser);

    if (program && !parser.had_error)
    {
        semantic_init(&analyzer, program);
        int success = semantic_analyze(&analyzer);

        printf("Resultado: %s\n", success ? "Sucesso" : "Erro");
        semantic_print_report(&analyzer);
        semantic_cleanup(&analyzer);
        ast_free(program);
    }

    parser_cleanup(&parser);
    lexer_cleanup(&lexer);
    printf("\n");
}

void test_error_cases()
{
    printf("=== TESTE: Casos de Erro ===\n");

    printf("--- Redeclaração de variável ---\n");
    const char *source1 =
        "let x: int = 10;\n"
        "let x: int = 20;"; // ERRO: Redeclaração

    Lexer lexer1;
    Parser parser1;
    SemanticAnalyzer analyzer1;

    lexer_init(&lexer1, source1);
    parser_init(&parser1, &lexer1);
    ASTNode *program1 = parse_program(&parser1);

    if (program1)
    {
        semantic_init(&analyzer1, program1);
        semantic_analyze(&analyzer1);
        semantic_print_report(&analyzer1);
        semantic_cleanup(&analyzer1);
        ast_free(program1);
    }
    parser_cleanup(&parser1);
    lexer_cleanup(&lexer1);

    printf("--- Tipo incompatível ---\n");
    const char *source2 =
        "let x: int = \"string\";"; // ERRO: Tipo incompatível

    Lexer lexer2;
    Parser parser2;
    SemanticAnalyzer analyzer2;

    lexer_init(&lexer2, source2);
    parser_init(&parser2, &lexer2);
    ASTNode *program2 = parse_program(&parser2);

    if (program2)
    {
        semantic_init(&analyzer2, program2);
        semantic_analyze(&analyzer2);
        semantic_print_report(&analyzer2);
        semantic_cleanup(&analyzer2);
        ast_free(program2);
    }
    parser_cleanup(&parser2);
    lexer_cleanup(&lexer2);

    printf("--- Função sem retorno ---\n");
    const char *source3 =
        "fn teste(): int {\n"
        "    let x: int = 10;\n"
        "}"; // ERRO: Falta return

    Lexer lexer3;
    Parser parser3;
    SemanticAnalyzer analyzer3;

    lexer_init(&lexer3, source3);
    parser_init(&parser3, &lexer3);
    ASTNode *program3 = parse_program(&parser3);

    if (program3)
    {
        semantic_init(&analyzer3, program3);
        semantic_analyze(&analyzer3);
        semantic_print_report(&analyzer3);
        semantic_cleanup(&analyzer3);
        ast_free(program3);
    }
    parser_cleanup(&parser3);
    lexer_cleanup(&lexer3);

    printf("\n");
}

void test_builtin_functions()
{
    printf("=== TESTE: Funções Built-in ===\n");

    const char *source =
        "let x: int = 42;\n"
        "print(\"Hello World\");\n"
        "let t: string = type(\"test\");\n"
        "let l: int = len(\"hello\");";

    Lexer lexer;
    Parser parser;
    SemanticAnalyzer analyzer;

    lexer_init(&lexer, source);
    parser_init(&parser, &lexer);

    ASTNode *program = parse_program(&parser);

    if (program && !parser.had_error)
    {
        semantic_init(&analyzer, program);
        int success = semantic_analyze(&analyzer);

        printf("Resultado: %s\n", success ? "Sucesso" : "Erro");
        semantic_print_report(&analyzer);
        semantic_cleanup(&analyzer);
        ast_free(program);
    }

    parser_cleanup(&parser);
    lexer_cleanup(&lexer);
    printf("\n");
}

void test_expression_types()
{
    printf("=== TESTE: Tipos de Expressões ===\n");

    const char *source =
        "let a: int = 10;\n"
        "let b: float = 3.14;\n"
        "let c: float = a + b;\n"                  // OK: int + float -> float
        "let d: string = \"hello\" + \"world\";\n" // OK: string + string
        "let e: bool = a > b;\n"                   // OK: int > float -> bool
        "let f: bool = a == 10;\n";                // OK: int == int -> bool

    Lexer lexer;
    Parser parser;
    SemanticAnalyzer analyzer;

    lexer_init(&lexer, source);
    parser_init(&parser, &lexer);

    ASTNode *program = parse_program(&parser);

    if (program && !parser.had_error)
    {
        semantic_init(&analyzer, program);
        int success = semantic_analyze(&analyzer);

        printf("Resultado: %s\n", success ? "Sucesso" : "Erro");
        semantic_print_report(&analyzer);
        semantic_cleanup(&analyzer);
        ast_free(program);
    }

    parser_cleanup(&parser);
    lexer_cleanup(&lexer);
    printf("\n");
}

void test_complex_program()
{
    printf("=== TESTE: Programa Complexo ===\n");

    const char *source =
        "let x: int = 42;\n"
        "let y: float = 3.14;\n"
        "\n"
        "fn maior(a: int, b: int): int {\n"
        "    if (a > b) {\n"
        "        return a;\n"
        "    } else {\n"
        "        return b;\n"
        "    }\n"
        "}\n"
        "\n"
        "fn principal(): void {\n"
        "    let resultado: int = maior(x, 30);\n"
        "    if (resultado > 0) {\n"
        "        print(\"Resultado é positivo\");\n"
        "    }\n"
        "}";

    Lexer lexer;
    Parser parser;
    SemanticAnalyzer analyzer;

    lexer_init(&lexer, source);
    parser_init(&parser, &lexer);

    ASTNode *program = parse_program(&parser);

    if (program && !parser.had_error)
    {
        semantic_init(&analyzer, program);
        int success = semantic_analyze(&analyzer);

        printf("Resultado: %s\n", success ? "Sucesso" : "Erro");
        semantic_print_report(&analyzer);
        semantic_cleanup(&analyzer);
        ast_free(program);
    }

    parser_cleanup(&parser);
    lexer_cleanup(&lexer);
    printf("\n");
}

int main()
{
    printf("========================================\n");
    printf("   TESTE DO ANALISADOR SEMÂNTICO v0.1  \n");
    printf("========================================\n\n");

    test_basic_variable_declaration();
    test_type_compatibility();
    test_function_declaration();
    test_function_call();
    test_scope_resolution();
    test_builtin_functions();
    test_expression_types();
    test_error_cases();
    test_complex_program();

    printf("========================================\n");
    printf("       TESTES CONCLUÍDOS               \n");
    printf("========================================\n");

    return 0;
}
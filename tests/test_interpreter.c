#include "../include/craze_interpreter.h"

/* --- Programas de Teste --- */

const char *test_program_1 =
    "let x: int = 10;\n"
    "let y: float = 3.14;\n"
    "let z: float = x + y;\n"
    "print(\"Resultado:\", z);";

const char *test_program_2 =
    "fn fatorial(n: int): int {\n"
    "    if (n == 0) {\n"
    "        return 1;\n"
    "    } else {\n"
    "        return n * fatorial(n - 1);\n"
    "    }\n"
    "}\n"
    "\n"
    "let num: int = 5;\n"
    "let resultado: int = fatorial(num);\n"
    "print(\"Fatorial de\", num, \"é\", resultado);";

const char *test_program_3 =
    "fn cumprimentar(nome: string): string {\n"
    "    return \"Olá, \" + nome + \"!\";\n"
    "}\n"
    "\n"
    "let mensagem: string = cumprimentar(\"Craze\");\n"
    "print(mensagem);\n"
    "print(\"Tamanho:\", len(mensagem));";

const char *test_program_4 =
    "let i: int = 0;\n"
    "while (i < 5) {\n"
    "    if (i % 2 == 0) {\n"
    "        print(i, \"é par\");\n"
    "    } else {\n"
    "        print(i, \"é ímpar\");\n"
    "    }\n"
    "    i = i + 1;\n"
    "}";

const char *test_program_5 =
    "let a: int = 42;\n"
    "let b: bool = true;\n"
    "let c: string = \"Teste\";\n"
    "\n"
    "print(\"Tipo de a:\", type(a));\n"
    "print(\"Tipo de b:\", type(b));\n"
    "print(\"Tipo de c:\", type(c));\n"
    "\n"
    "if (a > 40) {\n"
    "    print(\"a é maior que 40\");\n"
    "}";

const char *test_program_expressions =
    "let x: int = 10;\n"
    "let y: int = 5;\n"
    "\n"
    "print(\"Soma:\", x + y);\n"
    "print(\"Subtração:\", x - y);\n"
    "print(\"Multiplicação:\", x * y);\n"
    "print(\"Divisão:\", x / y);\n"
    "print(\"Módulo:\", x % y);\n"
    "\n"
    "print(\"Igual:\", x == y);\n"
    "print(\"Diferente:\", x != y);\n"
    "print(\"Maior:\", x > y);\n"
    "print(\"Menor:\", x < y);\n"
    "print(\"Maior ou igual:\", x >= y);\n"
    "print(\"Menor ou igual:\", x <= y);\n"
    "\n"
    "let verdadeiro: bool = true;\n"
    "print(\"Negação:\", !verdadeiro);";

const char *test_program_scopes =
    "let global_var: int = 100;\n"
    "\n"
    "fn teste_escopo(): void {\n"
    "    let local_var: int = 200;\n"
    "    print(\"Global dentro da função:\", global_var);\n"
    "    print(\"Local:\", local_var);\n"
    "    \n"
    "    if (local_var > global_var) {\n"
    "        let block_var: int = 300;\n"
    "        print(\"Variável do bloco:\", block_var);\n"
    "    }\n"
    "}\n"
    "\n"
    "print(\"Global no main:\", global_var);\n"
    "teste_escopo();";

/* --- Funções de Teste --- */

int execute_test_program(const char *name, const char *source)
{
    printf("========================================\n");
    printf("TESTE: %s\n", name);
    printf("========================================\n");
    printf("Código:\n%s\n", source);
    printf("----------------------------------------\n");
    printf("Saída:\n");

    Lexer lexer;
    Parser parser;
    SemanticAnalyzer analyzer;
    Interpreter interpreter;

    // Lexical analysis
    lexer_init(&lexer, source);
    parser_init(&parser, &lexer);

    ASTNode *program = parse_program(&parser);

    if (program && !parser.had_error)
    {
        // Semantic analysis
        semantic_init(&analyzer, program);
        int semantic_ok = semantic_analyze(&analyzer);

        if (semantic_ok && analyzer.error_count == 0)
        {
            // Interpretation
            interpreter_init(&interpreter, program);
            int result = interpreter_execute(&interpreter);

            interpreter_cleanup(&interpreter);
            semantic_cleanup(&analyzer);
            ast_free(program);
            parser_cleanup(&parser);
            lexer_cleanup(&lexer);

            printf("\n");
            return result;
        }
        else
        {
            printf("[ERRO] Análise semântica falhou:\n");
            semantic_print_report(&analyzer);
            semantic_cleanup(&analyzer);
            ast_free(program);
        }
    }
    else
    {
        printf("[ERRO] Parsing falhou\n");
        if (program)
            ast_free(program);
    }

    parser_cleanup(&parser);
    lexer_cleanup(&lexer);
    printf("\n");
    return 0;
}

void test_values_system()
{
    printf("========================================\n");
    printf("TESTE: Sistema de Valores\n");
    printf("========================================\n");

    // Teste de criação de valores
    Value *int_val = value_create_int(42);
    Value *float_val = value_create_float(3.14);
    Value *string_val = value_create_string("Hello World");
    Value *bool_val = value_create_bool(1);
    Value *void_val = value_create_void();

    printf("Int value: ");
    char *str = value_to_string(int_val);
    printf("%s (tipo: %s)\n", str, value_type_to_string(int_val->type));
    free(str);

    printf("Float value: ");
    str = value_to_string(float_val);
    printf("%s (tipo: %s)\n", str, value_type_to_string(float_val->type));
    free(str);

    printf("String value: ");
    str = value_to_string(string_val);
    printf("%s (tipo: %s)\n", str, value_type_to_string(string_val->type));
    free(str);

    printf("Bool value: ");
    str = value_to_string(bool_val);
    printf("%s (tipo: %s)\n", str, value_type_to_string(bool_val->type));
    free(str);

    printf("Void value: ");
    str = value_to_string(void_val);
    printf("%s (tipo: %s)\n", str, value_type_to_string(void_val->type));
    free(str);

    // Teste de reference counting
    printf("\nTeste de Reference Counting:\n");
    printf("Ref count inicial: %d\n", int_val->ref_count);
    value_incref(int_val);
    printf("Após incref: %d\n", int_val->ref_count);
    value_decref(int_val);
    printf("Após decref: %d\n", int_val->ref_count);

    // Liberar valores
    value_decref(int_val);
    value_decref(float_val);
    value_decref(string_val);
    value_decref(bool_val);
    value_decref(void_val);

    printf("✅ Sistema de valores OK\n\n");
}

void test_hashtable_system()
{
    printf("========================================\n");
    printf("TESTE: Sistema de Hash Table\n");
    printf("========================================\n");

    HashTable *table = hashtable_create(8);

    Value *val1 = value_create_int(100);
    Value *val2 = value_create_string("test");
    Value *val3 = value_create_bool(1);

    hashtable_set(table, "var1", val1);
    hashtable_set(table, "var2", val2);
    hashtable_set(table, "var3", val3);

    printf("Inseridos 3 valores na tabela\n");
    printf("Count: %d\n", table->count);

    Value *retrieved1 = hashtable_get(table, "var1");
    Value *retrieved2 = hashtable_get(table, "var2");
    Value *retrieved3 = hashtable_get(table, "var3");
    Value *not_found = hashtable_get(table, "inexistente");

    printf("var1: %s\n", retrieved1 ? "encontrado" : "não encontrado");
    printf("var2: %s\n", retrieved2 ? "encontrado" : "não encontrado");
    printf("var3: %s\n", retrieved3 ? "encontrado" : "não encontrado");
    printf("inexistente: %s\n", not_found ? "encontrado" : "não encontrado");

    char *str1 = value_to_string(retrieved1);
    char *str2 = value_to_string(retrieved2);
    char *str3 = value_to_string(retrieved3);

    printf("Valores: %s, %s, %s\n", str1, str2, str3);

    free(str1);
    free(str2);
    free(str3);

    hashtable_destroy(table);
    value_decref(val1);
    value_decref(val2);
    value_decref(val3);

    printf("✅ Sistema de hash table OK\n\n");
}

int main()
{
    printf("========================================\n");
    printf("     TESTE DO INTERPRETADOR CRAZE v0.1 \n");
    printf("========================================\n\n");

    // Testes unitários dos sistemas
    test_values_system();
    test_hashtable_system();

    // Testes de programas
    int total_tests = 0;
    int passed_tests = 0;

    total_tests++;
    if (execute_test_program("Cálculos Básicos", test_program_1))
        passed_tests++;
    total_tests++;
    if (execute_test_program("Função Recursiva (Fatorial)", test_program_2))
        passed_tests++;
    total_tests++;
    if (execute_test_program("Manipulação de Strings", test_program_3))
        passed_tests++;
    total_tests++;
    if (execute_test_program("Loop e Condições", test_program_4))
        passed_tests++;
    total_tests++;
    if (execute_test_program("Tipos e Built-ins", test_program_5))
        passed_tests++;
    total_tests++;
    if (execute_test_program("Operações e Expressões", test_program_expressions))
        passed_tests++;
    total_tests++;
    if (execute_test_program("Escopos de Variáveis", test_program_scopes))
        passed_tests++;

    printf("========================================\n");
    printf("       RESUMO DOS TESTES\n");
    printf("========================================\n");
    printf("Testes executados: %d\n", total_tests);
    printf("Testes bem-sucedidos: %d\n", passed_tests);
    printf("Testes falharam: %d\n", total_tests - passed_tests);
    printf("Taxa de sucesso: %.1f%%\n", (float)passed_tests / total_tests * 100.0f);

    if (passed_tests == total_tests)
    {
        printf("🎉 TODOS OS TESTES PASSARAM!\n");
    }
    else
    {
        printf("❌ Alguns testes falharam\n");
    }

    printf("========================================\n");

    return (passed_tests == total_tests) ? 0 : 1;
}
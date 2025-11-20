#include "../include/craze_interpreter.h"
#include <stdio.h>
#include <stdlib.h>

// Função para ler arquivo completo
char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "rb"); // IMPORTANTE: Modo BINÁRIO para evitar conversão CRLF
    if (!file)
    {
        fprintf(stderr, "Erro: Não foi possível abrir o arquivo '%s'\n", filename);
        return NULL;
    }

    // Descobrir tamanho do arquivo
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Alocar memória e ler conteúdo
    char *content = malloc(length + 1);
    if (!content)
    {
        fprintf(stderr, "Erro: Memória insuficiente\n");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(content, 1, length, file);
    if (bytes_read != (size_t)length)
    {
        fprintf(stderr, "Aviso: Esperava %ld bytes, leu %zu bytes\n", length, bytes_read);
    }
    content[length] = '\0';
    fclose(file);

    return content;
}

// Executar arquivo .craze
int execute_craze_file(const char *filename)
{
    printf("========================================\n");
    printf("       CRAZE v0.1 INTERPRETER\n");
    printf("========================================\n");
    printf("Executando: %s\n", filename);
    printf("----------------------------------------\n\n");

    // Ler código fonte
    char *source = read_file(filename);
    if (!source)
    {
        return 1;
    }

    printf("Código fonte:\n");
    // Fazer cópia do source para impressão para evitar corrupção
    size_t source_len = strlen(source);
    char *source_display = malloc(source_len + 1);
    if (source_display)
    {
        strcpy(source_display, source);
        printf("%s\n", source_display);
        free(source_display);
    }
    else
    {
        printf("%s\n", source);
    }
    printf("----------------------------------------\n");
    printf("Saída do programa:\n\n");

    Lexer lexer;
    Parser parser;
    SemanticAnalyzer analyzer;
    Interpreter interpreter;

    // Pipeline completo: Lexer → Parser → Semantic → Interpreter
    lexer_init(&lexer, source);
    parser_init(&parser, &lexer);

    ASTNode *program = parse_program(&parser);

    if (program && !parser.had_error)
    {
        // Análise semântica
        semantic_init(&analyzer, program);
        int semantic_ok = semantic_analyze(&analyzer);

        if (semantic_ok && analyzer.error_count == 0)
        {
            // Interpretação
            interpreter_init(&interpreter, program);
            int result = interpreter_execute(&interpreter);

            // Limpeza
            interpreter_cleanup(&interpreter);
            semantic_cleanup(&analyzer);
            ast_free(program);
            parser_cleanup(&parser);
            lexer_cleanup(&lexer);
            free(source);

            return result ? 0 : 1;
        }
        else
        {
            printf("\n[ERRO] Análise semântica falhou:\n");
            semantic_print_report(&analyzer);
            semantic_cleanup(&analyzer);
            ast_free(program);
        }
    }
    else
    {
        printf("\n[ERRO] Erro na análise sintática\n");
        if (program)
            ast_free(program);
    }

    parser_cleanup(&parser);
    lexer_cleanup(&lexer);
    free(source);
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("========================================\n");
        printf("         CRAZE v0.1 INTERPRETER\n");
        printf("========================================\n\n");
        printf("Uso: %s <arquivo.craze>\n\n", argv[0]);
        printf("Exemplos:\n");
        printf("  %s examples/01_hello_world.craze\n", argv[0]);
        printf("  %s examples/02_calculadora.craze\n", argv[0]);
        printf("  %s examples/03_fatorial.craze\n", argv[0]);
        printf("  %s meu_programa.craze\n", argv[0]);
        printf("\nArquivos exemplo disponíveis:\n");
        printf("  01_hello_world.craze  - Primeiro programa\n");
        printf("  02_calculadora.craze  - Operações matemáticas\n");
        printf("  03_fatorial.craze     - Recursão\n");
        printf("  04_loops.craze        - Contadores e loops\n");
        printf("  05_strings.craze      - Manipulação de texto\n");
        printf("  06_notas.craze        - Sistema acadêmico\n");
        printf("  07_algoritmos.craze   - Fibonacci, primos, MDC\n");
        return 1;
    }

    return execute_craze_file(argv[1]);
}
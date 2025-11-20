#include "../include/craze_parser.h"

void print_usage(const char *program_name)
{
    printf("Uso: %s <arquivo.craze>\n", program_name);
    printf("   ou: %s -c \"codigo craze\"\n", program_name);
    printf("\nExemplos:\n");
    printf("  %s exemplo.craze\n", program_name);
    printf("  %s -c \"let x: int = 42;\"\n", program_name);
}

char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        printf("Erro: Não foi possível abrir o arquivo '%s'\n", filename);
        return NULL;
    }

    // Determinar tamanho do arquivo
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Alocar buffer
    char *buffer = malloc(length + 1);
    if (buffer == NULL)
    {
        printf("Erro: Não foi possível alocar memória\n");
        fclose(file);
        return NULL;
    }

    // Ler arquivo
    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);
    return buffer;
}

void parse_and_print(const char *source)
{
    Lexer lexer;
    Parser parser;

    lexer_init(&lexer, source);
    parser_init(&parser, &lexer);

    printf("========================================\n");
    printf("       PARSING DO CÓDIGO CRAZE         \n");
    printf("========================================\n\n");

    ASTNode *program = parse_program(&parser);

    if (program && !parser.had_error)
    {
        printf("✅ PARSING CONCLUÍDO COM SUCESSO!\n\n");
        printf("AST (Abstract Syntax Tree):\n");
        printf("========================================\n");
        ast_print(program, 0);
        printf("========================================\n");

        ast_free(program);
    }
    else
    {
        printf("❌ ERRO NO PARSING!\n");
        printf("Detalhes: %s\n", parser.error_msg);
    }

    parser_cleanup(&parser);
    lexer_cleanup(&lexer);
}

int main(int argc, char *argv[])
{
    printf("Craze Parser v0.1 - Analisador Sintático\n\n");

    if (argc < 2)
    {
        print_usage(argv[0]);
        return 1;
    }

    char *source = NULL;

    // Verificar se é código inline (-c flag)
    if (argc == 3 && strcmp(argv[1], "-c") == 0)
    {
        source = malloc(strlen(argv[2]) + 1);
        strcpy(source, argv[2]);
        printf("Analisando código inline...\n\n");
    }
    // Caso contrário, ler arquivo
    else
    {
        const char *filename = argv[1];
        printf("Analisando arquivo: %s\n\n", filename);
        source = read_file(filename);
        if (source == NULL)
        {
            return 1;
        }
    }

    parse_and_print(source);

    free(source);
    return 0;
}
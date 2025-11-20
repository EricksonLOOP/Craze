#include "../include/craze_lexer.h"

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

void tokenize_and_print(const char *source)
{
    Lexer lexer;
    lexer_init(&lexer, source);

    printf("========================================\n");
    printf("       TOKENIZAÇÃO DO CÓDIGO CRAZE     \n");
    printf("========================================\n\n");

    int token_count = 0;
    Token token;

    do
    {
        token = lexer_next_token(&lexer);

        if (token.type == TOKEN_ERROR)
        {
            printf("❌ ERRO LÉXICO:\n");
            printf("   Linha %d, Coluna %d: %s\n\n",
                   token.line, token.column, token.lexeme);
            token_free(&token);
            break;
        }

        if (token.type != TOKEN_EOF)
        {
            printf("%3d. %-20s \"%s\" (L%d:C%d)\n",
                   ++token_count,
                   token_type_to_string(token.type),
                   token.lexeme,
                   token.line,
                   token.column);
        }

        token_free(&token);

    } while (token.type != TOKEN_EOF);

    if (token.type == TOKEN_EOF)
    {
        printf("\n✅ TOKENIZAÇÃO CONCLUÍDA COM SUCESSO!\n");
        printf("Total de tokens processados: %d\n", token_count);
    }

    lexer_cleanup(&lexer);
    printf("========================================\n");
}

int main(int argc, char *argv[])
{
    printf("Craze Lexer v0.1 - Analisador Léxico\n\n");

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

    tokenize_and_print(source);

    free(source);
    return 0;
}
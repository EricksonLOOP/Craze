# Lexer da Linguagem Craze - Documentação da API

## Visão Geral

Este lexer (analisador léxico) implementa a tokenização completa da linguagem de programação Craze v0.1 conforme especificação oficial. Ele transforma código fonte em uma sequência de tokens que podem ser utilizados por um parser.

## Compilação

### Requisitos
- Compilador C (gcc, clang, MSVC)
- Make (opcional, mas recomendado)

### Compilação Rápida
```bash
# Com Make
make test

# Manual
gcc -Wall -Wextra -std=c99 -Iinclude -c src/craze_lexer.c -o craze_lexer.o
gcc -Wall -Wextra -std=c99 -Iinclude -c tests/test_lexer.c -o test_lexer.o
gcc craze_lexer.o test_lexer.o -o test_lexer
```

### Execução dos Testes
```bash
make test
# ou
./test_lexer
```

## API Pública

### Estruturas de Dados

#### TokenType (enum)
```c
typedef enum {
    // Palavras-chave
    TOKEN_LET, TOKEN_FN, TOKEN_RETURN, TOKEN_IF, TOKEN_ELSE, TOKEN_WHILE,
    TOKEN_TRUE, TOKEN_FALSE, TOKEN_VOID, TOKEN_INT, TOKEN_FLOAT, TOKEN_STRING, TOKEN_BOOL,

    // Identificadores e literais
    TOKEN_IDENTIFIER, TOKEN_INT_LITERAL, TOKEN_FLOAT_LITERAL, TOKEN_STRING_LITERAL,

    // Operadores
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL, TOKEN_BANG_EQUAL, TOKEN_GREATER, TOKEN_LESS,
    TOKEN_GREATER_EQUAL, TOKEN_LESS_EQUAL,

    // Delimitadores
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COLON, TOKEN_COMMA, TOKEN_SEMICOLON,

    // Controle
    TOKEN_EOF, TOKEN_ERROR
} TokenType;
```

#### Token (struct)
```c
typedef struct {
    TokenType type;     // Tipo do token
    char* lexeme;       // String do token (alocada dinamicamente)
    int length;         // Comprimento do lexeme
    int line;           // Número da linha no fonte
    int column;         // Coluna inicial no fonte
} Token;
```

#### Lexer (struct)
```c
typedef struct {
    const char* source;    // Código fonte completo
    const char* start;     // Início do token atual
    const char* current;   // Posição atual de análise
    int line;              // Linha atual
    int column;            // Coluna atual
    char error_msg[256];   // Mensagem de erro
} Lexer;
```

### Funções Principais

#### lexer_init
```c
void lexer_init(Lexer* lexer, const char* source);
```
**Descrição:** Inicializa o lexer com o código fonte.
- `lexer`: Ponteiro para estrutura Lexer a ser inicializada
- `source`: String contendo o código fonte (deve permanecer válida durante uso do lexer)

**Exemplo:**
```c
Lexer lexer;
const char* codigo = "let x: int = 42;";
lexer_init(&lexer, codigo);
```

#### lexer_next_token
```c
Token lexer_next_token(Lexer* lexer);
```
**Descrição:** Obtém o próximo token do código fonte. Esta é a função principal do lexer.

**Retorno:** Token estrutura contendo o próximo token encontrado.

**Comportamento:**
- Retorna `TOKEN_EOF` quando atinge o fim do arquivo
- Retorna `TOKEN_ERROR` em caso de erro léxico
- Pula automaticamente espaços em branco e comentários

**Exemplo:**
```c
Token token;
do {
    token = lexer_next_token(&lexer);
    // Processar token...
    token_free(&token);  // Liberar memória
} while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);
```

#### token_free
```c
void token_free(Token* token);
```
**Descrição:** Libera a memória alocada para o lexeme do token.
- `token`: Ponteiro para o token a ser liberado

**IMPORTANTE:** Deve ser chamada para cada token obtido para evitar vazamentos de memória.

#### lexer_cleanup
```c
void lexer_cleanup(Lexer* lexer);
```
**Descrição:** Limpa recursos do lexer. 
- `lexer`: Ponteiro para o lexer a ser limpo

**Nota:** Atualmente não há recursos específicos para liberar, mas boa prática chamar ao final.

#### token_type_to_string
```c
const char* token_type_to_string(TokenType type);
```
**Descrição:** Converte tipo de token para string (útil para debug).
- `type`: Tipo do token a ser convertido
- **Retorno:** String constante com nome do tipo

## Exemplo de Uso Completo

```c
#include "craze_lexer.h"

int main() {
    const char* source = 
        "let x: int = 42;\n"
        "fn soma(a: int, b: int): int {\n"
        "    return a + b;\n"
        "}";
    
    // Inicializar lexer
    Lexer lexer;
    lexer_init(&lexer, source);
    
    // Processar todos os tokens
    Token token;
    do {
        token = lexer_next_token(&lexer);
        
        if (token.type == TOKEN_ERROR) {
            printf("ERRO na linha %d, coluna %d: %s\n", 
                   token.line, token.column, token.lexeme);
            break;
        }
        
        printf("Token: %s = \"%s\" (linha %d, col %d)\n",
               token_type_to_string(token.type),
               token.lexeme,
               token.line,
               token.column);
        
        // IMPORTANTE: Liberar memória do token
        token_free(&token);
        
    } while (token.type != TOKEN_EOF);
    
    // Limpar lexer
    lexer_cleanup(&lexer);
    return 0;
}
```

## Tratamento de Erros

O lexer detecta e reporta os seguintes erros:

1. **String não fechada**: String que atinge fim de arquivo sem `"`
2. **String com quebra de linha**: String contém `\n` (não permitido na v0.1)
3. **Caractere inesperado**: Caracteres não reconhecidos pela gramática
4. **Operador inválido**: `!` sem `=` seguinte

Quando um erro ocorre, `lexer_next_token()` retorna token do tipo `TOKEN_ERROR` com mensagem descritiva no campo `lexeme`.

## Tokens Reconhecidos

### Palavras-chave
`let`, `fn`, `return`, `if`, `else`, `while`, `true`, `false`, `void`, `int`, `float`, `string`, `bool`

### Operadores
`+`, `-`, `*`, `/`, `=`, `==`, `!=`, `>`, `<`, `>=`, `<=`

### Delimitadores
`(`, `)`, `{`, `}`, `:`, `,`, `;`

### Literais
- **Inteiros**: `42`, `0`, `-10`
- **Floats**: `3.14`, `0.0`, `-2.5`
- **Strings**: `"Hello"`, `"Craze"`
- **Booleanos**: `true`, `false`

### Comentários
Comentários de linha única iniciados com `#` até o final da linha são ignorados.

## Gestão de Memória

**CRÍTICO:** O lexer aloca dinamicamente memória para `token.lexeme`. É responsabilidade do usuário chamar `token_free()` para cada token obtido, evitando vazamentos de memória.

```c
Token token = lexer_next_token(&lexer);
// ... usar token ...
token_free(&token);  // OBRIGATÓRIO
```

## Considerações de Performance

- O lexer processa caractere por caractere em uma única passada
- Complexidade O(n) onde n é o tamanho do código fonte
- Memória adicional proporcional ao tamanho dos tokens individuais
- Não há pre-processamento ou caching de tokens

## Limitações da v0.1

1. **Sem escape sequences**: Strings não suportam `\n`, `\"`, etc.
2. **Sem números negativos como literais**: `-42` é tokenizado como `MINUS` + `42`
3. **Sem comentários de bloco**: Apenas `#` até fim de linha
4. **Sem Unicode**: Apenas ASCII básico
5. **Sem operadores lógicos**: `&&`, `||` não implementados

## Compatibilidade

- **Padrão C99** ou superior
- **Windows** (MinGW, MSVC)
- **Linux** (gcc, clang)
- **macOS** (clang, gcc)

Testado com gcc 9.4.0, clang 12.0.0, e MinGW-w64 8.1.0.
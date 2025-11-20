# Makefile para o Lexer da Linguagem Craze
# Compatível com Windows (MinGW), Linux e macOS

CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -pedantic -g
INCLUDES=-Iinclude
SRCDIR=src
TESTDIR=tests
OBJDIR=obj
BINDIR=bin

# Arquivos fonte
LEXER_SOURCES=$(SRCDIR)/craze_lexer.c
PARSER_SOURCES=$(SRCDIR)/craze_parser.c
SEMANTIC_SOURCES=$(SRCDIR)/craze_semantic.c
INTERPRETER_SOURCES=$(SRCDIR)/craze_interpreter.c
MAIN_SOURCES=$(SRCDIR)/craze_main.c
LEXER_OBJECTS=$(OBJDIR)/craze_lexer.o
PARSER_OBJECTS=$(OBJDIR)/craze_parser.o
SEMANTIC_OBJECTS=$(OBJDIR)/craze_semantic.o
INTERPRETER_OBJECTS=$(OBJDIR)/craze_interpreter.o
MAIN_OBJECTS=$(OBJDIR)/craze_main.o

# Testes
TEST_LEXER_SOURCES=$(TESTDIR)/test_lexer.c
TEST_SEMANTIC_SOURCES=$(TESTDIR)/test_semantic.c
TEST_INTERPRETER_SOURCES=$(TESTDIR)/test_interpreter.c
TEST_LEXER_OBJECTS=$(OBJDIR)/test_lexer.o
TEST_SEMANTIC_OBJECTS=$(OBJDIR)/test_semantic.o
TEST_INTERPRETER_OBJECTS=$(OBJDIR)/test_interpreter.o

# Utilitários
TOKENIZER_SOURCES=$(TESTDIR)/craze_tokenizer.c
PARSER_TOOL_SOURCES=$(TESTDIR)/craze_parser_tool.c
TOKENIZER_OBJECTS=$(OBJDIR)/craze_tokenizer.o
PARSER_TOOL_OBJECTS=$(OBJDIR)/craze_parser_tool.o

# Executáveis
TEST_LEXER_BIN=$(BINDIR)/test_lexer

TEST_SEMANTIC_BIN=$(BINDIR)/test_semantic
TEST_INTERPRETER_BIN=$(BINDIR)/test_interpreter
CRAZE_BIN=$(BINDIR)/craze
TOKENIZER_BIN=$(BINDIR)/craze_tokenizer
PARSER_TOOL_BIN=$(BINDIR)/craze_parser_tool

# Detectar sistema operacional
ifeq ($(OS),Windows_NT)
    # Windows
    RM=del /Q
    MKDIR=mkdir
    TEST_LEXER_BIN=$(BINDIR)/test_lexer.exe
    TEST_SEMANTIC_BIN=$(BINDIR)/test_semantic.exe
    TEST_INTERPRETER_BIN=$(BINDIR)/test_interpreter.exe
    CRAZE_BIN=$(BINDIR)/craze.exe
    TOKENIZER_BIN=$(BINDIR)/craze_tokenizer.exe
    PARSER_TOOL_BIN=$(BINDIR)/craze_parser_tool.exe
    PATHSEP=\\
else
    # Unix-like (Linux, macOS)
    RM=rm -f
    MKDIR=mkdir -p
    PATHSEP=/
endif

# Regra padrão
all: directories $(TEST_LEXER_BIN)  $(TEST_SEMANTIC_BIN) $(TEST_INTERPRETER_BIN) $(CRAZE_BIN) $(TOKENIZER_BIN) $(PARSER_TOOL_BIN)

# Criar diretórios necessários
directories:
ifeq ($(OS),Windows_NT)
	@if not exist "$(OBJDIR)" $(MKDIR) $(OBJDIR)
	@if not exist "$(BINDIR)" $(MKDIR) $(BINDIR)
else
	@$(MKDIR) $(OBJDIR)
	@$(MKDIR) $(BINDIR)
endif

# Compilar objetos principais
$(OBJDIR)/craze_lexer.o: $(SRCDIR)/craze_lexer.c include/craze_lexer.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/craze_parser.o: $(SRCDIR)/craze_parser.c include/craze_parser.h include/craze_lexer.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/craze_semantic.o: $(SRCDIR)/craze_semantic.c include/craze_semantic.h include/craze_parser.h include/craze_lexer.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/craze_interpreter.o: $(SRCDIR)/craze_interpreter.c include/craze_interpreter.h include/craze_semantic.h include/craze_parser.h include/craze_lexer.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compilar objetos de teste
$(OBJDIR)/test_lexer.o: $(TESTDIR)/test_lexer.c include/craze_lexer.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/test_semantic.o: $(TESTDIR)/test_semantic.c include/craze_semantic.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/test_interpreter.o: $(TESTDIR)/test_interpreter.c include/craze_interpreter.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/craze_main.o: $(SRCDIR)/craze_main.c include/craze_interpreter.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/craze_tokenizer.o: $(TESTDIR)/craze_tokenizer.c include/craze_lexer.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/craze_parser_tool.o: $(TESTDIR)/craze_parser_tool.c include/craze_parser.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Linkar executáveis
$(TEST_LEXER_BIN): $(LEXER_OBJECTS) $(TEST_LEXER_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_SEMANTIC_BIN): $(LEXER_OBJECTS) $(PARSER_OBJECTS) $(SEMANTIC_OBJECTS) $(TEST_SEMANTIC_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_INTERPRETER_BIN): $(LEXER_OBJECTS) $(PARSER_OBJECTS) $(SEMANTIC_OBJECTS) $(INTERPRETER_OBJECTS) $(TEST_INTERPRETER_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

$(CRAZE_BIN): $(LEXER_OBJECTS) $(PARSER_OBJECTS) $(SEMANTIC_OBJECTS) $(INTERPRETER_OBJECTS) $(MAIN_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

$(TOKENIZER_BIN): $(LEXER_OBJECTS) $(TOKENIZER_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

$(PARSER_TOOL_BIN): $(LEXER_OBJECTS) $(PARSER_OBJECTS) $(PARSER_TOOL_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

# Executar testes
test: $(TEST_LEXER_BIN) $(TEST_SEMANTIC_BIN) $(TEST_INTERPRETER_BIN)
	@echo ========================================
	@echo Executando testes do lexer...
	@echo ========================================
	./$(TEST_LEXER_BIN)
	@echo ========================================
	@echo ========================================
	@echo Executando testes do analisador semântico...
	@echo ========================================
	./$(TEST_SEMANTIC_BIN)
	@echo ========================================
	@echo Executando testes do interpretador...
	@echo ========================================
	./$(TEST_INTERPRETER_BIN)

test-lexer: $(TEST_LEXER_BIN)
	./$(TEST_LEXER_BIN)

test-semantic: $(TEST_SEMANTIC_BIN)
	./$(TEST_SEMANTIC_BIN)

test-interpreter: $(TEST_INTERPRETER_BIN)
	./$(TEST_INTERPRETER_BIN)

tools: $(TOKENIZER_BIN) $(PARSER_TOOL_BIN)
	@echo Ferramentas compiladas com sucesso!

# Compilação rápida para desenvolvimento
dev: CFLAGS += -O0 -DDEBUG
dev: all

# Compilação para release
release: CFLAGS += -O2 -DNDEBUG
release: all

# Limpeza
clean:
ifeq ($(OS),Windows_NT)
	@if exist "$(OBJDIR)" $(RM) $(OBJDIR)$(PATHSEP)*
	@if exist "$(BINDIR)" $(RM) $(BINDIR)$(PATHSEP)*
else
	$(RM) $(OBJDIR)/*
	$(RM) $(BINDIR)/*
endif

# Limpeza completa
distclean: clean
ifeq ($(OS),Windows_NT)
	@if exist "$(OBJDIR)" rmdir $(OBJDIR)
	@if exist "$(BINDIR)" rmdir $(BINDIR)
else
	$(RM) -r $(OBJDIR) $(BINDIR)
endif

# Verificar vazamentos de memória (Linux/macOS apenas)
memcheck: $(TEST_LEXER_BIN) $(TEST_SEMANTIC_BIN) $(TEST_INTERPRETER_BIN)
ifneq ($(OS),Windows_NT)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TEST_LEXER_BIN)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TEST_SEMANTIC_BIN)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TEST_INTERPRETER_BIN)
else
	@echo Memcheck não disponível no Windows. Use ferramentas como Dr. Memory.
endif

# Informações do sistema
info:
	@echo Sistema: $(OS)
	@echo Compilador: $(CC)
	@echo Flags: $(CFLAGS)
	@echo Includes: $(INCLUDES)

# Instalar dependências de desenvolvimento (apenas informativo)
install-deps:
ifeq ($(OS),Windows_NT)
	@echo Para Windows: Instale MinGW-w64 ou Visual Studio Build Tools
	@echo https://www.mingw-w64.org/downloads/
else
	@echo Para Ubuntu/Debian: sudo apt-get install build-essential valgrind
	@echo Para macOS: xcode-select --install
	@echo Para Fedora: sudo dnf install gcc make valgrind
endif

# Ajuda
help:
	@echo Targets disponíveis:
	@echo   all        - Compila tudo (padrão)
	@echo   test             - Compila e executa todos os testes
	@echo   test-lexer       - Executa apenas testes do lexer
	@echo   test-parser      - Executa apenas testes do parser
	@echo   test-semantic    - Executa apenas testes do analisador semântico
	@echo   test-interpreter - Executa apenas testes do interpretador
	@echo   tools      - Compila ferramentas utilitárias
	@echo   dev        - Compilação para desenvolvimento
	@echo   release    - Compilação otimizada
	@echo   clean      - Remove arquivos objeto e executáveis
	@echo   distclean  - Remove tudo incluindo diretórios
	@echo   memcheck   - Verifica vazamentos de memória (Unix apenas)
	@echo   info       - Mostra informações do sistema
	@echo   help       - Mostra esta ajuda

# Evitar problemas com arquivos de mesmo nome
.PHONY: all directories test test-lexer test-parser test-semantic test-interpreter tools dev release clean distclean memcheck info install-deps help
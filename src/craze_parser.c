#include "../include/craze_parser.h"
#include "../include/craze_semantic.h"

/* Para strdup no MinGW */
#ifdef _WIN32
#endif

/* --- DECLARAÇÕES ANTECIPADAS --- */
static void parser_error(Parser *parser, const char *message);
static void synchronize(Parser *parser);
static ASTNode *parse_block(Parser *parser);

/* --- FUNÇÕES UTILITÁRIAS DE TOKEN --- */

static void advance(Parser *parser)
{
    // Liberar apenas o lexeme do previous_token antes de sobrescrevê-lo
    // Não chamamos token_free pois queremos manter o struct, só liberar o lexeme
    if (parser->previous_token.lexeme != NULL)
    {
        free(parser->previous_token.lexeme);
        parser->previous_token.lexeme = NULL;
    }

    // Mover current para previous (cópia rasa, mas segura agora)
    parser->previous_token = parser->current_token;

    // Obter novo token do lexer
    parser->current_token = lexer_next_token(parser->lexer);
}

static int check(Parser *parser, TokenType type)
{
    return parser->current_token.type == type;
}

static int match(Parser *parser, TokenType type)
{
    if (check(parser, type))
    {
        advance(parser);
        return 1;
    }
    return 0;
}

static void consume(Parser *parser, TokenType type, const char *error_msg)
{
    if (parser->current_token.type == type)
    {
        advance(parser);
        return;
    }

    parser_error(parser, error_msg);
    // Sincronizar para evitar loop infinito
    synchronize(parser);
}

static void parser_error(Parser *parser, const char *message)
{
    if (parser->panic_mode)
        return;

    parser->panic_mode = 1;
    parser->had_error = 1;

    snprintf(parser->error_msg, sizeof(parser->error_msg),
             "[Linha %d, Coluna %d] Erro: %s",
             parser->current_token.line,
             parser->current_token.column,
             message);

    fprintf(stderr, "%s\n", parser->error_msg);
}

static void synchronize(Parser *parser)
{
    parser->panic_mode = 0;

    while (parser->current_token.type != TOKEN_EOF)
    {
        if (parser->previous_token.type == TOKEN_SEMICOLON)
            return;

        switch (parser->current_token.type)
        {
        case TOKEN_LET:
        case TOKEN_FN:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_RETURN:
            return;
        default:
            break;
        }

        advance(parser);
    }
}

/* --- FUNÇÕES DE CONSTRUÇÃO DE NÓS --- */

static ASTNode *make_node(NodeType type, int line, int col)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node)
        return NULL;

    node->node_type = type;
    node->data_type = TYPE_INVALID;
    node->line = line;
    node->column = col;

    return node;
}

static ASTNode *make_var_decl_node(char *name, ASTNode *type, ASTNode *initializer, int line, int col)
{
    ASTNode *node = make_node(NODE_VAR_DECL, line, col);
    if (!node)
        return NULL;

    node->data.var_decl.name = name;
    node->data.var_decl.type_node = type;
    node->data.var_decl.initializer = initializer;

    return node;
}

static ASTNode *make_func_decl_node(char *name, ASTNode **params, int param_count,
                                    ASTNode *return_type, ASTNode *body, int line, int col)
{
    ASTNode *node = make_node(NODE_FUNC_DECL, line, col);
    if (!node)
        return NULL;

    node->data.func_decl.name = name;
    node->data.func_decl.params = params;
    node->data.func_decl.param_count = param_count;
    node->data.func_decl.return_type = return_type;
    node->data.func_decl.body = body;

    return node;
}

static ASTNode *make_param_node(char *name, ASTNode *type, int line, int col)
{
    ASTNode *node = make_node(NODE_PARAM, line, col);
    if (!node)
        return NULL;

    node->data.param.name = name;
    node->data.param.type_node = type;

    return node;
}

static ASTNode *make_if_node(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch, int line, int col)
{
    ASTNode *node = make_node(NODE_IF_STMT, line, col);
    if (!node)
        return NULL;

    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_branch = then_branch;
    node->data.if_stmt.else_branch = else_branch;

    return node;
}

static ASTNode *make_while_node(ASTNode *condition, ASTNode *body, int line, int col)
{
    ASTNode *node = make_node(NODE_WHILE_STMT, line, col);
    if (!node)
        return NULL;

    node->data.while_stmt.condition = condition;
    node->data.while_stmt.body = body;

    return node;
}

static ASTNode *make_return_node(ASTNode *value, int line, int col)
{
    ASTNode *node = make_node(NODE_RETURN_STMT, line, col);
    if (!node)
        return NULL;

    node->data.return_stmt.value = value;

    return node;
}

static ASTNode *make_block_node(ASTNode **statements, int count, int line, int col)
{
    ASTNode *node = make_node(NODE_BLOCK, line, col);
    if (!node)
        return NULL;

    node->data.block.statements = statements;
    node->data.block.stmt_count = count;

    return node;
}

static ASTNode *make_binary_node(TokenType operator, ASTNode *left, ASTNode *right, int line, int col)
{
    ASTNode *node = make_node(NODE_BINARY_EXPR, line, col);
    if (!node)
        return NULL;

    node->data.binary_expr.operator = operator;
    node->data.binary_expr.left = left;
    node->data.binary_expr.right = right;

    return node;
}

static ASTNode *make_unary_node(TokenType operator, ASTNode *operand, int line, int col)
{
    ASTNode *node = make_node(NODE_UNARY_EXPR, line, col);
    if (!node)
        return NULL;

    node->data.unary_expr.operator = operator;
    node->data.unary_expr.operand = operand;

    return node;
}

static ASTNode *make_assign_node(char *name, ASTNode *value, int line, int col)
{
    ASTNode *node = make_node(NODE_ASSIGN_EXPR, line, col);
    if (!node)
        return NULL;

    node->data.assign_expr.variable_name = name;
    node->data.assign_expr.value = value;

    return node;
}

static ASTNode *make_call_node(char *name, ASTNode **args, int arg_count, int line, int col)
{
    ASTNode *node = make_node(NODE_CALL_EXPR, line, col);
    if (!node)
        return NULL;

    node->data.call_expr.function_name = name;
    node->data.call_expr.arguments = args;
    node->data.call_expr.arg_count = arg_count;

    return node;
}

static ASTNode *make_var_node(char *name, int line, int col)
{
    ASTNode *node = make_node(NODE_VAR_EXPR, line, col);
    if (!node)
        return NULL;

    node->data.var_expr.name = name;

    return node;
}

static ASTNode *make_literal_node(Token *token)
{
    ASTNode *node = make_node(NODE_LITERAL, token->line, token->column);
    if (!node)
        return NULL;

    node->data.literal.literal_type = token->type;

    switch (token->type)
    {
    case TOKEN_INT_LITERAL:
        node->data.literal.value.int_value = atoi(token->lexeme);
        node->data_type = TYPE_INT;
        break;
    case TOKEN_FLOAT_LITERAL:
        node->data.literal.value.float_value = atof(token->lexeme);
        node->data_type = TYPE_FLOAT;
        break;
    case TOKEN_STRING_LITERAL:
        // Remove aspas da string literal
        node->data.literal.value.string_value = strdup(token->lexeme + 1);
        node->data.literal.value.string_value[strlen(node->data.literal.value.string_value) - 1] = '\0';
        node->data_type = TYPE_STRING;
        break;
    case TOKEN_TRUE:
        node->data.literal.value.bool_value = 1;
        node->data_type = TYPE_BOOL;
        break;
    case TOKEN_FALSE:
        node->data.literal.value.bool_value = 0;
        node->data_type = TYPE_BOOL;
        break;
    default:
        free(node);
        return NULL;
    }

    return node;
}

static ASTNode *make_type_node(DataType type, int line, int col)
{
    ASTNode *node = make_node(NODE_TYPE, line, col);
    if (!node)
        return NULL;

    node->data.type_node.type = type;
    node->data_type = type;

    return node;
}

/* --- FUNÇÕES DE PARSING RECURSIVO --- */

// Declarações antecipadas
static ASTNode *parse_declaration(Parser *parser);
static ASTNode *parse_statement(Parser *parser);
static ASTNode *parse_expression(Parser *parser);
static ASTNode *parse_assignment(Parser *parser);
static ASTNode *parse_equality(Parser *parser);
static ASTNode *parse_comparison(Parser *parser);
static ASTNode *parse_term(Parser *parser);
static ASTNode *parse_factor(Parser *parser);
static ASTNode *parse_unary(Parser *parser);
static ASTNode *parse_primary(Parser *parser);

static ASTNode *parse_type(Parser *parser)
{
    DataType type = TYPE_INVALID;
    int line = parser->current_token.line;
    int col = parser->current_token.column;

    switch (parser->current_token.type)
    {
    case TOKEN_INT:
        type = TYPE_INT;
        break;
    case TOKEN_FLOAT:
        type = TYPE_FLOAT;
        break;
    case TOKEN_STRING:
        type = TYPE_STRING;
        break;
    case TOKEN_BOOL:
        type = TYPE_BOOL;
        break;
    case TOKEN_VOID:
        type = TYPE_VOID;
        break;
    default:
        parser_error(parser, "Esperado tipo válido");
        return NULL;
    }

    advance(parser);
    return make_type_node(type, line, col);
}

static ASTNode *parse_variable_declaration(Parser *parser)
{
    consume(parser, TOKEN_IDENTIFIER, "Esperado nome da variável");
    char *name = strdup(parser->previous_token.lexeme);
    int line = parser->previous_token.line;
    int col = parser->previous_token.column;

    consume(parser, TOKEN_COLON, "Esperado ':' após nome da variável");
    ASTNode *type_node = parse_type(parser);
    if (!type_node)
    {
        free(name);
        return NULL;
    }

    consume(parser, TOKEN_EQUAL, "Esperado '=' após tipo");
    ASTNode *initializer = parse_expression(parser);
    if (!initializer)
    {
        ast_free(type_node);
        free(name);
        return NULL;
    }

    consume(parser, TOKEN_SEMICOLON, "Esperado ';' após declaração de variável");

    return make_var_decl_node(name, type_node, initializer, line, col);
}

static ASTNode **parse_parameters(Parser *parser, int *count)
{
    *count = 0;
    ASTNode **params = NULL;
    int capacity = 0;

    while (!check(parser, TOKEN_RIGHT_PAREN) && !check(parser, TOKEN_EOF))
    {
        if (*count > 0)
        {
            consume(parser, TOKEN_COMMA, "Esperado ',' entre parâmetros");
        }

        consume(parser, TOKEN_IDENTIFIER, "Esperado nome do parâmetro");
        char *param_name = strdup(parser->previous_token.lexeme);
        int line = parser->previous_token.line;
        int col = parser->previous_token.column;

        consume(parser, TOKEN_COLON, "Esperado ':' após nome do parâmetro");
        ASTNode *param_type = parse_type(parser);
        if (!param_type)
        {
            free(param_name);
            // Cleanup já alocados
            for (int i = 0; i < *count; i++)
            {
                ast_free(params[i]);
            }
            free(params);
            return NULL;
        }

        // Expandir array se necessário
        if (*count >= capacity)
        {
            capacity = capacity == 0 ? 4 : capacity * 2;
            params = realloc(params, sizeof(ASTNode *) * capacity);
        }

        params[(*count)++] = make_param_node(param_name, param_type, line, col);
    }

    return params;
}

static ASTNode *parse_function_declaration(Parser *parser)
{
    consume(parser, TOKEN_IDENTIFIER, "Esperado nome da função");
    char *name = strdup(parser->previous_token.lexeme);
    int line = parser->previous_token.line;
    int col = parser->previous_token.column;

    consume(parser, TOKEN_LEFT_PAREN, "Esperado '(' após nome da função");

    int param_count = 0;
    ASTNode **params = parse_parameters(parser, &param_count);

    consume(parser, TOKEN_RIGHT_PAREN, "Esperado ')' após parâmetros");
    consume(parser, TOKEN_COLON, "Esperado ':' após parâmetros");

    ASTNode *return_type = parse_type(parser);
    if (!return_type)
    {
        free(name);
        for (int i = 0; i < param_count; i++)
        {
            ast_free(params[i]);
        }
        free(params);
        return NULL;
    }

    ASTNode *body = parse_block(parser);
    if (!body)
    {
        free(name);
        ast_free(return_type);
        for (int i = 0; i < param_count; i++)
        {
            ast_free(params[i]);
        }
        free(params);
        return NULL;
    }

    return make_func_decl_node(name, params, param_count, return_type, body, line, col);
}

static ASTNode *parse_block(Parser *parser)
{
    consume(parser, TOKEN_LEFT_BRACE, "Esperado '{'");

    ASTNode **statements = NULL;
    int stmt_count = 0;
    int capacity = 0;
    int line = parser->previous_token.line;
    int col = parser->previous_token.column;

    while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF))
    {
        ASTNode *stmt = parse_declaration(parser);
        if (stmt)
        {
            if (stmt_count >= capacity)
            {
                capacity = capacity == 0 ? 8 : capacity * 2;
                statements = realloc(statements, sizeof(ASTNode *) * capacity);
            }
            statements[stmt_count++] = stmt;
        }
        else
        {
            synchronize(parser);
        }
    }

    consume(parser, TOKEN_RIGHT_BRACE, "Esperado '}'");

    return make_block_node(statements, stmt_count, line, col);
}

static ASTNode *parse_if_statement(Parser *parser)
{
    int line = parser->previous_token.line;
    int col = parser->previous_token.column;

    consume(parser, TOKEN_LEFT_PAREN, "Esperado '(' após 'if'");
    ASTNode *condition = parse_expression(parser);
    if (!condition)
        return NULL;

    consume(parser, TOKEN_RIGHT_PAREN, "Esperado ')' após condição");

    ASTNode *then_branch = parse_block(parser);
    if (!then_branch)
    {
        ast_free(condition);
        return NULL;
    }

    ASTNode *else_branch = NULL;
    if (match(parser, TOKEN_ELSE))
    {
        else_branch = parse_block(parser);
        if (!else_branch)
        {
            ast_free(condition);
            ast_free(then_branch);
            return NULL;
        }
    }

    return make_if_node(condition, then_branch, else_branch, line, col);
}

static ASTNode *parse_while_statement(Parser *parser)
{
    int line = parser->previous_token.line;
    int col = parser->previous_token.column;

    consume(parser, TOKEN_LEFT_PAREN, "Esperado '(' após 'while'");
    ASTNode *condition = parse_expression(parser);
    if (!condition)
        return NULL;

    consume(parser, TOKEN_RIGHT_PAREN, "Esperado ')' após condição");

    ASTNode *body = parse_block(parser);
    if (!body)
    {
        ast_free(condition);
        return NULL;
    }

    return make_while_node(condition, body, line, col);
}

static ASTNode *parse_return_statement(Parser *parser)
{
    int line = parser->previous_token.line;
    int col = parser->previous_token.column;

    ASTNode *value = NULL;
    if (!check(parser, TOKEN_SEMICOLON))
    {
        value = parse_expression(parser);
        if (!value)
            return NULL;
    }

    consume(parser, TOKEN_SEMICOLON, "Esperado ';' após return");

    return make_return_node(value, line, col);
}

static ASTNode *parse_expression_statement(Parser *parser)
{
    ASTNode *expr = parse_expression(parser);
    if (!expr)
        return NULL;

    consume(parser, TOKEN_SEMICOLON, "Esperado ';' após expressão");

    ASTNode *node = make_node(NODE_EXPR_STMT, expr->line, expr->column);
    if (!node)
    {
        ast_free(expr);
        return NULL;
    }

    // Armazenar a expressão diretamente
    node->data.expr_stmt.expression = expr;

    return node;
}

static ASTNode *parse_statement(Parser *parser)
{
    if (match(parser, TOKEN_IF))
    {
        return parse_if_statement(parser);
    }
    else if (match(parser, TOKEN_WHILE))
    {
        return parse_while_statement(parser);
    }
    else if (match(parser, TOKEN_RETURN))
    {
        return parse_return_statement(parser);
    }
    else if (check(parser, TOKEN_LEFT_BRACE))
    {
        return parse_block(parser);
    }
    else
    {
        return parse_expression_statement(parser);
    }
}

static ASTNode *parse_declaration(Parser *parser)
{
    if (match(parser, TOKEN_LET))
    {
        return parse_variable_declaration(parser);
    }
    else if (match(parser, TOKEN_FN))
    {
        return parse_function_declaration(parser);
    }
    else
    {
        return parse_statement(parser);
    }
}

static ASTNode **parse_argument_list(Parser *parser, int *count)
{
    *count = 0;
    ASTNode **args = NULL;
    int capacity = 0;

    while (!check(parser, TOKEN_RIGHT_PAREN) && !check(parser, TOKEN_EOF))
    {
        if (*count > 0)
        {
            consume(parser, TOKEN_COMMA, "Esperado ',' entre argumentos");
        }

        ASTNode *arg = parse_expression(parser);
        if (!arg)
        {
            // Cleanup
            for (int i = 0; i < *count; i++)
            {
                ast_free(args[i]);
            }
            free(args);
            return NULL;
        }

        if (*count >= capacity)
        {
            capacity = capacity == 0 ? 4 : capacity * 2;
            args = realloc(args, sizeof(ASTNode *) * capacity);
        }

        args[(*count)++] = arg;
    }

    return args;
}

static ASTNode *parse_primary(Parser *parser)
{
    if (match(parser, TOKEN_TRUE) || match(parser, TOKEN_FALSE) ||
        match(parser, TOKEN_INT_LITERAL) || match(parser, TOKEN_FLOAT_LITERAL) ||
        match(parser, TOKEN_STRING_LITERAL))
    {
        return make_literal_node(&parser->previous_token);
    }

    if (match(parser, TOKEN_IDENTIFIER))
    {
        char *name = strdup(parser->previous_token.lexeme);
        int line = parser->previous_token.line;
        int col = parser->previous_token.column;

        // Verificar se é chamada de função
        if (check(parser, TOKEN_LEFT_PAREN))
        {
            advance(parser); // consome '('

            int arg_count = 0;
            ASTNode **args = parse_argument_list(parser, &arg_count);

            consume(parser, TOKEN_RIGHT_PAREN, "Esperado ')' após argumentos");

            return make_call_node(name, args, arg_count, line, col);
        }

        return make_var_node(name, line, col);
    }

    if (match(parser, TOKEN_LEFT_PAREN))
    {
        ASTNode *expr = parse_expression(parser);
        consume(parser, TOKEN_RIGHT_PAREN, "Esperado ')' após expressão");
        return expr;
    }

    parser_error(parser, "Expressão esperada");
    return NULL;
}

static ASTNode *parse_unary(Parser *parser)
{
    if (match(parser, TOKEN_MINUS))
    {
        TokenType operator = parser->previous_token.type;
        int line = parser->previous_token.line;
        int col = parser->previous_token.column;

        ASTNode *operand = parse_unary(parser);
        if (!operand)
            return NULL;

        return make_unary_node(operator, operand, line, col);
    }

    return parse_primary(parser);
}

static ASTNode *parse_factor(Parser *parser)
{
    ASTNode *expr = parse_unary(parser);

    while (match(parser, TOKEN_STAR) || match(parser, TOKEN_SLASH))
    {
        TokenType operator = parser->previous_token.type;
        int line = parser->previous_token.line;
        int col = parser->previous_token.column;

        ASTNode *right = parse_unary(parser);
        if (!right)
        {
            ast_free(expr);
            return NULL;
        }

        expr = make_binary_node(operator, expr, right, line, col);
    }

    return expr;
}

static ASTNode *parse_term(Parser *parser)
{
    ASTNode *expr = parse_factor(parser);

    while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS))
    {
        TokenType operator = parser->previous_token.type;
        int line = parser->previous_token.line;
        int col = parser->previous_token.column;

        ASTNode *right = parse_factor(parser);
        if (!right)
        {
            ast_free(expr);
            return NULL;
        }

        expr = make_binary_node(operator, expr, right, line, col);
    }

    return expr;
}

static ASTNode *parse_comparison(Parser *parser)
{
    ASTNode *expr = parse_term(parser);

    while (match(parser, TOKEN_GREATER) || match(parser, TOKEN_GREATER_EQUAL) ||
           match(parser, TOKEN_LESS) || match(parser, TOKEN_LESS_EQUAL))
    {
        TokenType operator = parser->previous_token.type;
        int line = parser->previous_token.line;
        int col = parser->previous_token.column;

        ASTNode *right = parse_term(parser);
        if (!right)
        {
            ast_free(expr);
            return NULL;
        }

        expr = make_binary_node(operator, expr, right, line, col);
    }

    return expr;
}

static ASTNode *parse_equality(Parser *parser)
{
    ASTNode *expr = parse_comparison(parser);

    while (match(parser, TOKEN_EQUAL_EQUAL) || match(parser, TOKEN_BANG_EQUAL))
    {
        TokenType operator = parser->previous_token.type;
        int line = parser->previous_token.line;
        int col = parser->previous_token.column;

        ASTNode *right = parse_comparison(parser);
        if (!right)
        {
            ast_free(expr);
            return NULL;
        }

        expr = make_binary_node(operator, expr, right, line, col);
    }

    return expr;
}

static ASTNode *parse_assignment(Parser *parser)
{
    ASTNode *expr = parse_equality(parser);

    if (match(parser, TOKEN_EQUAL))
    {
        if (expr->node_type != NODE_VAR_EXPR)
        {
            parser_error(parser, "Lado esquerdo da atribuição deve ser uma variável");
            ast_free(expr);
            return NULL;
        }

        char *var_name = strdup(expr->data.var_expr.name);
        int line = parser->previous_token.line;
        int col = parser->previous_token.column;
        ast_free(expr);

        ASTNode *value = parse_assignment(parser);
        if (!value)
        {
            free(var_name);
            return NULL;
        }

        return make_assign_node(var_name, value, line, col);
    }

    return expr;
}

static ASTNode *parse_expression(Parser *parser)
{
    return parse_assignment(parser);
}

/* --- FUNÇÕES PÚBLICAS --- */

void parser_init(Parser *parser, Lexer *lexer)
{
    parser->lexer = lexer;
    parser->had_error = 0;
    parser->panic_mode = 0;
    parser->error_msg[0] = '\0';

    // Inicializar tokens
    parser->current_token.lexeme = NULL;
    parser->previous_token.lexeme = NULL;

    // Avançar para o primeiro token
    advance(parser);
}

void parser_cleanup(Parser *parser)
{
    token_free(&parser->current_token);
    token_free(&parser->previous_token);
}

ASTNode *parse_program(Parser *parser)
{
    ASTNode **declarations = NULL;
    int decl_count = 0;
    int capacity = 0;

    while (!check(parser, TOKEN_EOF))
    {
        ASTNode *decl = parse_declaration(parser);
        if (decl)
        {
            if (decl_count >= capacity)
            {
                capacity = capacity == 0 ? 8 : capacity * 2;
                declarations = realloc(declarations, sizeof(ASTNode *) * capacity);
            }
            declarations[decl_count++] = decl;
        }
        else
        {
            synchronize(parser);
        }
    }

    return make_block_node(declarations, decl_count, 1, 1);
}

/* --- FUNÇÕES DE UTILIDADE --- */

void ast_free(ASTNode *node)
{
    if (!node)
        return;

    switch (node->node_type)
    {
    case NODE_VAR_DECL:
        free(node->data.var_decl.name);
        ast_free(node->data.var_decl.type_node);
        ast_free(node->data.var_decl.initializer);
        break;

    case NODE_FUNC_DECL:
        free(node->data.func_decl.name);
        for (int i = 0; i < node->data.func_decl.param_count; i++)
        {
            ast_free(node->data.func_decl.params[i]);
        }
        free(node->data.func_decl.params);
        ast_free(node->data.func_decl.return_type);
        ast_free(node->data.func_decl.body);
        break;

    case NODE_PARAM:
        free(node->data.param.name);
        ast_free(node->data.param.type_node);
        break;

    case NODE_IF_STMT:
        ast_free(node->data.if_stmt.condition);
        ast_free(node->data.if_stmt.then_branch);
        ast_free(node->data.if_stmt.else_branch);
        break;

    case NODE_WHILE_STMT:
        ast_free(node->data.while_stmt.condition);
        ast_free(node->data.while_stmt.body);
        break;

    case NODE_RETURN_STMT:
        ast_free(node->data.return_stmt.value);
        break;

    case NODE_EXPR_STMT:
        ast_free(node->data.expr_stmt.expression);
        break;

    case NODE_BLOCK:
        for (int i = 0; i < node->data.block.stmt_count; i++)
        {
            ast_free(node->data.block.statements[i]);
        }
        free(node->data.block.statements);
        break;

    case NODE_BINARY_EXPR:
        ast_free(node->data.binary_expr.left);
        ast_free(node->data.binary_expr.right);
        break;

    case NODE_UNARY_EXPR:
        ast_free(node->data.unary_expr.operand);
        break;

    case NODE_ASSIGN_EXPR:
        free(node->data.assign_expr.variable_name);
        ast_free(node->data.assign_expr.value);
        break;

    case NODE_CALL_EXPR:
        free(node->data.call_expr.function_name);
        for (int i = 0; i < node->data.call_expr.arg_count; i++)
        {
            ast_free(node->data.call_expr.arguments[i]);
        }
        free(node->data.call_expr.arguments);
        break;

    case NODE_VAR_EXPR:
        free(node->data.var_expr.name);
        break;

    case NODE_LITERAL:
        if (node->data.literal.literal_type == TOKEN_STRING_LITERAL)
        {
            free(node->data.literal.value.string_value);
        }
        break;

    case NODE_TYPE:
        // Sem cleanup necessário
        break;

    default:
        break;
    }

    free(node);
}

void ast_print(ASTNode *node, int indent)
{
    if (!node)
        return;

    for (int i = 0; i < indent; i++)
    {
        printf("  ");
    }

    switch (node->node_type)
    {
    case NODE_VAR_DECL:
        printf("VAR_DECL: %s:%s = \n",
               node->data.var_decl.name,
               data_type_to_string(node->data.var_decl.type_node->data.type_node.type));
        ast_print(node->data.var_decl.initializer, indent + 1);
        break;

    case NODE_FUNC_DECL:
        printf("FUNC_DECL: %s(", node->data.func_decl.name);
        for (int i = 0; i < node->data.func_decl.param_count; i++)
        {
            if (i > 0)
                printf(", ");
            printf("%s:%s",
                   node->data.func_decl.params[i]->data.param.name,
                   data_type_to_string(node->data.func_decl.params[i]->data.param.type_node->data.type_node.type));
        }
        printf(") -> %s\n", data_type_to_string(node->data.func_decl.return_type->data.type_node.type));
        ast_print(node->data.func_decl.body, indent + 1);
        break;

    case NODE_IF_STMT:
        printf("IF\n");
        for (int i = 0; i < indent + 1; i++)
            printf("  ");
        printf("CONDITION:\n");
        ast_print(node->data.if_stmt.condition, indent + 2);
        for (int i = 0; i < indent + 1; i++)
            printf("  ");
        printf("THEN:\n");
        ast_print(node->data.if_stmt.then_branch, indent + 2);
        if (node->data.if_stmt.else_branch)
        {
            for (int i = 0; i < indent + 1; i++)
                printf("  ");
            printf("ELSE:\n");
            ast_print(node->data.if_stmt.else_branch, indent + 2);
        }
        break;

    case NODE_WHILE_STMT:
        printf("WHILE\n");
        for (int i = 0; i < indent + 1; i++)
            printf("  ");
        printf("CONDITION:\n");
        ast_print(node->data.while_stmt.condition, indent + 2);
        for (int i = 0; i < indent + 1; i++)
            printf("  ");
        printf("BODY:\n");
        ast_print(node->data.while_stmt.body, indent + 2);
        break;

    case NODE_RETURN_STMT:
        printf("RETURN\n");
        if (node->data.return_stmt.value)
        {
            ast_print(node->data.return_stmt.value, indent + 1);
        }
        break;

    case NODE_BLOCK:
        printf("BLOCK\n");
        for (int i = 0; i < node->data.block.stmt_count; i++)
        {
            ast_print(node->data.block.statements[i], indent + 1);
        }
        break;

    case NODE_EXPR_STMT:
        printf("EXPR_STMT\n");
        ast_print(node->data.block.statements[0], indent + 1);
        break;

    case NODE_BINARY_EXPR:
        printf("BINARY_OP(%s)\n", token_type_to_string(node->data.binary_expr.operator));
        ast_print(node->data.binary_expr.left, indent + 1);
        ast_print(node->data.binary_expr.right, indent + 1);
        break;

    case NODE_UNARY_EXPR:
        printf("UNARY_OP(%s)\n", token_type_to_string(node->data.unary_expr.operator));
        ast_print(node->data.unary_expr.operand, indent + 1);
        break;

    case NODE_ASSIGN_EXPR:
        printf("ASSIGN: %s =\n", node->data.assign_expr.variable_name);
        ast_print(node->data.assign_expr.value, indent + 1);
        break;

    case NODE_CALL_EXPR:
        printf("CALL: %s\n", node->data.call_expr.function_name);
        for (int i = 0; i < node->data.call_expr.arg_count; i++)
        {
            for (int j = 0; j < indent + 1; j++)
                printf("  ");
            printf("ARG %d:\n", i);
            ast_print(node->data.call_expr.arguments[i], indent + 2);
        }
        break;

    case NODE_VAR_EXPR:
        printf("VAR: %s\n", node->data.var_expr.name);
        break;

    case NODE_LITERAL:
        printf("LITERAL: ");
        switch (node->data.literal.literal_type)
        {
        case TOKEN_INT_LITERAL:
            printf("%d\n", node->data.literal.value.int_value);
            break;
        case TOKEN_FLOAT_LITERAL:
            printf("%.6f\n", node->data.literal.value.float_value);
            break;
        case TOKEN_STRING_LITERAL:
            printf("\"%s\"\n", node->data.literal.value.string_value);
            break;
        case TOKEN_TRUE:
        case TOKEN_FALSE:
            printf("%s\n", node->data.literal.value.bool_value ? "true" : "false");
            break;
        default:
            printf("unknown\n");
            break;
        }
        break;

    default:
        printf("UNKNOWN_NODE\n");
        break;
    }
}

const char *node_type_to_string(NodeType type)
{
    switch (type)
    {
    case NODE_VAR_DECL:
        return "VAR_DECL";
    case NODE_FUNC_DECL:
        return "FUNC_DECL";
    case NODE_PARAM:
        return "PARAM";
    case NODE_EXPR_STMT:
        return "EXPR_STMT";
    case NODE_IF_STMT:
        return "IF_STMT";
    case NODE_WHILE_STMT:
        return "WHILE_STMT";
    case NODE_RETURN_STMT:
        return "RETURN_STMT";
    case NODE_BLOCK:
        return "BLOCK";
    case NODE_ASSIGN_EXPR:
        return "ASSIGN_EXPR";
    case NODE_BINARY_EXPR:
        return "BINARY_EXPR";
    case NODE_UNARY_EXPR:
        return "UNARY_EXPR";
    case NODE_CALL_EXPR:
        return "CALL_EXPR";
    case NODE_VAR_EXPR:
        return "VAR_EXPR";
    case NODE_LITERAL_EXPR:
        return "LITERAL_EXPR";
    case NODE_TYPE:
        return "TYPE";
    case NODE_LITERAL:
        return "LITERAL";
    default:
        return "UNKNOWN";
    }
}

const char *data_type_to_string(DataType type)
{
    switch (type)
    {
    case TYPE_VOID:
        return "void";
    case TYPE_INT:
        return "int";
    case TYPE_FLOAT:
        return "float";
    case TYPE_STRING:
        return "string";
    case TYPE_BOOL:
        return "bool";
    case TYPE_INVALID:
        return "invalid";
    default:
        return "unknown";
    }
}

/* Nota: Funções da tabela de símbolos movidas para craze_semantic.c */
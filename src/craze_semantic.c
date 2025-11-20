#include "../include/craze_semantic.h"

/* Para strdup no MinGW */
#ifdef _WIN32
#endif

/* --- DECLARAÇÕES ANTECIPADAS --- */
static void semantic_error(SemanticAnalyzer *analyzer, int line, int column, const char *format, ...);
static void semantic_warning(SemanticAnalyzer *analyzer, int line, int column, const char *format, ...);

static Scope *scope_create(ScopeType type, Scope *parent);
static void scope_destroy(Scope *scope);
static void enter_scope(SemanticAnalyzer *analyzer, ScopeType type);
static void exit_scope(SemanticAnalyzer *analyzer);

static SymbolEntry *symbol_lookup(SemanticAnalyzer *analyzer, const char *name);
static SymbolEntry *symbol_lookup_current(SemanticAnalyzer *analyzer, const char *name);
static int symbol_insert(SemanticAnalyzer *analyzer, SymbolEntry *entry);

static void visit_node(SemanticAnalyzer *analyzer, ASTNode *node);
static TypeCheckResult check_expression(SemanticAnalyzer *analyzer, ASTNode *node);

/* --- FUNÇÕES DE UTILIDADE PARA TIPOS --- */

TypeInfo *typeinfo_create(DataType base_type)
{
    TypeInfo *type_info = malloc(sizeof(TypeInfo));
    if (!type_info)
        return NULL;

    type_info->base_type = base_type;
    type_info->is_array = 0;
    type_info->is_const = 0;
    type_info->inner = NULL;

    return type_info;
}

void typeinfo_free(TypeInfo *type_info)
{
    if (!type_info)
        return;

    if (type_info->inner)
    {
        typeinfo_free(type_info->inner);
    }

    free(type_info);
}

TypeInfo *typeinfo_copy(TypeInfo *type_info)
{
    if (!type_info)
        return NULL;

    TypeInfo *copy = typeinfo_create(type_info->base_type);
    if (!copy)
        return NULL;

    copy->is_array = type_info->is_array;
    copy->is_const = type_info->is_const;
    if (type_info->inner)
    {
        copy->inner = typeinfo_copy(type_info->inner);
    }

    return copy;
}

const char *typeinfo_to_string(TypeInfo *type_info)
{
    if (!type_info)
        return "unknown";

    switch (type_info->base_type)
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

/* --- FUNÇÕES DE GESTÃO DE ESCOPOS --- */

static Scope *scope_create(ScopeType type, Scope *parent)
{
    Scope *scope = malloc(sizeof(Scope));
    if (!scope)
        return NULL;

    scope->symbols = NULL;
    scope->parent = parent;
    scope->depth = parent ? parent->depth + 1 : 0;
    scope->scope_type = type;

    return scope;
}

static void scope_destroy(Scope *scope)
{
    if (!scope)
        return;

    // Liberar todos os símbolos
    SymbolEntry *current = scope->symbols;
    while (current)
    {
        SymbolEntry *next = current->next;

        free(current->name);
        typeinfo_free(current->type);

        if (current->category == SYMBOL_FUNCTION)
        {
            for (int i = 0; i < current->details.func_info.param_count; i++)
            {
                // Os parâmetros serão liberados quando seus escopos forem destruídos
            }
            free(current->details.func_info.parameters);
            typeinfo_free(current->details.func_info.return_type);
        }

        free(current);
        current = next;
    }

    free(scope);
}

static void enter_scope(SemanticAnalyzer *analyzer, ScopeType type)
{
    Scope *new_scope = scope_create(type, analyzer->symbol_table->current_scope);
    if (!new_scope)
        return;

    analyzer->symbol_table->current_scope = new_scope;
    analyzer->symbol_table->scope_count++;
}

static void exit_scope(SemanticAnalyzer *analyzer)
{
    if (!analyzer->symbol_table->current_scope)
        return;

    Scope *old_scope = analyzer->symbol_table->current_scope;
    analyzer->symbol_table->current_scope = old_scope->parent;

    scope_destroy(old_scope);
    analyzer->symbol_table->scope_count--;
}

/* --- FUNÇÕES DE GESTÃO DE SÍMBOLOS --- */

static SymbolEntry *symbol_lookup(SemanticAnalyzer *analyzer, const char *name)
{
    Scope *current = analyzer->symbol_table->current_scope;

    while (current)
    {
        SymbolEntry *symbol = current->symbols;
        while (symbol)
        {
            if (strcmp(symbol->name, name) == 0)
            {
                return symbol;
            }
            symbol = symbol->next;
        }
        current = current->parent;
    }

    return NULL;
}

static SymbolEntry *symbol_lookup_current(SemanticAnalyzer *analyzer, const char *name)
{
    if (!analyzer->symbol_table->current_scope)
        return NULL;

    SymbolEntry *symbol = analyzer->symbol_table->current_scope->symbols;
    while (symbol)
    {
        if (strcmp(symbol->name, name) == 0)
        {
            return symbol;
        }
        symbol = symbol->next;
    }

    return NULL;
}

static int symbol_insert(SemanticAnalyzer *analyzer, SymbolEntry *entry)
{
    if (!entry || !analyzer->symbol_table->current_scope)
        return 0;

    entry->scope_depth = analyzer->symbol_table->current_scope->depth;
    entry->next = analyzer->symbol_table->current_scope->symbols;
    analyzer->symbol_table->current_scope->symbols = entry;

    return 1;
}

static SymbolEntry *symbol_create_variable(const char *name, TypeInfo *type, int line, int col)
{
    SymbolEntry *entry = malloc(sizeof(SymbolEntry));
    if (!entry)
        return NULL;

    entry->name = strdup(name);
    entry->category = SYMBOL_VARIABLE;
    entry->type = type;
    entry->declared_line = line;
    entry->declared_column = col;
    entry->next = NULL;
    entry->details.var_info.initializer = NULL;

    return entry;
}

static SymbolEntry *symbol_create_function(const char *name, TypeInfo *return_type,
                                           SymbolEntry **params, int param_count,
                                           int line, int col)
{
    SymbolEntry *entry = malloc(sizeof(SymbolEntry));
    if (!entry)
        return NULL;

    entry->name = strdup(name);
    entry->category = SYMBOL_FUNCTION;
    entry->type = return_type; // Para funções, o tipo é o tipo de retorno
    entry->declared_line = line;
    entry->declared_column = col;
    entry->next = NULL;
    entry->details.func_info.parameters = params;
    entry->details.func_info.param_count = param_count;
    entry->details.func_info.return_type = typeinfo_copy(return_type);
    entry->details.func_info.function_node = NULL;

    return entry;
}

/* --- FUNÇÕES DE VERIFICAÇÃO DE TIPOS --- */

static int is_numeric_type(TypeInfo *type)
{
    return type && (type->base_type == TYPE_INT || type->base_type == TYPE_FLOAT);
}

static int are_types_compatible(TypeInfo *expected, TypeInfo *actual)
{
    if (!expected || !actual)
        return 0;

    // Mesmo tipo
    if (expected->base_type == actual->base_type)
        return 1;

    // Conversões numéricas permitidas: int -> float
    if (expected->base_type == TYPE_FLOAT && actual->base_type == TYPE_INT)
        return 1;

    return 0;
}

static int are_types_comparable(TypeInfo *left, TypeInfo *right)
{
    if (!left || !right)
        return 0;

    // Mesmo tipo (exceto void)
    if (left->base_type == right->base_type && left->base_type != TYPE_VOID)
        return 1;

    // Numéricos podem ser comparados entre si
    if (is_numeric_type(left) && is_numeric_type(right))
        return 1;

    return 0;
}

static int is_comparison_operator(TokenType op)
{
    return (op == TOKEN_EQUAL_EQUAL || op == TOKEN_BANG_EQUAL ||
            op == TOKEN_GREATER || op == TOKEN_LESS ||
            op == TOKEN_GREATER_EQUAL || op == TOKEN_LESS_EQUAL);
}

static TypeInfo *type_from_ast_node(ASTNode *type_node)
{
    if (!type_node || type_node->node_type != NODE_TYPE)
    {
        return typeinfo_create(TYPE_INVALID);
    }

    return typeinfo_create(type_node->data.type_node.type);
}

/* --- FUNÇÕES DE VERIFICAÇÃO DE EXPRESSÕES --- */

static TypeCheckResult check_literal_expression(SemanticAnalyzer *analyzer, ASTNode *node)
{
    (void)analyzer; // Supprime warning de parâmetro não usado
    TypeCheckResult result = {0};
    result.is_valid = 1;

    switch (node->data.literal.literal_type)
    {
    case TOKEN_INT_LITERAL:
        result.type = typeinfo_create(TYPE_INT);
        break;
    case TOKEN_FLOAT_LITERAL:
        result.type = typeinfo_create(TYPE_FLOAT);
        break;
    case TOKEN_STRING_LITERAL:
        result.type = typeinfo_create(TYPE_STRING);
        break;
    case TOKEN_TRUE:
    case TOKEN_FALSE:
        result.type = typeinfo_create(TYPE_BOOL);
        break;
    default:
        result.is_valid = 0;
        result.type = typeinfo_create(TYPE_INVALID);
        snprintf(result.error_msg, sizeof(result.error_msg), "Literal desconhecido");
        break;
    }

    return result;
}

static TypeCheckResult check_variable_expression(SemanticAnalyzer *analyzer, ASTNode *node)
{
    TypeCheckResult result = {0};

    SymbolEntry *symbol = symbol_lookup(analyzer, node->data.var_expr.name);
    if (symbol == NULL)
    {
        snprintf(result.error_msg, sizeof(result.error_msg),
                 "Variável '%s' não declarada", node->data.var_expr.name);
        semantic_error(analyzer, node->line, node->column, result.error_msg);
        result.is_valid = 0;
        result.type = typeinfo_create(TYPE_INVALID);
        return result;
    }

    result.is_valid = 1;
    result.type = typeinfo_copy(symbol->type);
    return result;
}

static TypeCheckResult check_binary_expression(SemanticAnalyzer *analyzer, ASTNode *node)
{
    TypeCheckResult left = check_expression(analyzer, node->data.binary_expr.left);
    TypeCheckResult right = check_expression(analyzer, node->data.binary_expr.right);

    TypeCheckResult result = {0};
    result.is_valid = 0;

    if (!left.is_valid || !right.is_valid)
    {
        typeinfo_free(left.type);
        typeinfo_free(right.type);
        result.type = typeinfo_create(TYPE_INVALID);
        return result;
    }

    TokenType op = node->data.binary_expr.operator;

    // Operadores aritméticos: +, -, *, /
    if (op == TOKEN_PLUS || op == TOKEN_MINUS || op == TOKEN_STAR || op == TOKEN_SLASH)
    {
        // Verificar se ambos os operandos são numéricos
        if (is_numeric_type(left.type) && is_numeric_type(right.type))
        {
            result.is_valid = 1;
            // Regra: int OP int -> int, demais casos -> float
            if (left.type->base_type == TYPE_INT && right.type->base_type == TYPE_INT &&
                op != TOKEN_SLASH)
            { // Divisão de ints pode resultar em float
                result.type = typeinfo_create(TYPE_INT);
            }
            else
            {
                result.type = typeinfo_create(TYPE_FLOAT);
                // Marcar conversão implícita se necessário
                if (left.type->base_type == TYPE_INT)
                {
                    result.implicit_conversion |= 1;
                    semantic_warning(analyzer, node->line, node->column,
                                     "Conversão implícita de int para float");
                }
                if (right.type->base_type == TYPE_INT)
                {
                    result.implicit_conversion |= 2;
                }
            }
        }
        // Operador + para strings (concatenação)
        else if (op == TOKEN_PLUS && left.type->base_type == TYPE_STRING &&
                 right.type->base_type == TYPE_STRING)
        {
            result.is_valid = 1;
            result.type = typeinfo_create(TYPE_STRING);
        }
        else
        {
            snprintf(result.error_msg, sizeof(result.error_msg),
                     "Operador '%s' não suportado para tipos %s e %s",
                     token_type_to_string(op),
                     typeinfo_to_string(left.type),
                     typeinfo_to_string(right.type));
            semantic_error(analyzer, node->line, node->column, result.error_msg);
            result.type = typeinfo_create(TYPE_INVALID);
        }
    }
    // Operadores relacionais
    else if (is_comparison_operator(op))
    {
        if (are_types_comparable(left.type, right.type))
        {
            result.is_valid = 1;
            result.type = typeinfo_create(TYPE_BOOL);
            // Marcar conversões implícitas
            if (is_numeric_type(left.type) && is_numeric_type(right.type) &&
                left.type->base_type != right.type->base_type)
            {
                result.implicit_conversion = 1;
                semantic_warning(analyzer, node->line, node->column,
                                 "Conversão implícita em comparação");
            }
        }
        else
        {
            snprintf(result.error_msg, sizeof(result.error_msg),
                     "Tipos %s e %s não são comparáveis",
                     typeinfo_to_string(left.type),
                     typeinfo_to_string(right.type));
            semantic_error(analyzer, node->line, node->column, result.error_msg);
            result.type = typeinfo_create(TYPE_INVALID);
        }
    }

    typeinfo_free(left.type);
    typeinfo_free(right.type);

    return result;
}

static TypeCheckResult check_unary_expression(SemanticAnalyzer *analyzer, ASTNode *node)
{
    TypeCheckResult operand = check_expression(analyzer, node->data.unary_expr.operand);
    TypeCheckResult result = {0};

    if (!operand.is_valid)
    {
        result.is_valid = 0;
        result.type = typeinfo_create(TYPE_INVALID);
        typeinfo_free(operand.type);
        return result;
    }

    TokenType op = node->data.unary_expr.operator;

    if (op == TOKEN_MINUS)
    {
        if (is_numeric_type(operand.type))
        {
            result.is_valid = 1;
            result.type = typeinfo_copy(operand.type);
        }
        else
        {
            snprintf(result.error_msg, sizeof(result.error_msg),
                     "Operador unário '-' não suportado para tipo %s",
                     typeinfo_to_string(operand.type));
            semantic_error(analyzer, node->line, node->column, result.error_msg);
            result.is_valid = 0;
            result.type = typeinfo_create(TYPE_INVALID);
        }
    }

    typeinfo_free(operand.type);
    return result;
}

static TypeCheckResult check_call_expression(SemanticAnalyzer *analyzer, ASTNode *node)
{
    TypeCheckResult result = {0};

    // Verificar se função existe
    SymbolEntry *function = symbol_lookup(analyzer, node->data.call_expr.function_name);
    if (function == NULL || function->category != SYMBOL_FUNCTION)
    {
        snprintf(result.error_msg, sizeof(result.error_msg),
                 "Função '%s' não declarada", node->data.call_expr.function_name);
        semantic_error(analyzer, node->line, node->column, result.error_msg);
        result.is_valid = 0;
        result.type = typeinfo_create(TYPE_INVALID);
        return result;
    }

    // Verificar número de argumentos (exceção para print que aceita qualquer número)
    if (strcmp(node->data.call_expr.function_name, "print") != 0 &&
        node->data.call_expr.arg_count != function->details.func_info.param_count)
    {
        snprintf(result.error_msg, sizeof(result.error_msg),
                 "Número incorreto de argumentos para '%s': esperado %d, encontrado %d",
                 node->data.call_expr.function_name,
                 function->details.func_info.param_count,
                 node->data.call_expr.arg_count);
        semantic_error(analyzer, node->line, node->column, result.error_msg);
        result.is_valid = 0;
        result.type = typeinfo_create(TYPE_INVALID);
        return result;
    }

    // Verificar tipos dos argumentos
    result.is_valid = 1;

    // Para print, apenas verificar se argumentos são válidos (aceita qualquer tipo)
    if (strcmp(node->data.call_expr.function_name, "print") == 0)
    {
        for (int i = 0; i < node->data.call_expr.arg_count; i++)
        {
            TypeCheckResult arg_result = check_expression(analyzer, node->data.call_expr.arguments[i]);
            if (!arg_result.is_valid)
            {
                result.is_valid = 0;
                result.type = typeinfo_create(TYPE_INVALID);
                return result;
            }
        }
    }
    else
    {
        // Para outras funções, verificar compatibilidade de tipos
        for (int i = 0; i < node->data.call_expr.arg_count; i++)
        {
            TypeCheckResult arg_result = check_expression(analyzer, node->data.call_expr.arguments[i]);
            SymbolEntry *param = function->details.func_info.parameters[i];

            if (!are_types_compatible(param->type, arg_result.type))
            {
                semantic_error(analyzer, node->line, node->column,
                               "Tipo incompatível para argumento %d de '%s': esperado %s, encontrado %s",
                               i + 1, node->data.call_expr.function_name,
                               typeinfo_to_string(param->type),
                               typeinfo_to_string(arg_result.type));
                result.is_valid = 0;
            }

            typeinfo_free(arg_result.type);
        }
    }

    if (result.is_valid)
    {
        result.type = typeinfo_copy(function->details.func_info.return_type);
    }
    else
    {
        result.type = typeinfo_create(TYPE_INVALID);
    }

    return result;
}

static TypeCheckResult check_assignment(SemanticAnalyzer *analyzer, ASTNode *node)
{
    TypeCheckResult result = {0};

    // Verificar se variável existe
    SymbolEntry *var = symbol_lookup(analyzer, node->data.assign_expr.variable_name);
    if (var == NULL || var->category != SYMBOL_VARIABLE)
    {
        snprintf(result.error_msg, sizeof(result.error_msg),
                 "Variável '%s' não declarada", node->data.assign_expr.variable_name);
        semantic_error(analyzer, node->line, node->column, result.error_msg);
        result.is_valid = 0;
        result.type = typeinfo_create(TYPE_INVALID);
        return result;
    }

    // Verificar valor de atribuição
    TypeCheckResult value_result = check_expression(analyzer, node->data.assign_expr.value);

    if (!value_result.is_valid)
    {
        result.is_valid = 0;
        result.type = typeinfo_create(TYPE_INVALID);
        typeinfo_free(value_result.type);
        return result;
    }

    // Verificar compatibilidade de tipos
    if (!are_types_compatible(var->type, value_result.type))
    {
        semantic_error(analyzer, node->line, node->column,
                       "Tipo incompatível na atribuição: variável %s, valor %s",
                       typeinfo_to_string(var->type),
                       typeinfo_to_string(value_result.type));
        result.is_valid = 0;
        result.type = typeinfo_create(TYPE_INVALID);
    }
    else
    {
        result.is_valid = 1;
        result.type = typeinfo_copy(var->type);
    }

    typeinfo_free(value_result.type);
    return result;
}

static TypeCheckResult check_expression(SemanticAnalyzer *analyzer, ASTNode *node)
{
    TypeCheckResult result = {0};

    if (!node)
    {
        result.is_valid = 0;
        result.type = typeinfo_create(TYPE_INVALID);
        return result;
    }

    switch (node->node_type)
    {
    case NODE_LITERAL:
        return check_literal_expression(analyzer, node);
    case NODE_VAR_EXPR:
        return check_variable_expression(analyzer, node);
    case NODE_BINARY_EXPR:
        return check_binary_expression(analyzer, node);
    case NODE_UNARY_EXPR:
        return check_unary_expression(analyzer, node);
    case NODE_CALL_EXPR:
        return check_call_expression(analyzer, node);
    case NODE_ASSIGN_EXPR:
        return check_assignment(analyzer, node);
    default:
        result.is_valid = 0;
        result.type = typeinfo_create(TYPE_INVALID);
        snprintf(result.error_msg, sizeof(result.error_msg), "Tipo de expressão não suportado");
        break;
    }

    return result;
}

/* --- FUNÇÕES DE VISITAÇÃO DA AST --- */

static void visit_variable_decl(SemanticAnalyzer *analyzer, ASTNode *node)
{
    // Verificar se variável já foi declarada no escopo atual
    if (symbol_lookup_current(analyzer, node->data.var_decl.name))
    {
        semantic_error(analyzer, node->line, node->column,
                       "Variável '%s' já declarada neste escopo",
                       node->data.var_decl.name);
        return;
    }

    // Verificar inicializador
    TypeCheckResult init_result = check_expression(analyzer, node->data.var_decl.initializer);
    TypeInfo *declared_type = type_from_ast_node(node->data.var_decl.type_node);

    if (!init_result.is_valid)
    {
        typeinfo_free(declared_type);
        typeinfo_free(init_result.type);
        return;
    }

    // Verificar compatibilidade de tipos
    if (!are_types_compatible(declared_type, init_result.type))
    {
        semantic_error(analyzer, node->line, node->column,
                       "Tipo incompatível na inicialização: declarado %s, inicializador %s",
                       typeinfo_to_string(declared_type),
                       typeinfo_to_string(init_result.type));
    }
    else
    {
        // Registrar variável na tabela de símbolos
        SymbolEntry *var_entry = symbol_create_variable(
            node->data.var_decl.name, typeinfo_copy(declared_type), node->line, node->column);
        symbol_insert(analyzer, var_entry);
    }

    typeinfo_free(declared_type);
    typeinfo_free(init_result.type);
}

static void visit_function_decl(SemanticAnalyzer *analyzer, ASTNode *node)
{
    // Verificar se função já foi declarada
    if (symbol_lookup_current(analyzer, node->data.func_decl.name))
    {
        semantic_error(analyzer, node->line, node->column,
                       "Função '%s' já declarada neste escopo",
                       node->data.func_decl.name);
        return;
    }

    // Configurar contexto da função
    analyzer->in_function = 1;
    analyzer->has_return_statement = 0;
    strncpy(analyzer->current_function, node->data.func_decl.name, sizeof(analyzer->current_function) - 1);

    // Determinar tipo de retorno
    TypeInfo *return_type = type_from_ast_node(node->data.func_decl.return_type);
    analyzer->current_return_type = return_type;

    // Criar entrada da função na tabela de símbolos
    SymbolEntry **params = malloc(sizeof(SymbolEntry *) * node->data.func_decl.param_count);
    for (int i = 0; i < node->data.func_decl.param_count; i++)
    {
        ASTNode *param = node->data.func_decl.params[i];
        TypeInfo *param_type = type_from_ast_node(param->data.param.type_node);
        params[i] = symbol_create_variable(param->data.param.name, param_type, param->line, param->column);
        params[i]->category = SYMBOL_PARAMETER;
    }

    SymbolEntry *func_entry = symbol_create_function(
        node->data.func_decl.name, return_type, params, node->data.func_decl.param_count,
        node->line, node->column);
    func_entry->details.func_info.function_node = node;
    symbol_insert(analyzer, func_entry);

    // Entrar no escopo da função
    enter_scope(analyzer, SCOPE_FUNCTION);

    // Registrar parâmetros no escopo da função
    for (int i = 0; i < node->data.func_decl.param_count; i++)
    {
        SymbolEntry *param_copy = symbol_create_variable(
            params[i]->name, typeinfo_copy(params[i]->type), params[i]->declared_line, params[i]->declared_column);
        param_copy->category = SYMBOL_PARAMETER;
        symbol_insert(analyzer, param_copy);
    }

    // Visitar corpo da função
    visit_node(analyzer, node->data.func_decl.body);

    // Verificar se função não-void tem return
    if (return_type->base_type != TYPE_VOID && !analyzer->has_return_statement)
    {
        semantic_error(analyzer, node->line, node->column,
                       "Função '%s' do tipo %s deve retornar um valor",
                       node->data.func_decl.name, typeinfo_to_string(return_type));
    }

    // Sair do escopo da função
    exit_scope(analyzer);
    analyzer->in_function = 0;
    analyzer->current_return_type = NULL;

    typeinfo_free(return_type);
}

static void visit_return_statement(SemanticAnalyzer *analyzer, ASTNode *node)
{
    if (!analyzer->in_function)
    {
        semantic_error(analyzer, node->line, node->column,
                       "Instrução 'return' fora de função");
        return;
    }

    analyzer->has_return_statement = 1;

    if (analyzer->current_return_type->base_type == TYPE_VOID)
    {
        // Verificar que return void não tem valor
        if (node->data.return_stmt.value != NULL)
        {
            semantic_error(analyzer, node->line, node->column,
                           "Função void não pode retornar um valor");
        }
    }
    else
    {
        // Verificar que return não-void tem valor
        if (node->data.return_stmt.value == NULL)
        {
            semantic_error(analyzer, node->line, node->column,
                           "Esperado valor de retorno do tipo %s",
                           typeinfo_to_string(analyzer->current_return_type));
        }
        else
        {
            // Verificar compatibilidade do tipo do valor de retorno
            TypeCheckResult result = check_expression(analyzer, node->data.return_stmt.value);
            if (result.is_valid && !are_types_compatible(analyzer->current_return_type, result.type))
            {
                semantic_error(analyzer, node->line, node->column,
                               "Tipo de retorno incompatível: esperado %s, encontrado %s",
                               typeinfo_to_string(analyzer->current_return_type),
                               typeinfo_to_string(result.type));
            }
            typeinfo_free(result.type);
        }
    }
}

static void visit_if_statement(SemanticAnalyzer *analyzer, ASTNode *node)
{
    // Verificar condição
    TypeCheckResult condition_result = check_expression(analyzer, node->data.if_stmt.condition);

    if (condition_result.is_valid && condition_result.type->base_type != TYPE_BOOL)
    {
        semantic_error(analyzer, node->line, node->column,
                       "Condição do 'if' deve ser do tipo bool, encontrado %s",
                       typeinfo_to_string(condition_result.type));
    }

    typeinfo_free(condition_result.type);

    // Visitar ramos
    visit_node(analyzer, node->data.if_stmt.then_branch);
    if (node->data.if_stmt.else_branch)
    {
        visit_node(analyzer, node->data.if_stmt.else_branch);
    }
}

static void visit_while_statement(SemanticAnalyzer *analyzer, ASTNode *node)
{
    // Verificar condição
    TypeCheckResult condition_result = check_expression(analyzer, node->data.while_stmt.condition);

    if (condition_result.is_valid && condition_result.type->base_type != TYPE_BOOL)
    {
        semantic_error(analyzer, node->line, node->column,
                       "Condição do 'while' deve ser do tipo bool, encontrado %s",
                       typeinfo_to_string(condition_result.type));
    }

    typeinfo_free(condition_result.type);

    // Visitar corpo
    visit_node(analyzer, node->data.while_stmt.body);
}

static void visit_block(SemanticAnalyzer *analyzer, ASTNode *node)
{
    enter_scope(analyzer, SCOPE_BLOCK);

    for (int i = 0; i < node->data.block.stmt_count; i++)
    {
        visit_node(analyzer, node->data.block.statements[i]);
    }

    exit_scope(analyzer);
}

static void visit_expression_statement(SemanticAnalyzer *analyzer, ASTNode *node)
{
    // Para NODE_EXPR_STMT, a expressão está no primeiro statement do bloco
    if (node->data.block.stmt_count > 0)
    {
        TypeCheckResult result = check_expression(analyzer, node->data.block.statements[0]);
        typeinfo_free(result.type);
    }
}

static void visit_node(SemanticAnalyzer *analyzer, ASTNode *node)
{
    if (!node)
        return;

    switch (node->node_type)
    {
    case NODE_VAR_DECL:
        visit_variable_decl(analyzer, node);
        break;
    case NODE_FUNC_DECL:
        visit_function_decl(analyzer, node);
        break;
    case NODE_IF_STMT:
        visit_if_statement(analyzer, node);
        break;
    case NODE_WHILE_STMT:
        visit_while_statement(analyzer, node);
        break;
    case NODE_RETURN_STMT:
        visit_return_statement(analyzer, node);
        break;
    case NODE_BLOCK:
        visit_block(analyzer, node);
        break;
    case NODE_EXPR_STMT:
        visit_expression_statement(analyzer, node);
        break;
    default:
        // Para outros tipos de nó, apenas verificar como expressão se aplicável
        break;
    }
}

/* --- FUNÇÕES DE RELATÓRIO DE ERROS --- */

static void semantic_error(SemanticAnalyzer *analyzer, int line, int column, const char *format, ...)
{
    analyzer->error_count++;

    va_list args;
    va_start(args, format);

    char message[256];
    vsnprintf(message, sizeof(message), format, args);

    fprintf(stderr, "[ERRO Semântico] Linha %d, Coluna %d: %s\n", line, column, message);

    va_end(args);
}

static void semantic_warning(SemanticAnalyzer *analyzer, int line, int column, const char *format, ...)
{
    analyzer->warning_count++;

    va_list args;
    va_start(args, format);

    char message[256];
    vsnprintf(message, sizeof(message), format, args);

    fprintf(stderr, "[AVISO Semântico] Linha %d, Coluna %d: %s\n", line, column, message);

    va_end(args);
}

/* --- FUNÇÕES PÚBLICAS --- */

void semantic_init(SemanticAnalyzer *analyzer, ASTNode *ast_root)
{
    analyzer->ast_root = ast_root;
    analyzer->error_count = 0;
    analyzer->warning_count = 0;
    analyzer->error_msg[0] = '\0';
    analyzer->current_return_type = NULL;
    analyzer->in_function = 0;
    analyzer->current_function[0] = '\0';
    analyzer->has_return_statement = 0;
    analyzer->strict_mode = 0;

    // Inicializar tabela de símbolos
    analyzer->symbol_table = malloc(sizeof(SymbolTable));
    analyzer->symbol_table->global_scope = scope_create(SCOPE_GLOBAL, NULL);
    analyzer->symbol_table->current_scope = analyzer->symbol_table->global_scope;
    analyzer->symbol_table->scope_count = 1;

    // Registrar built-ins
    semantic_register_builtins(analyzer);
}

int semantic_analyze(SemanticAnalyzer *analyzer)
{
    if (!analyzer || !analyzer->ast_root)
    {
        return 0;
    }

    visit_node(analyzer, analyzer->ast_root);

    return analyzer->error_count == 0;
}

void semantic_cleanup(SemanticAnalyzer *analyzer)
{
    if (!analyzer || !analyzer->symbol_table)
        return;

    // Voltar ao escopo global e limpar tudo
    while (analyzer->symbol_table->current_scope != analyzer->symbol_table->global_scope)
    {
        exit_scope(analyzer);
    }

    scope_destroy(analyzer->symbol_table->global_scope);
    free(analyzer->symbol_table);
    analyzer->symbol_table = NULL;
}

void semantic_print_report(SemanticAnalyzer *analyzer)
{
    if (!analyzer)
        return;

    printf("========================================\n");
    printf("     RELATÓRIO DE ANÁLISE SEMÂNTICA    \n");
    printf("========================================\n");

    if (analyzer->error_count == 0 && analyzer->warning_count == 0)
    {
        printf("✅ Análise semântica concluída sem problemas!\n");
    }
    else
    {
        if (analyzer->error_count > 0)
        {
            printf("❌ %d erro(s) semântico(s) encontrado(s)\n", analyzer->error_count);
        }
        if (analyzer->warning_count > 0)
        {
            printf("⚠️  %d aviso(s) semântico(s) encontrado(s)\n", analyzer->warning_count);
        }
    }

    printf("========================================\n");
}

void semantic_register_builtins(SemanticAnalyzer *analyzer)
{
    // Registrar função print(any): void
    SymbolEntry **print_params = malloc(sizeof(SymbolEntry *) * 1);
    print_params[0] = symbol_create_variable("value", typeinfo_create(TYPE_STRING), 0, 0);
    print_params[0]->category = SYMBOL_PARAMETER;

    SymbolEntry *print_func = symbol_create_function(
        "print", typeinfo_create(TYPE_VOID), print_params, 1, 0, 0);
    symbol_insert(analyzer, print_func);

    // Registrar função type(any): string
    SymbolEntry **type_params = malloc(sizeof(SymbolEntry *) * 1);
    type_params[0] = symbol_create_variable("value", typeinfo_create(TYPE_STRING), 0, 0);
    type_params[0]->category = SYMBOL_PARAMETER;

    SymbolEntry *type_func = symbol_create_function(
        "type", typeinfo_create(TYPE_STRING), type_params, 1, 0, 0);
    symbol_insert(analyzer, type_func);

    // Registrar função len(string): int
    SymbolEntry **len_params = malloc(sizeof(SymbolEntry *) * 1);
    len_params[0] = symbol_create_variable("text", typeinfo_create(TYPE_STRING), 0, 0);
    len_params[0]->category = SYMBOL_PARAMETER;

    SymbolEntry *len_func = symbol_create_function(
        "len", typeinfo_create(TYPE_INT), len_params, 1, 0, 0);
    symbol_insert(analyzer, len_func);
}
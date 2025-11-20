#include "../include/craze_interpreter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
#include <string.h>
#endif


Value *value_create_int(int value)
{
    Value *val = malloc(sizeof(Value));
    val->type = VAL_INT;
    val->data.int_val = value;
    val->ref_count = 1;
    return val;
}

Value *value_create_float(double value)
{
    Value *val = malloc(sizeof(Value));
    val->type = VAL_FLOAT;
    val->data.float_val = value;
    val->ref_count = 1;
    return val;
}

Value *value_create_string(const char *value)
{
    Value *val = malloc(sizeof(Value));
    val->type = VAL_STRING;
    val->data.string_val = strdup(value);
    val->ref_count = 1;
    return val;
}

Value *value_create_bool(int value)
{
    Value *val = malloc(sizeof(Value));
    val->type = VAL_BOOL;
    val->data.bool_val = value ? 1 : 0;
    val->ref_count = 1;
    return val;
}

Value *value_create_void(void)
{
    Value *val = malloc(sizeof(Value));
    val->type = VAL_VOID;
    val->ref_count = 1;
    return val;
}

Value *value_create_null(void)
{
    Value *val = malloc(sizeof(Value));
    val->type = VAL_NULL;
    val->ref_count = 1;
    return val;
}

Value *value_create_builtin(BuiltinFn function, const char *name)
{
    Value *val = malloc(sizeof(Value));
    val->type = VAL_BUILTIN_FN;
    val->data.builtin_fn.function = function;
    val->data.builtin_fn.name = strdup(name);
    val->ref_count = 1;
    return val;
}

void value_incref(Value *value)
{
    if (value != NULL)
    {
        value->ref_count++;
    }
}

void value_decref(Value *value)
{
    if (value != NULL)
    {
        value->ref_count--;
        if (value->ref_count <= 0)
        {
            value_free(value);
        }
    }
}

void value_free(Value *value)
{
    if (value == NULL)
        return;

    switch (value->type)
    {
    case VAL_STRING:
        free(value->data.string_val);
        break;
    case VAL_BUILTIN_FN:
        free(value->data.builtin_fn.name);
        break;
    default:
        break;
    }

    free(value);
}

char *value_to_string(Value *value)
{
    if (value == NULL)
    {
        return strdup("null");
    }

    char *buffer = malloc(256);
    switch (value->type)
    {
    case VAL_INT:
        snprintf(buffer, 256, "%d", value->data.int_val);
        break;
    case VAL_FLOAT:
        snprintf(buffer, 256, "%.6g", value->data.float_val);
        break;
    case VAL_STRING:
        free(buffer);
        return strdup(value->data.string_val);
    case VAL_BOOL:
        snprintf(buffer, 256, "%s", value->data.bool_val ? "true" : "false");
        break;
    case VAL_VOID:
        snprintf(buffer, 256, "void");
        break;
    case VAL_NULL:
        snprintf(buffer, 256, "null");
        break;
    case VAL_BUILTIN_FN:
        snprintf(buffer, 256, "<builtin function %s>", value->data.builtin_fn.name);
        break;
    default:
        snprintf(buffer, 256, "<unknown>");
        break;
    }
    return buffer;
}

const char *value_type_to_string(ValueType type)
{
    switch (type)
    {
    case VAL_INT:
        return "int";
    case VAL_FLOAT:
        return "float";
    case VAL_STRING:
        return "string";
    case VAL_BOOL:
        return "bool";
    case VAL_VOID:
        return "void";
    case VAL_NULL:
        return "null";
    case VAL_BUILTIN_FN:
        return "builtin_function";
    default:
        return "unknown";
    }
}

/* --- SISTEMA DE TABELA HASH --- */

static unsigned int hash_function(const char *key, int capacity)
{
    unsigned int hash = 5381;
    int c;
    while ((c = *key++))
    {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % capacity;
}

HashTable *hashtable_create(int capacity)
{
    HashTable *table = malloc(sizeof(HashTable));
    table->capacity = capacity;
    table->count = 0;
    table->buckets = calloc(capacity, sizeof(HashEntry *));
    return table;
}

void hashtable_destroy(HashTable *table)
{
    if (table == NULL)
        return;

    for (int i = 0; i < table->capacity; i++)
    {
        HashEntry *entry = table->buckets[i];
        while (entry != NULL)
        {
            HashEntry *next = entry->next;
            free(entry->key);
            value_decref(entry->value);
            free(entry);
            entry = next;
        }
    }

    free(table->buckets);
    free(table);
}

void hashtable_set(HashTable *table, const char *key, Value *value)
{
    unsigned int index = hash_function(key, table->capacity);
    HashEntry *entry = table->buckets[index];

    // Procurar entrada existente
    while (entry != NULL)
    {
        if (strcmp(entry->key, key) == 0)
        {
            // Atualizar valor existente
            value_decref(entry->value);
            entry->value = value;
            value_incref(value);
            return;
        }
        entry = entry->next;
    }

    // Criar nova entrada
    HashEntry *new_entry = malloc(sizeof(HashEntry));
    new_entry->key = strdup(key);
    new_entry->value = value;
    value_incref(value);
    new_entry->next = table->buckets[index];
    table->buckets[index] = new_entry;
    table->count++;
}

Value *hashtable_get(HashTable *table, const char *key)
{
    unsigned int index = hash_function(key, table->capacity);
    HashEntry *entry = table->buckets[index];

    while (entry != NULL)
    {
        if (strcmp(entry->key, key) == 0)
        {
            return entry->value;
        }
        entry = entry->next;
    }

    return NULL;
}

int hashtable_has(HashTable *table, const char *key)
{
    return hashtable_get(table, key) != NULL;
}

void hashtable_remove(HashTable *table, const char *key)
{
    unsigned int index = hash_function(key, table->capacity);
    HashEntry *entry = table->buckets[index];
    HashEntry *prev = NULL;

    while (entry != NULL)
    {
        if (strcmp(entry->key, key) == 0)
        {
            if (prev == NULL)
            {
                table->buckets[index] = entry->next;
            }
            else
            {
                prev->next = entry->next;
            }

            free(entry->key);
            value_decref(entry->value);
            free(entry);
            table->count--;
            return;
        }
        prev = entry;
        entry = entry->next;
    }
}

/* --- SISTEMA DE AMBIENTE --- */

Environment *environment_create(Environment *parent)
{
    Environment *env = malloc(sizeof(Environment));
    env->parent = parent;
    env->variables = hashtable_create(32);
    env->functions = hashtable_create(16);
    return env;
}

void environment_destroy(Environment *env)
{
    if (env == NULL)
        return;

    hashtable_destroy(env->variables);

    // Limpar hashtable de funções (valores são ASTNode*, não Values)
    if (env->functions)
    {
        for (int i = 0; i < env->functions->capacity; i++)
        {
            HashEntry *entry = env->functions->buckets[i];
            while (entry != NULL)
            {
                HashEntry *next = entry->next;
                free(entry->key);
                // ASTNode* não é liberado aqui (gerenciado pelo parser)
                free(entry);
                entry = next;
            }
        }
        free(env->functions->buckets);
        free(env->functions);
    }

    free(env);
}

void environment_define_var(Environment *env, const char *name, Value *value)
{
    hashtable_set(env->variables, name, value);
}

Value *environment_get_var(Environment *env, const char *name)
{
    Environment *current = env;
    while (current != NULL)
    {
        Value *value = hashtable_get(current->variables, name);
        if (value != NULL)
        {
            return value;
        }
        current = current->parent;
    }
    return NULL; // Variável não encontrada
}

int environment_assign_var(Environment *env, const char *name, Value *value)
{
    Environment *current = env;
    while (current != NULL)
    {
        if (hashtable_has(current->variables, name))
        {
            hashtable_set(current->variables, name, value);
            return 1; // Sucesso
        }
        current = current->parent;
    }
    return 0; // Variável não existe
}

void environment_define_func(Environment *env, const char *name, ASTNode *func_node)
{
    // Para funções, usamos uma hashtable separada com ASTNode*
    unsigned int index = hash_function(name, env->functions->capacity);

    // Criar entrada manual já que não temos Value* para função
    HashEntry *entry = malloc(sizeof(HashEntry));
    entry->key = strdup(name);
    entry->value = (Value *)func_node; // Cast temporário
    entry->next = env->functions->buckets[index];
    env->functions->buckets[index] = entry;
    env->functions->count++;
}

ASTNode *environment_get_func(Environment *env, const char *name)
{
    Environment *current = env;
    while (current != NULL)
    {
        unsigned int index = hash_function(name, current->functions->capacity);
        HashEntry *entry = current->functions->buckets[index];

        while (entry != NULL)
        {
            if (strcmp(entry->key, name) == 0)
            {
                return (ASTNode *)entry->value; // Cast de volta
            }
            entry = entry->next;
        }
        current = current->parent;
    }
    return NULL;
}

/* --- FUNÇÕES DE PILHA DE CHAMADAS --- */

void push_call_frame(Interpreter *interpreter, const char *function_name, int line)
{
    if (interpreter->call_stack_size >= interpreter->call_stack_capacity)
    {
        interpreter->call_stack_capacity = interpreter->call_stack_capacity == 0 ? 8 : interpreter->call_stack_capacity * 2;
        interpreter->call_stack = realloc(interpreter->call_stack,
                                          sizeof(CallFrame) * interpreter->call_stack_capacity);
    }

    CallFrame *frame = &interpreter->call_stack[interpreter->call_stack_size];
    frame->function_name = strdup(function_name);
    frame->line_number = line;
    frame->environment = interpreter->current_env;

    interpreter->call_stack_size++;
}

void pop_call_frame(Interpreter *interpreter)
{
    if (interpreter->call_stack_size > 0)
    {
        interpreter->call_stack_size--;
        CallFrame *frame = &interpreter->call_stack[interpreter->call_stack_size];
        free(frame->function_name);
    }
}

/* --- FUNÇÕES DE ERRO RUNTIME --- */

void runtime_error(Interpreter *interpreter, int line, int column, const char *format, ...)
{
    interpreter->has_runtime_error = 1;
    interpreter->error_count++;

    va_list args;
    va_start(args, format);

    char message[512];
    vsnprintf(message, sizeof(message), format, args);

    if (line > 0)
    {
        fprintf(stderr, "[ERRO Runtime] Linha %d, Coluna %d: %s\n", line, column, message);
    }
    else
    {
        fprintf(stderr, "[ERRO Runtime]: %s\n", message);
    }

    // Print call stack se disponível
    if (interpreter->call_stack_size > 0)
    {
        fprintf(stderr, "Call stack:\n");
        for (int i = interpreter->call_stack_size - 1; i >= 0; i--)
        {
            fprintf(stderr, "  [%d] %s (linha %d)\n",
                    interpreter->call_stack_size - i,
                    interpreter->call_stack[i].function_name,
                    interpreter->call_stack[i].line_number);
        }
    }

    va_end(args);

    // Copiar para buffer interno
    strncpy(interpreter->error_msg, message, sizeof(interpreter->error_msg) - 1);
    interpreter->error_msg[sizeof(interpreter->error_msg) - 1] = '\0';
}

void runtime_warning(Interpreter *interpreter, int line, int column, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char message[512];
    vsnprintf(message, sizeof(message), format, args);

    if (line > 0)
    {
        fprintf(stderr, "[AVISO Runtime] Linha %d, Coluna %d: %s\n", line, column, message);
    }
    else
    {
        fprintf(stderr, "[AVISO Runtime]: %s\n", message);
    }

    va_end(args);
}

/* --- FUNÇÕES BUILT-IN --- */

static Value *builtin_print(Interpreter *interpreter, Value **args, int arg_count)
{
    (void)interpreter; // Supprime warning

    for (int i = 0; i < arg_count; i++)
    {
        char *str = value_to_string(args[i]);
        printf("%s", str);
        free(str);
        if (i < arg_count - 1)
            printf(" ");
    }
    printf("\n");
    return value_create_void();
}

static Value *builtin_type(Interpreter *interpreter, Value **args, int arg_count)
{
    if (arg_count != 1)
    {
        runtime_error(interpreter, 0, 0, "função type() espera 1 argumento, obtido %d", arg_count);
        return NULL;
    }

    const char *type_name = value_type_to_string(args[0]->type);
    return value_create_string(type_name);
}

static Value *builtin_len(Interpreter *interpreter, Value **args, int arg_count)
{
    if (arg_count != 1)
    {
        runtime_error(interpreter, 0, 0, "função len() espera 1 argumento, obtido %d", arg_count);
        return NULL;
    }

    if (args[0]->type != VAL_STRING)
    {
        runtime_error(interpreter, 0, 0, "função len() espera string, obtido %s",
                      value_type_to_string(args[0]->type));
        return NULL;
    }

    return value_create_int((int)strlen(args[0]->data.string_val));
}

void register_builtin_functions(Interpreter *interpreter)
{
    Value *print_fn = value_create_builtin(builtin_print, "print");
    environment_define_var(interpreter->global_env, "print", print_fn);
    value_decref(print_fn);

    Value *type_fn = value_create_builtin(builtin_type, "type");
    environment_define_var(interpreter->global_env, "type", type_fn);
    value_decref(type_fn);

    Value *len_fn = value_create_builtin(builtin_len, "len");
    environment_define_var(interpreter->global_env, "len", len_fn);
    value_decref(len_fn);
}

/* --- OPERAÇÕES ARITMÉTICAS E LÓGICAS --- */

static Value *op_add(Interpreter *interpreter, Value *left, Value *right)
{
    // Int + Int
    if (left->type == VAL_INT && right->type == VAL_INT)
    {
        return value_create_int(left->data.int_val + right->data.int_val);
    }
    // Float + Float, Int + Float, Float + Int
    else if ((left->type == VAL_FLOAT || left->type == VAL_INT) &&
             (right->type == VAL_FLOAT || right->type == VAL_INT))
    {
        double l = (left->type == VAL_INT) ? (double)left->data.int_val : left->data.float_val;
        double r = (right->type == VAL_INT) ? (double)right->data.int_val : right->data.float_val;
        return value_create_float(l + r);
    }
    // String + String
    else if (left->type == VAL_STRING && right->type == VAL_STRING)
    {
        size_t len = strlen(left->data.string_val) + strlen(right->data.string_val) + 1;
        char *result_str = malloc(len);
        strcpy(result_str, left->data.string_val);
        strcat(result_str, right->data.string_val);
        Value *result = value_create_string(result_str);
        free(result_str);
        return result;
    }
    else
    {
        runtime_error(interpreter, 0, 0,
                      "Operação '+' não suportada para tipos %s e %s",
                      value_type_to_string(left->type),
                      value_type_to_string(right->type));
        return NULL;
    }
}

static Value *op_subtract(Interpreter *interpreter, Value *left, Value *right)
{
    if (left->type == VAL_INT && right->type == VAL_INT)
    {
        return value_create_int(left->data.int_val - right->data.int_val);
    }
    else if ((left->type == VAL_FLOAT || left->type == VAL_INT) &&
             (right->type == VAL_FLOAT || right->type == VAL_INT))
    {
        double l = (left->type == VAL_INT) ? (double)left->data.int_val : left->data.float_val;
        double r = (right->type == VAL_INT) ? (double)right->data.int_val : right->data.float_val;
        return value_create_float(l - r);
    }
    else
    {
        runtime_error(interpreter, 0, 0,
                      "Operação '-' não suportada para tipos %s e %s",
                      value_type_to_string(left->type),
                      value_type_to_string(right->type));
        return NULL;
    }
}

static Value *op_multiply(Interpreter *interpreter, Value *left, Value *right)
{
    if (left->type == VAL_INT && right->type == VAL_INT)
    {
        return value_create_int(left->data.int_val * right->data.int_val);
    }
    else if ((left->type == VAL_FLOAT || left->type == VAL_INT) &&
             (right->type == VAL_FLOAT || right->type == VAL_INT))
    {
        double l = (left->type == VAL_INT) ? (double)left->data.int_val : left->data.float_val;
        double r = (right->type == VAL_INT) ? (double)right->data.int_val : right->data.float_val;
        return value_create_float(l * r);
    }
    else
    {
        runtime_error(interpreter, 0, 0,
                      "Operação '*' não suportada para tipos %s e %s",
                      value_type_to_string(left->type),
                      value_type_to_string(right->type));
        return NULL;
    }
}

static Value *op_divide(Interpreter *interpreter, Value *left, Value *right)
{
    if ((left->type == VAL_FLOAT || left->type == VAL_INT) &&
        (right->type == VAL_FLOAT || right->type == VAL_INT))
    {
        double l = (left->type == VAL_INT) ? (double)left->data.int_val : left->data.float_val;
        double r = (right->type == VAL_INT) ? (double)right->data.int_val : right->data.float_val;

        if (r == 0.0)
        {
            runtime_error(interpreter, 0, 0, "Divisão por zero");
            return NULL;
        }

        return value_create_float(l / r);
    }
    else
    {
        runtime_error(interpreter, 0, 0,
                      "Operação '/' não suportada para tipos %s e %s",
                      value_type_to_string(left->type),
                      value_type_to_string(right->type));
        return NULL;
    }
}

static Value *op_modulo(Interpreter *interpreter, Value *left, Value *right)
{
    if (left->type == VAL_INT && right->type == VAL_INT)
    {
        if (right->data.int_val == 0)
        {
            runtime_error(interpreter, 0, 0, "Módulo por zero");
            return NULL;
        }
        return value_create_int(left->data.int_val % right->data.int_val);
    }
    else
    {
        runtime_error(interpreter, 0, 0,
                      "Operação '%%' não suportada para tipos %s e %s",
                      value_type_to_string(left->type),
                      value_type_to_string(right->type));
        return NULL;
    }
}

static Value *op_compare_eq(Interpreter *interpreter, Value *left, Value *right)
{
    (void)interpreter; // Supprime warning

    // Comparação entre tipos iguais
    if (left->type == right->type)
    {
        switch (left->type)
        {
        case VAL_INT:
            return value_create_bool(left->data.int_val == right->data.int_val);
        case VAL_FLOAT:
            return value_create_bool(fabs(left->data.float_val - right->data.float_val) < 1e-10);
        case VAL_STRING:
            return value_create_bool(strcmp(left->data.string_val, right->data.string_val) == 0);
        case VAL_BOOL:
            return value_create_bool(left->data.bool_val == right->data.bool_val);
        case VAL_VOID:
        case VAL_NULL:
            return value_create_bool(1); // void == void, null == null
        default:
            return value_create_bool(0);
        }
    }
    // Comparação entre int e float
    else if ((left->type == VAL_INT && right->type == VAL_FLOAT) ||
             (left->type == VAL_FLOAT && right->type == VAL_INT))
    {
        double l = (left->type == VAL_INT) ? (double)left->data.int_val : left->data.float_val;
        double r = (right->type == VAL_INT) ? (double)right->data.int_val : right->data.float_val;
        return value_create_bool(fabs(l - r) < 1e-10);
    }

    return value_create_bool(0); // Tipos diferentes são diferentes
}

static Value *op_compare_neq(Interpreter *interpreter, Value *left, Value *right)
{
    Value *eq_result = op_compare_eq(interpreter, left, right);
    if (eq_result == NULL)
        return NULL;

    Value *result = value_create_bool(!eq_result->data.bool_val);
    value_decref(eq_result);
    return result;
}

static Value *op_compare_gt(Interpreter *interpreter, Value *left, Value *right)
{
    if ((left->type == VAL_INT || left->type == VAL_FLOAT) &&
        (right->type == VAL_INT || right->type == VAL_FLOAT))
    {
        double l = (left->type == VAL_INT) ? (double)left->data.int_val : left->data.float_val;
        double r = (right->type == VAL_INT) ? (double)right->data.int_val : right->data.float_val;
        return value_create_bool(l > r);
    }
    else
    {
        runtime_error(interpreter, 0, 0,
                      "Operação '>' não suportada para tipos %s e %s",
                      value_type_to_string(left->type),
                      value_type_to_string(right->type));
        return NULL;
    }
}

static Value *op_compare_lt(Interpreter *interpreter, Value *left, Value *right)
{
    if ((left->type == VAL_INT || left->type == VAL_FLOAT) &&
        (right->type == VAL_INT || right->type == VAL_FLOAT))
    {
        double l = (left->type == VAL_INT) ? (double)left->data.int_val : left->data.float_val;
        double r = (right->type == VAL_INT) ? (double)right->data.int_val : right->data.float_val;
        return value_create_bool(l < r);
    }
    else
    {
        runtime_error(interpreter, 0, 0,
                      "Operação '<' não suportada para tipos %s e %s",
                      value_type_to_string(left->type),
                      value_type_to_string(right->type));
        return NULL;
    }
}

static Value *op_compare_gte(Interpreter *interpreter, Value *left, Value *right)
{
    Value *gt_result = op_compare_gt(interpreter, left, right);
    if (gt_result == NULL)
        return NULL;

    Value *eq_result = op_compare_eq(interpreter, left, right);
    if (eq_result == NULL)
    {
        value_decref(gt_result);
        return NULL;
    }

    Value *result = value_create_bool(gt_result->data.bool_val || eq_result->data.bool_val);
    value_decref(gt_result);
    value_decref(eq_result);
    return result;
}

static Value *op_compare_lte(Interpreter *interpreter, Value *left, Value *right)
{
    Value *lt_result = op_compare_lt(interpreter, left, right);
    if (lt_result == NULL)
        return NULL;

    Value *eq_result = op_compare_eq(interpreter, left, right);
    if (eq_result == NULL)
    {
        value_decref(lt_result);
        return NULL;
    }

    Value *result = value_create_bool(lt_result->data.bool_val || eq_result->data.bool_val);
    value_decref(lt_result);
    value_decref(eq_result);
    return result;
}

static Value *op_logical_not(Interpreter *interpreter, Value *operand)
{
    if (operand->type != VAL_BOOL)
    {
        runtime_error(interpreter, 0, 0,
                      "Operação '!' não suportada para tipo %s",
                      value_type_to_string(operand->type));
        return NULL;
    }

    return value_create_bool(!operand->data.bool_val);
}

/* --- FORWARD DECLARATIONS PARA EXECUÇÃO --- */
static Value *execute_program(Interpreter *interpreter, ASTNode *node);
static Value *execute_statement(Interpreter *interpreter, ASTNode *node);
static Value *execute_expression(Interpreter *interpreter, ASTNode *node);
static Value *execute_variable_decl(Interpreter *interpreter, ASTNode *node);
static Value *execute_function_decl(Interpreter *interpreter, ASTNode *node);
static Value *execute_block(Interpreter *interpreter, ASTNode *node);
static Value *execute_if_statement(Interpreter *interpreter, ASTNode *node);
static Value *execute_while_statement(Interpreter *interpreter, ASTNode *node);
static Value *execute_return_statement(Interpreter *interpreter, ASTNode *node);
static Value *execute_expression_statement(Interpreter *interpreter, ASTNode *node);
static Value *execute_binary_expr(Interpreter *interpreter, ASTNode *node);
static Value *execute_unary_expr(Interpreter *interpreter, ASTNode *node);
static Value *execute_assignment(Interpreter *interpreter, ASTNode *node);
static Value *execute_call_expr(Interpreter *interpreter, ASTNode *node);
static Value *execute_variable_expr(Interpreter *interpreter, ASTNode *node);
static Value *execute_literal_expr(Interpreter *interpreter, ASTNode *node);

/* --- FUNÇÕES DE EXECUÇÃO PRINCIPAIS --- */

static Value *execute_program(Interpreter *interpreter, ASTNode *node)
{
    if (node == NULL)
    {
        runtime_error(interpreter, 0, 0, "Nó raiz inválido");
        return NULL;
    }

    // Executa o nó raiz (geralmente um bloco)
    Value *result = execute_statement(interpreter, node);

    if (interpreter->has_runtime_error)
    {
        if (result)
            value_decref(result);
        return NULL;
    }

    return result;
}

static Value *execute_statement(Interpreter *interpreter, ASTNode *node)
{
    if (node == NULL)
        return value_create_void();

    switch (node->node_type)
    {
    case NODE_VAR_DECL:
        return execute_variable_decl(interpreter, node);
    case NODE_FUNC_DECL:
        return execute_function_decl(interpreter, node);
    case NODE_BLOCK:
        return execute_block(interpreter, node);
    case NODE_IF_STMT:
        return execute_if_statement(interpreter, node);
    case NODE_WHILE_STMT:
        return execute_while_statement(interpreter, node);
    case NODE_RETURN_STMT:
        return execute_return_statement(interpreter, node);
    case NODE_EXPR_STMT:
        return execute_expression_statement(interpreter, node);
    default:
        runtime_error(interpreter, node->line, node->column,
                      "Tipo de statement não implementado: %s",
                      node_type_to_string(node->node_type));
        return NULL;
    }
}

static Value *execute_variable_decl(Interpreter *interpreter, ASTNode *node)
{
    const char *var_name = node->data.var_decl.name;

    // Verificar se já existe no escopo atual (não nos pais)
    Value *existing = hashtable_get(interpreter->current_env->variables, var_name);
    if (existing != NULL)
    {
        runtime_error(interpreter, node->line, node->column,
                      "Variável '%s' já declarada neste escopo", var_name);
        return NULL;
    }

    Value *init_value = NULL;
    if (node->data.var_decl.initializer)
    {
        init_value = execute_expression(interpreter, node->data.var_decl.initializer);
        if (interpreter->has_runtime_error)
        {
            return NULL;
        }
    }
    else
    {
        // Valor padrão baseado no tipo
        switch (node->data_type)
        {
        case TYPE_INT:
            init_value = value_create_int(0);
            break;
        case TYPE_FLOAT:
            init_value = value_create_float(0.0);
            break;
        case TYPE_STRING:
            init_value = value_create_string("");
            break;
        case TYPE_BOOL:
            init_value = value_create_bool(0);
            break;
        default:
            init_value = value_create_null();
            break;
        }
    }

    // TODO: Verificar compatibilidade de tipos (deve ser feito pelo semantic analyzer)

    environment_define_var(interpreter->current_env, var_name, init_value);
    value_decref(init_value); // environment_define_var incrementa ref_count

    return value_create_void();
}

static Value *execute_function_decl(Interpreter *interpreter, ASTNode *node)
{
    const char *func_name = node->data.func_decl.name;

    // Registrar função no ambiente atual
    environment_define_func(interpreter->current_env, func_name, node);

    return value_create_void();
}

static Value *execute_block(Interpreter *interpreter, ASTNode *node)
{
    // Criar novo ambiente para o bloco
    Environment *block_env = environment_create(interpreter->current_env);
    Environment *previous_env = interpreter->current_env;
    interpreter->current_env = block_env;

    Value *result = value_create_void();

    for (int i = 0; i < node->data.block.stmt_count; i++)
    {
        Value *stmt_result = execute_statement(interpreter, node->data.block.statements[i]);

        if (interpreter->has_runtime_error)
        {
            value_decref(result);
            if (stmt_result)
                value_decref(stmt_result);
            interpreter->current_env = previous_env;
            environment_destroy(block_env);
            return NULL;
        }

        if (stmt_result)
        {
            value_decref(result);
            result = stmt_result;
        }

        // Verificar controle de fluxo
        if (interpreter->should_return || interpreter->should_break || interpreter->should_continue)
        {
            break;
        }
    }

    // Restaurar ambiente anterior
    interpreter->current_env = previous_env;
    environment_destroy(block_env);

    return result;
}

static Value *execute_if_statement(Interpreter *interpreter, ASTNode *node)
{
    // Avaliar condição
    Value *condition = execute_expression(interpreter, node->data.if_stmt.condition);
    if (interpreter->has_runtime_error)
    {
        if (condition)
            value_decref(condition);
        return NULL;
    }

    // Verificar se condition é bool
    if (condition->type != VAL_BOOL)
    {
        runtime_error(interpreter, node->line, node->column,
                      "Condição do if deve ser booleana, encontrado: %s",
                      value_type_to_string(condition->type));
        value_decref(condition);
        return NULL;
    }

    Value *result = value_create_void();

    // Executar ramo apropriado
    if (condition->data.bool_val)
    {
        Value *then_result = execute_statement(interpreter, node->data.if_stmt.then_branch);
        if (then_result)
        {
            value_decref(result);
            result = then_result;
        }
    }
    else if (node->data.if_stmt.else_branch)
    {
        Value *else_result = execute_statement(interpreter, node->data.if_stmt.else_branch);
        if (else_result)
        {
            value_decref(result);
            result = else_result;
        }
    }

    value_decref(condition);
    return result;
}

static Value *execute_while_statement(Interpreter *interpreter, ASTNode *node)
{
    Value *result = value_create_void();

    while (1)
    {
        // Verificar condição
        Value *condition = execute_expression(interpreter, node->data.while_stmt.condition);
        if (interpreter->has_runtime_error)
        {
            value_decref(condition);
            value_decref(result);
            return NULL;
        }

        if (condition->type != VAL_BOOL)
        {
            runtime_error(interpreter, node->line, node->column,
                          "Condição do while deve ser booleana, encontrado: %s",
                          value_type_to_string(condition->type));
            value_decref(condition);
            value_decref(result);
            return NULL;
        }

        // Se condição falsa, sair do loop
        if (!condition->data.bool_val)
        {
            value_decref(condition);
            break;
        }
        value_decref(condition);

        // Executar corpo
        Value *body_result = execute_statement(interpreter, node->data.while_stmt.body);

        // Verificar controle de fluxo
        if (interpreter->should_break)
        {
            interpreter->should_break = 0;
            if (body_result)
                value_decref(body_result);
            break;
        }

        if (interpreter->should_continue)
        {
            interpreter->should_continue = 0;
            if (body_result)
                value_decref(body_result);
            continue;
        }

        if (interpreter->should_return)
        {
            if (body_result)
            {
                value_decref(result);
                result = body_result;
            }
            break;
        }

        // Atualizar resultado (última expressão no corpo)
        if (body_result)
        {
            value_decref(result);
            result = body_result;
        }

        if (interpreter->has_runtime_error)
        {
            break;
        }
    }

    return result;
}

static Value *execute_return_statement(Interpreter *interpreter, ASTNode *node)
{
    Value *return_value = value_create_void();

    if (node->data.return_stmt.value)
    {
        return_value = execute_expression(interpreter, node->data.return_stmt.value);
        if (interpreter->has_runtime_error)
        {
            return NULL;
        }
    }

    interpreter->should_return = 1;
    if (interpreter->return_value)
    {
        value_decref(interpreter->return_value);
    }
    interpreter->return_value = return_value;
    value_incref(return_value);

    return return_value;
}

static Value *execute_expression_statement(Interpreter *interpreter, ASTNode *node)
{
    // Usar a estrutura correta do NODE_EXPR_STMT
    if (node->data.expr_stmt.expression)
    {
        return execute_expression(interpreter, node->data.expr_stmt.expression);
    }

    runtime_error(interpreter, node->line, node->column,
                  "Expression statement inválido");
    return NULL;
}

/* --- EXECUÇÃO DE EXPRESSÕES --- */

static Value *execute_expression(Interpreter *interpreter, ASTNode *node)
{
    if (node == NULL)
        return value_create_void();

    switch (node->node_type)
    {
    case NODE_BINARY_EXPR:
        return execute_binary_expr(interpreter, node);
    case NODE_UNARY_EXPR:
        return execute_unary_expr(interpreter, node);
    case NODE_ASSIGN_EXPR:
        return execute_assignment(interpreter, node);
    case NODE_CALL_EXPR:
        return execute_call_expr(interpreter, node);
    case NODE_VAR_EXPR:
        return execute_variable_expr(interpreter, node);
    case NODE_LITERAL:
        return execute_literal_expr(interpreter, node);
    default:
        runtime_error(interpreter, node->line, node->column,
                      "Tipo de expressão não implementado: %s",
                      node_type_to_string(node->node_type));
        return NULL;
    }
}

static Value *execute_binary_expr(Interpreter *interpreter, ASTNode *node)
{
    Value *left = execute_expression(interpreter, node->data.binary_expr.left);
    if (interpreter->has_runtime_error)
    {
        if (left)
            value_decref(left);
        return NULL;
    }

    Value *right = execute_expression(interpreter, node->data.binary_expr.right);
    if (interpreter->has_runtime_error)
    {
        value_decref(left);
        if (right)
            value_decref(right);
        return NULL;
    }

    Value *result = NULL;
    TokenType op = node->data.binary_expr.operator;

    switch (op)
    {
    case TOKEN_PLUS:
        result = op_add(interpreter, left, right);
        break;
    case TOKEN_MINUS:
        result = op_subtract(interpreter, left, right);
        break;
    case TOKEN_STAR:
        result = op_multiply(interpreter, left, right);
        break;
    case TOKEN_SLASH:
        result = op_divide(interpreter, left, right);
        break;
    // TOKEN_PERCENT não implementado no lexer
    case TOKEN_EQUAL_EQUAL:
        result = op_compare_eq(interpreter, left, right);
        break;
    case TOKEN_BANG_EQUAL:
        result = op_compare_neq(interpreter, left, right);
        break;
    case TOKEN_GREATER:
        result = op_compare_gt(interpreter, left, right);
        break;
    case TOKEN_LESS:
        result = op_compare_lt(interpreter, left, right);
        break;
    case TOKEN_GREATER_EQUAL:
        result = op_compare_gte(interpreter, left, right);
        break;
    case TOKEN_LESS_EQUAL:
        result = op_compare_lte(interpreter, left, right);
        break;
    default:
        runtime_error(interpreter, node->line, node->column,
                      "Operador binário não implementado: %d", op);
        break;
    }

    value_decref(left);
    value_decref(right);
    return result;
}

static Value *execute_unary_expr(Interpreter *interpreter, ASTNode *node)
{
    Value *operand = execute_expression(interpreter, node->data.unary_expr.operand);
    if (interpreter->has_runtime_error)
    {
        if (operand)
            value_decref(operand);
        return NULL;
    }

    Value *result = NULL;
    TokenType op = node->data.unary_expr.operator;

    switch (op)
    {
    // TOKEN_BANG não implementado no lexer
    case TOKEN_MINUS:
        if (operand->type == VAL_INT)
        {
            result = value_create_int(-operand->data.int_val);
        }
        else if (operand->type == VAL_FLOAT)
        {
            result = value_create_float(-operand->data.float_val);
        }
        else
        {
            runtime_error(interpreter, node->line, node->column,
                          "Operação unária '-' não suportada para tipo %s",
                          value_type_to_string(operand->type));
        }
        break;
    default:
        runtime_error(interpreter, node->line, node->column,
                      "Operador unário não implementado: %d", op);
        break;
    }

    value_decref(operand);
    return result;
}

static Value *execute_assignment(Interpreter *interpreter, ASTNode *node)
{
    Value *value = execute_expression(interpreter, node->data.assign_expr.value);
    if (interpreter->has_runtime_error)
    {
        if (value)
            value_decref(value);
        return NULL;
    }

    const char *var_name = node->data.assign_expr.variable_name;

    if (!environment_assign_var(interpreter->current_env, var_name, value))
    {
        runtime_error(interpreter, node->line, node->column,
                      "Variável '%s' não declarada", var_name);
        value_decref(value);
        return NULL;
    }

    // Retornar o valor atribuído
    return value;
}

static Value *execute_call_expr(Interpreter *interpreter, ASTNode *node)
{
    const char *func_name = node->data.call_expr.function_name;

    // Verificar se é função built-in
    Value *builtin = environment_get_var(interpreter->current_env, func_name);
    if (builtin && builtin->type == VAL_BUILTIN_FN)
    {
        // Preparar argumentos
        Value **args = NULL;
        if (node->data.call_expr.arg_count > 0)
        {
            args = malloc(sizeof(Value *) * node->data.call_expr.arg_count);
            for (int i = 0; i < node->data.call_expr.arg_count; i++)
            {
                args[i] = execute_expression(interpreter, node->data.call_expr.arguments[i]);
                if (interpreter->has_runtime_error)
                {
                    for (int j = 0; j <= i; j++)
                    {
                        if (args[j])
                            value_decref(args[j]);
                    }
                    free(args);
                    return NULL;
                }
            }
        }

        // Chamar função built-in
        Value *result = builtin->data.builtin_fn.function(interpreter, args, node->data.call_expr.arg_count);

        // Limpar argumentos
        if (args)
        {
            for (int i = 0; i < node->data.call_expr.arg_count; i++)
            {
                value_decref(args[i]);
            }
            free(args);
        }

        return result;
    }

    // Buscar função definida pelo usuário
    ASTNode *function_node = environment_get_func(interpreter->current_env, func_name);
    if (!function_node)
    {
        runtime_error(interpreter, node->line, node->column,
                      "Função '%s' não definida", func_name);
        return NULL;
    }

    // Verificar número de argumentos
    if (node->data.call_expr.arg_count != function_node->data.func_decl.param_count)
    {
        runtime_error(interpreter, node->line, node->column,
                      "Número incorreto de argumentos para '%s': esperado %d, obtido %d",
                      func_name,
                      function_node->data.func_decl.param_count,
                      node->data.call_expr.arg_count);
        return NULL;
    }

    // Criar novo ambiente para a função
    Environment *function_env = environment_create(interpreter->current_env);

    // Avaliar argumentos e definir parâmetros
    for (int i = 0; i < node->data.call_expr.arg_count; i++)
    {
        Value *arg_value = execute_expression(interpreter, node->data.call_expr.arguments[i]);
        if (interpreter->has_runtime_error)
        {
            environment_destroy(function_env);
            if (arg_value)
                value_decref(arg_value);
            return NULL;
        }

        ASTNode *param = function_node->data.func_decl.params[i];
        environment_define_var(function_env, param->data.param.name, arg_value);
        value_decref(arg_value); // environment_define_var incrementa ref
    }

    // Executar função
    push_call_frame(interpreter, func_name, node->line);

    Environment *previous_env = interpreter->current_env;
    interpreter->current_env = function_env;

    // Salvar estado de return anterior
    int old_should_return = interpreter->should_return;
    Value *old_return_value = interpreter->return_value;
    interpreter->should_return = 0;
    interpreter->return_value = NULL;

    Value *result = execute_block(interpreter, function_node->data.func_decl.body);

    // Se houve return, usar o valor de retorno
    if (interpreter->should_return && interpreter->return_value)
    {
        if (result)
            value_decref(result);
        result = interpreter->return_value;
        value_incref(result);
    }

    // Restaurar estado anterior
    interpreter->current_env = previous_env;
    interpreter->should_return = old_should_return;
    if (interpreter->return_value)
    {
        value_decref(interpreter->return_value);
    }
    interpreter->return_value = old_return_value;

    pop_call_frame(interpreter);

    // Limpar ambiente da função
    environment_destroy(function_env);

    // Se não retornou explicitamente, retornar void
    if (!result)
    {
        result = value_create_void();
    }

    return result;
}

static Value *execute_variable_expr(Interpreter *interpreter, ASTNode *node)
{
    const char *var_name = node->data.var_expr.name;

    Value *value = environment_get_var(interpreter->current_env, var_name);
    if (value == NULL)
    {
        runtime_error(interpreter, node->line, node->column,
                      "Variável '%s' não definida", var_name);
        return NULL;
    }

    // Incrementar referência e retornar
    value_incref(value);
    return value;
}

static Value *execute_literal_expr(Interpreter *interpreter, ASTNode *node)
{
    (void)interpreter; // Supprime warning

    switch (node->data.literal.literal_type)
    {
    case TOKEN_INT_LITERAL:
        return value_create_int(node->data.literal.value.int_value);
    case TOKEN_FLOAT_LITERAL:
        return value_create_float(node->data.literal.value.float_value);
    case TOKEN_STRING_LITERAL:
        return value_create_string(node->data.literal.value.string_value);
    case TOKEN_TRUE:
        return value_create_bool(1);
    case TOKEN_FALSE:
        return value_create_bool(0);
    default:
        runtime_error(interpreter, node->line, node->column,
                      "Tipo de literal não suportado: %d", node->data.literal.literal_type);
        return NULL;
    }
}

/* --- FUNÇÕES PÚBLICAS PRINCIPAIS --- */

void interpreter_init(Interpreter *interpreter, ASTNode *ast)
{
    // Inicializar estrutura principal
    interpreter->global_env = environment_create(NULL);
    interpreter->current_env = interpreter->global_env;
    interpreter->ast_root = ast;

    // Estado de execução
    interpreter->should_return = 0;
    interpreter->return_value = NULL;
    interpreter->should_break = 0;
    interpreter->should_continue = 0;

    // Contadores de erro
    interpreter->error_count = 0;
    interpreter->has_runtime_error = 0;
    interpreter->error_msg[0] = '\0';

    // Configurações
    interpreter->debug_mode = 0;
    interpreter->trace_execution = 0;

    // Stack de chamadas
    interpreter->call_stack = NULL;
    interpreter->call_stack_size = 0;
    interpreter->call_stack_capacity = 0;

    // Sistema de valores (para GC futuro)
    interpreter->allocated_values = NULL;
    interpreter->allocated_count = 0;
    interpreter->allocated_capacity = 0;

    // Registrar funções built-in
    register_builtin_functions(interpreter);
}

int interpreter_execute(Interpreter *interpreter)
{
    if (interpreter->ast_root == NULL)
    {
        runtime_error(interpreter, 0, 0, "AST não fornecida para execução");
        return 0;
    }

    printf("========================================\n");
    printf("       EXECUTANDO PROGRAMA CRAZE v0.1   \n");
    printf("========================================\n\n");

    Value *result = execute_program(interpreter, interpreter->ast_root);

    if (interpreter->has_runtime_error)
    {
        printf("\n========================================\n");
        printf("        EXECUÇÃO INTERROMPIDA POR ERRO  \n");
        printf("========================================\n");
        printf("Erros encontrados: %d\n", interpreter->error_count);
        if (result)
            value_decref(result);
        return 0;
    }

    printf("\n========================================\n");
    printf("        EXECUÇÃO CONCLUÍDA COM SUCESSO  \n");
    printf("========================================\n");

    if (result)
    {
        char *result_str = value_to_string(result);
        printf("Resultado final: %s\n", result_str);
        free(result_str);
        value_decref(result);
    }

    return 1;
}

void interpreter_cleanup(Interpreter *interpreter)
{
    // Limpar ambiente global (que limpa todos os ambientes filhos)
    if (interpreter->global_env)
    {
        environment_destroy(interpreter->global_env);
        interpreter->global_env = NULL;
        interpreter->current_env = NULL;
    }

    // Limpar valor de retorno se existe
    if (interpreter->return_value)
    {
        value_decref(interpreter->return_value);
        interpreter->return_value = NULL;
    }

    // Limpar stack de chamadas
    if (interpreter->call_stack)
    {
        for (int i = 0; i < interpreter->call_stack_size; i++)
        {
            free(interpreter->call_stack[i].function_name);
        }
        free(interpreter->call_stack);
        interpreter->call_stack = NULL;
        interpreter->call_stack_size = 0;
        interpreter->call_stack_capacity = 0;
    }

    // Limpar valores alocados (para GC futuro)
    if (interpreter->allocated_values)
    {
        for (int i = 0; i < interpreter->allocated_count; i++)
        {
            if (interpreter->allocated_values[i])
            {
                value_free(interpreter->allocated_values[i]);
            }
        }
        free(interpreter->allocated_values);
        interpreter->allocated_values = NULL;
        interpreter->allocated_count = 0;
        interpreter->allocated_capacity = 0;
    }
}

/* --- FUNÇÕES DE ACESSO GLOBAL --- */

Value *interpreter_get_global(Interpreter *interpreter, const char *name)
{
    return hashtable_get(interpreter->global_env->variables, name);
}

int interpreter_set_global(Interpreter *interpreter, const char *name, Value *value)
{
    hashtable_set(interpreter->global_env->variables, name, value);
    return 1;
}
#ifndef CRAZE_INTERPRETER_H
#define CRAZE_INTERPRETER_H

#include "craze_semantic.h"
#include <stdarg.h>

/* --- Tipos de Valores Runtime --- */
typedef enum
{
    VAL_INT,
    VAL_FLOAT,
    VAL_STRING,
    VAL_BOOL,
    VAL_VOID,
    VAL_NULL,      // Para valores não inicializados/erros
    VAL_BUILTIN_FN // Para funções built-in
} ValueType;

/* --- Forward declarations --- */
typedef struct Value Value;
typedef struct Environment Environment;
typedef struct Interpreter Interpreter;
typedef struct HashTable HashTable;
typedef struct HashEntry HashEntry;
typedef struct CallFrame CallFrame;

/* --- Função Built-in --- */
typedef Value *(*BuiltinFn)(Interpreter *interpreter, Value **args, int arg_count);

/* --- Valor Runtime --- */
typedef struct Value
{
    ValueType type;
    union
    {
        int int_val;
        double float_val;
        char *string_val;
        int bool_val;
        struct
        {
            BuiltinFn function;
            char *name;
        } builtin_fn;
    } data;
    int ref_count; // Para garbage collection simples
} Value;

/* --- Entrada da Tabela Hash --- */
typedef struct HashEntry
{
    char *key;
    Value *value;
    struct HashEntry *next;
} HashEntry;

/* --- Tabela Hash para Variáveis --- */
typedef struct HashTable
{
    HashEntry **buckets;
    int capacity;
    int count;
} HashTable;

/* --- Ambiente de Execução (Environment) --- */
typedef struct Environment
{
    struct Environment *parent; // Encadeamento para escopos externos
    HashTable *variables;       // Tabela hash: nome -> Value*
    HashTable *functions;       // Tabela hash: nome -> ASTNode* (função definida)
} Environment;

/* --- Frame de Chamada --- */
typedef struct CallFrame
{
    char *function_name;
    int line_number;
    Environment *environment;
} CallFrame;

/* --- Estado do Interpretador --- */
typedef struct Interpreter
{
    Environment *global_env;
    Environment *current_env;
    ASTNode *ast_root;

    // Controle de fluxo
    int should_return;
    Value *return_value;
    int should_break;
    int should_continue;

    // Estado de execução
    int error_count;
    int has_runtime_error;
    char error_msg[512];

    // Configurações
    int debug_mode;
    int trace_execution;

    // Stack de chamadas para debug
    CallFrame *call_stack;
    int call_stack_size;
    int call_stack_capacity;

    // Sistema de valores (para GC futuro)
    Value **allocated_values;
    int allocated_count;
    int allocated_capacity;
} Interpreter;

/* --- FUNÇÕES PÚBLICAS --- */

/* Inicializa o interpretador com AST validada */
void interpreter_init(Interpreter *interpreter, ASTNode *ast);

/* Executa o programa completo */
int interpreter_execute(Interpreter *interpreter);

/* Libera todos os recursos do interpretador */
void interpreter_cleanup(Interpreter *interpreter);

/* Funções de utilidade para valores */
Value *value_create_int(int value);
Value *value_create_float(double value);
Value *value_create_string(const char *value);
Value *value_create_bool(int value);
Value *value_create_void(void);
Value *value_create_null(void);
void value_free(Value *value);
void value_incref(Value *value);
void value_decref(Value *value);
char *value_to_string(Value *value);
const char *value_type_to_string(ValueType type);

/* Acesso a variáveis globais para embedding */
Value *interpreter_get_global(Interpreter *interpreter, const char *name);
int interpreter_set_global(Interpreter *interpreter, const char *name, Value *value);

/* Funções de ambiente */
Environment *environment_create(Environment *parent);
void environment_destroy(Environment *env);
void environment_define_var(Environment *env, const char *name, Value *value);
Value *environment_get_var(Environment *env, const char *name);
int environment_assign_var(Environment *env, const char *name, Value *value);
void environment_define_func(Environment *env, const char *name, ASTNode *func_node);
ASTNode *environment_get_func(Environment *env, const char *name);

/* Funções de tabela hash */
HashTable *hashtable_create(int capacity);
void hashtable_destroy(HashTable *table);
void hashtable_set(HashTable *table, const char *key, Value *value);
Value *hashtable_get(HashTable *table, const char *key);
int hashtable_has(HashTable *table, const char *key);
void hashtable_remove(HashTable *table, const char *key);

/* Funções de pilha de chamadas */
void push_call_frame(Interpreter *interpreter, const char *function_name, int line);
void pop_call_frame(Interpreter *interpreter);

/* Funções de erro runtime */
void runtime_error(Interpreter *interpreter, int line, int column, const char *format, ...);
void runtime_warning(Interpreter *interpreter, int line, int column, const char *format, ...);

/* Registro de funções built-in */
void register_builtin_functions(Interpreter *interpreter);

#endif /* CRAZE_INTERPRETER_H */
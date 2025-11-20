# 🚀 COMO PROGRAMAR EM CRAZE v0.1

## 📝 **O QUE É CRAZE?**

Craze é uma linguagem de programação:
- **Imperativa**: Baseada em comandos e instruções
- **Estruturada**: Com funções, loops e condicionais
- **Estaticamente tipada**: Tipos definidos em tempo de compilação
- **Interpretada**: Execução direta do código fonte

## 🎯 **CARACTERÍSTICAS PRINCIPAIS**

✅ **Sintaxe simples e clara**  
✅ **Tipagem estática** (int, float, string, bool)  
✅ **Funções com recursão**  
✅ **Estruturas de controle** (if/else, while)  
✅ **Operadores aritméticos e lógicos**  
✅ **Funções built-in** (print, type, len)  
✅ **Gestão automática de memória**  

## 📚 **DOCUMENTAÇÃO**

- **[Guia Completo](docs/GUIA_PROGRAMACAO.md)** - Sintaxe e exemplos detalhados
- **[Exemplos](examples/)** - Programas prontos para executar

## 🔧 **ESTRUTURA DO PROJETO**

```
Craze v0.1/
├── 📁 src/           # Código fonte do interpretador
├── 📁 include/       # Headers
├── 📁 tests/         # Testes automatizados
├── 📁 examples/      # Programas exemplo em Craze
├── 📁 docs/          # Documentação
├── 📁 obj/           # Arquivos objeto (compilação)
├── 📁 bin/           # Executáveis
└── Makefile          # Sistema de build
```

## ⚡ **PRIMEIROS PASSOS**

### **1. Seu Primeiro Programa**

Crie um arquivo `meu_programa.craze`:

```craze
// Meu primeiro programa em Craze
print("Olá, Craze!");

let nome: string = "Programador";
let idade: int = 25;

print("Meu nome é", nome);
print("Tenho", idade, "anos");

if (idade >= 18) {
    print("Sou maior de idade!");
}
```

### **2. Exemplo com Função**

```craze
fn saudacao(nome: string): void {
    print("Bem-vindo,", nome, "!");
    print("Você está programando em Craze v0.1");
}

fn calcular(a: int, b: int): int {
    let resultado: int = a + b;
    return resultado;
}

// Programa principal
saudacao("João");

let x: int = 10;
let y: int = 20;
let soma: int = calcular(x, y);

print("A soma de", x, "e", y, "é", soma);
```

## 📖 **EXEMPLOS INCLUSOS**

| Arquivo | Descrição |
|---------|-----------|
| `01_hello_world.craze` | Primeiro programa básico |
| `02_calculadora.craze` | Operações matemáticas |
| `03_fatorial.craze` | Recursão e fatorial |
| `04_loops.craze` | Loops e contadores |
| `05_strings.craze` | Manipulação de texto |
| `06_notas.craze` | Sistema acadêmico |
| `07_algoritmos.craze` | Fibonacci, primos, MDC |

## 🛠️ **COMPILAÇÃO E EXECUÇÃO**

### **Compilar o Interpretador:**
```bash
# Windows (MinGW)
gcc -Wall -Wextra -std=c99 -g -Iinclude -c src/*.c
gcc *.o -o craze.exe

# Linux/macOS
make all
```

### **Executar Testes:**
```bash
# Testar todos os componentes
make test

# Testar apenas o interpretador
make test-interpreter
```

### **Executar Programa (em desenvolvimento):**
```bash
# Futuro: executar arquivo .craze
./craze meu_programa.craze
```

## 🎓 **APRENDENDO CRAZE**

### **1. Conceitos Básicos**
- Variáveis e tipos
- Operadores
- Entrada e saída

### **2. Controle de Fluxo**
- Condicionais (if/else)
- Loops (while)
- Lógica booleana

### **3. Funções**
- Declaração e chamada
- Parâmetros e retorno
- Recursão

### **4. Algoritmos**
- Matemática básica
- Strings e texto
- Estruturas de dados simples

## 🔍 **SINTAXE RÁPIDA**

```craze
// Variáveis
let numero: int = 42;
let texto: string = "Hello";
let ativo: bool = true;
let valor: float = 3.14;

// Função
fn minhaFuncao(param: int): string {
    return "Resultado: " + type(param);
}

// Condicional
if (numero > 0) {
    print("Positivo");
} else {
    print("Não positivo");
}

// Loop
let i: int = 0;
while (i < 5) {
    print("Contador:", i);
    i = i + 1;
}

// Chamada de função
let resultado: string = minhaFuncao(100);
print(resultado);
```

## 🚨 **REGRAS IMPORTANTES**

1. **Todo identificador deve ter tipo explícito**
2. **Statements terminam com `;`**
3. **Blocos usam `{ }`**
4. **Strings usam aspas duplas `""`**
5. **Comentários começam com `//`**
6. **Case-sensitive** (maiúsculas ≠ minúsculas)

## 🎯 **PRÓXIMOS PASSOS**

1. **Leia o [Guia Completo](docs/GUIA_PROGRAMACAO.md)**
2. **Execute os exemplos** em `examples/`
3. **Experimente modificar** os programas
4. **Crie seus próprios algoritmos**
5. **Explore recursão e matemática**

## 🆘 **PRECISA DE AJUDA?**

- Consulte a **[documentação completa](docs/GUIA_PROGRAMACAO.md)**
- Analise os **[exemplos práticos](examples/)**
- Execute os **testes** para ver a linguagem funcionando

---

**Divirta-se programando em Craze v0.1! 🎉**

*Uma linguagem simples, poderosa e educativa para aprender programação!*
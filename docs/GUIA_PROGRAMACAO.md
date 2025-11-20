# 📚 GUIA DE PROGRAMAÇÃO EM CRAZE v0.1

## 🎯 **INTRODUÇÃO**

Craze é uma linguagem de programação imperativa, estruturada e estaticamente tipada. Este guia te ensinará como escrever programas em Craze v0.1.

--- 

## 🔤 **SINTAXE BÁSICA**

### **1. Declaração de Variáveis**

```craze
// Declaração com tipo explícito
let nome: int = 42;
let pi: float = 3.14159;
let mensagem: string = "Olá, Craze!";
let ativo: bool = true;

// Declaração sem inicialização (valor padrão)
let contador: int;        // = 0
let taxa: float;          // = 0.0
let texto: string;        // = ""
let flag: bool;           // = false
```

### **2. Tipos de Dados**

- **`int`** - Números inteiros: `42`, `-17`, `0`
- **`float`** - Números decimais: `3.14`, `-2.5`, `0.0`
- **`string`** - Texto: `"Hello"`, `"Mundo Craze"`, `""`
- **`bool`** - Booleanos: `true`, `false`
- **`void`** - Tipo vazio (apenas para funções)

### **3. Operadores**

#### **Aritméticos:**
```craze
let a: int = 10;
let b: int = 3;

let soma: int = a + b;        // 13
let sub: int = a - b;         // 7
let mult: int = a * b;        // 30
let div: float = a / b;       // 3.333333
let mod: int = a % b;         // 1
```

#### **Comparação:**
```craze
let x: int = 5;
let y: int = 10;

let igual: bool = x == y;     // false
let diferente: bool = x != y; // true
let maior: bool = x > y;      // false
let menor: bool = x < y;      // true
let maiorIgual: bool = x >= y; // false
let menorIgual: bool = x <= y; // true
```

#### **Lógicos:**
```craze
let p: bool = true;
let q: bool = false;

let negacao: bool = !p;       // false
```

---

## 🔧 **ESTRUTURAS DE CONTROLE**

### **1. Condicionais (if/else)**

```craze
let idade: int = 18;

if (idade >= 18) {
    print("Maior de idade");
} else {
    print("Menor de idade");
}

// If aninhado
let nota: float = 8.5;

if (nota >= 9.0) {
    print("Excelente");
} else {
    if (nota >= 7.0) {
        print("Bom");
    } else {
        print("Precisa melhorar");
    }
}
```

### **2. Loops (while)**

```craze
// Contagem simples
let i: int = 0;
while (i < 5) {
    print("Contador:", i);
    i = i + 1;
}

// Loop com condição
let numero: int = 1024;
let divisoes: int = 0;

while (numero > 1) {
    numero = numero / 2;
    divisoes = divisoes + 1;
}
print("Número de divisões:", divisoes);
```

---

## 🎯 **FUNÇÕES**

### **1. Declaração de Funções**

```craze
// Função simples
fn saudacao(): void {
    print("Olá, mundo Craze!");
}

// Função com parâmetros
fn somar(a: int, b: int): int {
    return a + b;
}

// Função com múltiplos parâmetros
fn apresentar(nome: string, idade: int): void {
    print("Meu nome é", nome, "e tenho", idade, "anos");
}
```

### **2. Chamada de Funções**

```craze
// Chamadas simples
saudacao();

let resultado: int = somar(10, 20);
print("Resultado:", resultado);

apresentar("João", 25);
```

### **3. Funções Recursivas**

```craze
// Fatorial recursivo
fn fatorial(n: int): int {
    if (n == 0) {
        return 1;
    } else {
        return n * fatorial(n - 1);
    }
}

// Fibonacci recursivo
fn fibonacci(n: int): int {
    if (n <= 1) {
        return n;
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}

// Uso das funções
let fat5: int = fatorial(5);
print("5! =", fat5);  // 5! = 120

let fib10: int = fibonacci(10);
print("Fibonacci(10) =", fib10);  // Fibonacci(10) = 55
```

---

## 🛠️ **FUNÇÕES BUILT-IN**

### **1. print() - Saída de dados**
```craze
print("Hello World");
print("Valor:", 42);
print("Nome:", "Craze", "Versão:", 1.0);
```

### **2. type() - Verificar tipo**
```craze
let valor: int = 100;
let tipo: string = type(valor);
print("Tipo:", tipo);  // Tipo: int
```

### **3. len() - Comprimento de string**
```craze
let texto: string = "Craze";
let tamanho: int = len(texto);
print("Tamanho:", tamanho);  // Tamanho: 5
```

---

## 📝 **PROGRAMAS EXEMPLO**

### **1. Calculadora Simples**
```craze
fn calculadora(): void {
    let a: float = 15.5;
    let b: float = 4.2;
    
    print("=== CALCULADORA CRAZE ===");
    print("a =", a);
    print("b =", b);
    print("");
    
    print("Soma:", a + b);
    print("Subtração:", a - b);
    print("Multiplicação:", a * b);
    print("Divisão:", a / b);
}

calculadora();
```

### **2. Contador de Pares/Ímpares**
```craze
fn analisarNumeros(): void {
    let i: int = 1;
    let pares: int = 0;
    let impares: int = 0;
    
    while (i <= 10) {
        if (i % 2 == 0) {
            print(i, "é par");
            pares = pares + 1;
        } else {
            print(i, "é ímpar");
            impares = impares + 1;
        }
        i = i + 1;
    }
    
    print("Total pares:", pares);
    print("Total ímpares:", impares);
}

analisarNumeros();
```

### **3. Sistema de Classificação**
```craze
fn classificarNota(nota: float): string {
    if (nota >= 9.0) {
        return "A";
    } else {
        if (nota >= 8.0) {
            return "B";
        } else {
            if (nota >= 7.0) {
                return "C";
            } else {
                if (nota >= 6.0) {
                    return "D";
                } else {
                    return "F";
                }
            }
        }
    }
}

fn sistemaNotas(): void {
    let nota1: float = 9.5;
    let nota2: float = 7.8;
    let nota3: float = 5.2;
    
    print("Nota", nota1, "=", classificarNota(nota1));
    print("Nota", nota2, "=", classificarNota(nota2));
    print("Nota", nota3, "=", classificarNota(nota3));
}

sistemaNotas();
```

### **4. Processamento de Strings**
```craze
fn analisarTexto(): void {
    let nome: string = "Linguagem Craze";
    let versao: string = "v0.1";
    
    print("=== ANÁLISE DE TEXTO ===");
    print("Texto:", nome);
    print("Versão:", versao);
    
    let tamanho: int = len(nome);
    print("Tamanho do nome:", tamanho);
    
    let nomeCompleto: string = nome + " " + versao;
    print("Nome completo:", nomeCompleto);
    print("Tamanho completo:", len(nomeCompleto));
    
    print("Tipo do nome:", type(nome));
    print("Tipo do tamanho:", type(tamanho));
}

analisarTexto();
```

### **5. Algoritmo Matemático (Sequência)**
```craze
fn gerarSequencia(limite: int): void {
    let atual: int = 1;
    let anterior: int = 1;
    let proximo: int = 0;
    
    print("=== SEQUÊNCIA FIBONACCI ===");
    print("Limite:", limite);
    print("");
    
    if (limite >= 1) {
        print("F(1) =", atual);
    }
    if (limite >= 2) {
        print("F(2) =", anterior);
    }
    
    let i: int = 3;
    while (i <= limite) {
        proximo = atual + anterior;
        print("F(" + type(i) + ") =", proximo);
        
        anterior = atual;
        atual = proximo;
        i = i + 1;
    }
}

gerarSequencia(10);
```

---

## ⚡ **DICAS E BOAS PRÁTICAS**

### **1. Convenções de Nomenclatura**
```craze
// ✅ Boas práticas
let nomeCompleto: string = "João Silva";
let idadeUsuario: int = 30;
let estaAtivo: bool = true;

fn calcularMedia(nota1: float, nota2: float): float {
    return (nota1 + nota2) / 2.0;
}
```

### **2. Estruturação de Código**
```craze
// ✅ Código bem estruturado
fn main(): void {
    let resultado: int = processarDados();
    exibirResultado(resultado);
}

fn processarDados(): int {
    let soma: int = 0;
    let i: int = 1;
    
    while (i <= 100) {
        soma = soma + i;
        i = i + 1;
    }
    
    return soma;
}

fn exibirResultado(valor: int): void {
    print("O resultado é:", valor);
}

main();
```

### **3. Comentários e Documentação**
```craze
// Esta função calcula o maior divisor comum
fn mdc(a: int, b: int): int {
    // Algoritmo de Euclides
    while (b != 0) {
        let temp: int = b;
        b = a % b;
        a = temp;
    }
    return a;
}
```

---

## 🚀 **COMO EXECUTAR SEUS PROGRAMAS**

### **1. Criar um arquivo `.craze`**
```bash
# Crie um arquivo exemplo.craze com seu código
```

### **2. Compilar e executar (quando os ajustes estiverem prontos)**
```bash
# Compilar o interpretador
make test-interpreter

# Executar programa (futuro)
./bin/craze exemplo.craze
```

### **3. Testar no interpretador integrado**
```bash
# Os testes já executam programas exemplo
./bin/test_interpreter.exe
```

---

## 📖 **RESUMO DA LINGUAGEM**

- ✅ **Tipagem estática**: Todos os tipos devem ser declarados
- ✅ **Sintaxe clara**: Inspirada em linguagens modernas
- ✅ **Funções**: Suporte a recursão e parâmetros
- ✅ **Controle de fluxo**: if/else e while
- ✅ **Operadores**: Aritméticos, lógicos e comparação
- ✅ **Built-ins**: print(), type(), len()
- ✅ **Escopos**: Variáveis locais e globais
- ✅ **Strings**: Concatenação e manipulação

**Craze v0.1** é perfeita para aprender programação, algoritmos e estruturas de dados! 🎉
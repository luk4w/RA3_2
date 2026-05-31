# ANALISADOR SEMÂNTICO

Lucas Franco de Mello - luk4w
Nome do grupo no Canvas: RA3 2

Instituição: Pontifícia Universidade Católica do Paraná (PUCPR)  
Disciplina: Linguagens Formais e Compiladores
Professor: Frank de Alcantara  
Ano: 2026  
Linguagem de implementação: C++ (padrão C++23)

> Devido a plataforma do github não permitir a criação de repositórios com espaços, o nome do grupo foi alterado para RA3_2, no entanto, o nome real do grupo é RA3 2.

O `ANALISADOR SEMÂNTICO` é a terceira fase do projeto da disciplina de `Linguagens Formais e Compiladores` ministrada pelo professor `Frank de Alcantara` na `Pontifícia Universidade Católica do Paraná`.

## Instruções de Compilação e Execução

A infraestrutura de *build* deste projeto é orquestrada pelo **CMake**, e garante uma compilação modular e reprodutível. Como o desenvolvimento tem como alvo principal o ambiente Windows, as instruções abaixo utilizam a *toolchain* do **MSVC** (Microsoft Visual C++) integrada ao **Visual Studio Code**.

#### Pré-requisitos do Ambiente
Certifique-se de ter os seguintes componentes instalados:
* **Compilador:** Ferramentas de Build do Visual Studio Community 2022 (ou superior) com suporte para desenvolvimento em C++23.
* **Editor:** Visual Studio Code.
* **Extensões (Visual Studio Code):**
  * `C/C++` (Microsoft)
  * `CMake Tools` (Microsoft)

#### Metodo 1: Compilação via Visual Studio Code
A extensão [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) automatiza os comandos de configuração e construção, o que simplifica o processo. Siga os passos abaixo:

1. Abra a pasta raiz do projeto no Visual Studio Code (`File > Open Folder...`).
2. A extensão identificará o arquivo `CMakeLists.txt` então, através da paleta de comandos `Ctrl+Shift+p`, ou o atalho que você configurou, selecione um **Kit** de compilação, escolha a arquitetura nativa do MSVC (ex: `Visual Studio Community 2022 Release - x86_amd64`).
3. Aguarde o processo de configuração (*Configuring*) terminar. O CMake irá gerar a árvore de diretórios e o *cache* de compilação.
4. Utilize o atalho `Ctrl+Shift+p` para abrir a paleta de comandos, e execute `CMake: Build` para iniciar a compilação do projeto.
5. Se não houver erros, o executável final (`AnalisadorSemantico.exe`) será gerado dentro do diretório `build/Debug/` (ou `build/Release/`), conforme a configuração selecionada.

#### Metodo 2: Compilação via CMake CLI

Para quem prefere o terminal, com o CMake e a *toolchain* do MSVC disponíveis no `PATH` (use o **Developer PowerShell for VS 2022**), execute a partir da raiz do projeto:

```powershell
# 1. Configura o projeto e gera o sistema de build no diretorio build/
cmake -S . -B build

# 2. Compila no modo Release
cmake --build build --config Release
```

O executável final fica em `build\Release\AnalisadorSemantico.exe`. Para um build de depuração, troque `Release` por `Debug` no passo 2.

### Execução

O programa recebe **um único argumento**: o caminho de um arquivo de teste `.txt` que contém o programa em RPN (uma expressão por linha, entre `(START)` e `(END)`).

A partir do diretório onde está o executável:

```powershell
.\AnalisadorSemantico.exe .\teste.txt
```

Exemplos com os arquivos de teste fornecidos na pasta `tests/`:

```powershell
# Programa valido (gera ast_saida.json e saida.s)
.\AnalisadorSemantico.exe ..\..\tests\teste1.txt

# Programa com erros (exibe o relatório de erros e NÃO gera assembly)
.\AnalisadorSemantico.exe ..\..\tests\teste_erro_sintatico.txt
```

> Nota: ajustar o caminho relativo (`..\..\tests\`).

**Saídas geradas** no diretório de execução (o `tokens.txt` sai sempre; o `ast_saida.json` e o `saida.s` apenas quando o programa não contém erros):

| Arquivo | Descrição |
| :--- | :--- |
| `tokens.txt` | Vetor de tokens da última execução (saída do analisador léxico). |
| `ast_saida.json` | Árvore sintática **atribuída** (AST com `tipoDado` em cada nó) serializada em JSON. |
| `saida.s` | Código Assembly ARMv7 gerado a partir da AST. |

Se o programa **contém erros**, é exibido um **relatório de erros** (com tipo `LEXICO`/`SINTATICO`/`SEMANTICO` e número da linha) e nenhum código Assembly é gerado - o processo encerra com código de saída `1`.

### Sintaxe das Estruturas de Controle

A linguagem mantém a notação polonesa reversa (RPN) para todas as estruturas. Os operandos sempre precedem a keyword que os opera.

#### Tomada de Decisão - `IFELSE`

Requer **3 operandos** antes da keyword: condição, bloco verdadeiro e bloco falso.

```
( (condição) (bloco_verdadeiro) (bloco_falso) IFELSE )
```

**Exemplo** - se `A > B`, retorna `A + B`, senão retorna `A - B`:
```
( (A B >) (A B +) (A B -) IFELSE )
```

A condição deve ter tipo lógico (`bool`) - tipicamente o resultado de um operador relacional (`<`, `>`, `<=`, `>=`, `==`, `!=`) ou um literal `TRUE`/`FALSE`. Ambos os blocos são obrigatórios.

#### Laço de Repetição - `WHILE`

Requer **2 operandos** antes da keyword: condição e bloco de repetição.

```
( (condição) (bloco_repeticao) WHILE )
```

**Exemplo** - enquanto `CONTADOR < 10`, soma `1` ao contador e armazena:
```
( ((CONTADOR) 10 <) (((CONTADOR) 1 +) CONTADOR) WHILE )
```

O laço avalia a condição antes de cada iteração. Quando a condição resulta em falso, a execução continua na linha seguinte ao `WHILE`.

### Comandos Especiais (Manipulação de Memória e Histórico)

A linguagem suporta operações de estado por meio da palavra reservada `RES` e de identificadores arbitrários para variáveis (`MEM`). A sintaxe segue a notação polonesa reversa:

#### Leitura de Histórico `RES`: `(N RES)`
Retorna o valor computado `N` expressões atrás no histórico da FPU. O índice `N` deve ser um inteiro não negativo, onde `0` representa o último resultado avaliado. Exemplo:
```
(0 RES)
```
Carrega o resultado imediatamente anterior.

#### Escrita em Memória `STORE`: `(V MEM)`
Armazena o valor `V` (que pode ser um literal numérico ou o resultado de uma expressão aninhada) na variável identificada por `MEM`. Exemplos:
```
(10.5 CONTADOR)
```
```
((5 2 +) X)
```

#### Leitura de Memória `LOAD`: `(MEM)`
Carrega na pilha da FPU o valor armazenado na variável identificada. A variável precisa ter sido definida antes com `(V MEM)` - como `(V MEM)` define e inicializa em um único passo, ler uma memória nunca definida é **erro semântico** (uso antes da definição), reportado com a linha e o nome da variável. Exemplo:
```
((CONTADOR) 1 +)
```

## Sistema de Tipos

A linguagem é **estática e fortemente tipada**, com três tipos:

| Tipo | Descrição | Literais |
| :--- | :--- | :--- |
| `int` | inteiro | `42`, `0`, `1024` (sem ponto decimal) |
| `real` | ponto flutuante (`double` IEEE 754) | `3.14`, `10.0`, `0.5` (com ponto) |
| `bool` | lógico | `TRUE`, `FALSE` e resultados de operadores relacionais |

O tipo das variáveis é **inferido pelo contexto** de uso e **fixado** na primeira
definição (não pode mudar depois). **Não há coerção implícita** entre `int` e
`real`: misturar os dois numa mesma operação é erro semântico.

| Operador | Operandos exigidos | Resultado |
| :--- | :--- | :--- |
| `+` `-` `*` | mesmo tipo numérico | mesmo tipo |
| `\|` (divisão real) | mesmo tipo numérico | `real` |
| `^` (potência) | mesmo tipo numérico | mesmo tipo |
| `/` `%` (divisão inteira, resto) | **apenas `int`** | `int` |
| `< > <= >=` | mesmo tipo **numérico** | `bool` |
| `== !=` | mesmo tipo (`int`/`real`/`bool`) | `bool` |
| `IFELSE` | condição `bool`; ramos do mesmo tipo | tipo dos ramos |
| `WHILE` | condição `bool` | tipo do corpo |

As regras formais em **cálculo de sequentes** estão em
[`REGRAS_TIPOS.md`](REGRAS_TIPOS.md).

### Exemplos válidos e inválidos

```
*{ VALIDOS }*
(10 3 +)                 *{ int + int -> int }*
(10.5 2.5 *)             *{ real * real -> real }*
(20 6 /)                 *{ divisao inteira -> int }*
(9.0 2.0 |)              *{ divisao real -> real }*
((CONTADOR) 50 >=)       *{ relacional -> bool }*
((X 0 >) (X) (0 X) IFELSE)   *{ condicao bool, ramos compativeis }*

*{ INVALIDOS (erro semantico) }*
(10 2.5 +)               *{ mistura int com real }*
(10 3.0 /)               *{ divisao inteira exige inteiros }*
(TRUE 5 +)               *{ operando logico em aritmetica }*
(5 100 200 IFELSE)       *{ condicao do IFELSE nao e bool }*
```

Veja `tests/teste1.txt`..`teste3.txt` (programas válidos completos) e
`tests/teste4.txt` (erros semânticos intencionais).

## Gramática EBNF LL(1) Fatorada

Para garantir que o Analisador SEMÂNTICO opere de forma determinística, a gramática em Notação Polonesa Reversa (RPN) foi submetida à Fatoração à Esquerda. Isso eliminou os conflitos de derivação, e resultou na seguinte estrutura formal:

**Regras de Produção (EBNF):**

```ebnf
programa              = "(" , "START" , ")" , sequencia_execucao ;
sequencia_execucao    = "(" , avaliacao_sequencia ;
avaliacao_sequencia   = "END" , ")" | corpo_expressao , ")" , sequencia_execucao ;
expressao_aninhada    = "(" , corpo_expressao , ")" ;
operando              = NUMERO | TRUE | FALSE | IDENTIFICADOR | RES | expressao_aninhada ;
corpo_expressao       = operando , complemento_expressao ;
complemento_expressao = operando , resto_complemento | ε ;
resto_complemento     = operacao | ε ;
operacao              = OPERADOR | OPERADOR_RELACIONAL | WHILE | operando , "IFELSE" ;
```

### Dicionário de Símbolos

**Terminais (Tokens):**
`PARENTESE_ESQ`, `PARENTESE_DIR`, `START`, `END`, `NUMERO`, `IDENTIFICADOR`, `OPERADOR`, `OPERADOR_RELACIONAL`, `IFELSE`, `WHILE`, `RES`, `TRUE`, `FALSE`

**Não-Terminais:**
`programa`, `sequencia_execucao`, `avaliacao_sequencia`, `expressao_aninhada`, `operando`, `corpo_expressao`, `complemento_expressao`, `resto_complemento`, `operacao`

### Gramática atribuída, FIRST/FOLLOW e tabela LL(1)

Os conjuntos **FIRST/FOLLOW**, a **tabela de parsing LL(1)** completa e os
**atributos semânticos** (nó da AST e tipo inferido de cada produção) estão
documentados em **[`GRAMATICA.md`](GRAMATICA.md)** - calculados dinamicamente por
`src/gramatica.cpp` e impressos no início de cada execução. A ausência de colisões
em qualquer célula `M[A, t]` comprova que a gramática é **LL(1)**.
 
## Tratamento e Recuperação de Erros

O analisador implementa **recuperação de erros** (*panic mode*) com granularidade de **linha**: ao encontrar um erro, ele **não interrompe** a análise no primeiro problema - registra o erro e prossegue, de modo a reportar **todos** os erros do programa em uma única execução.

### Fluxo de análise

1. **Análise léxica** - cada linha é tokenizada isoladamente. Se uma linha contém um erro léxico (número malformado `10..5`, operador inexistente `//`, caractere inválido `@`), o erro é registrado, os tokens parciais daquela linha são descartados e a análise segue para a próxima linha.
2. **Validação estrutural** - verifica-se que o programa possui as linhas obrigatórias `(START)` e `(END)`.
3. **Análise sintática** - executa **sempre**, mesmo que existam erros léxicos. Como as linhas lexicamente inválidas já tiveram seus tokens descartados, elas não chegam ao parser e **não geram erros sintáticos falsos** (o que evita o efeito *cascata*). Cada linha de expressão é analisada isoladamente: monta-se internamente um miniprograma `(START) <linha> (END)` e invoca-se o parser LL(1); se a linha falha, o erro é registrado e a análise continua na próxima linha.
4. **Relatório de erros** - todos os erros (léxicos, sintáticos e semânticos) são acumulados, **ordenados por número de linha** e exibidos juntos, com o tipo (`LEXICO`/`SINTATICO`/`SEMANTICO`), a linha e a mensagem.
5. **Gate de geração de código** - o código Assembly **só é gerado se não houver nenhum erro**. Se houver qualquer erro, o programa exibe o relatório e encerra com código de saída `1`, sem produzir `saida.s`.

### Mensagens de erro

As mensagens incluem sempre o **número da linha** e o **tipo** do erro. Exemplo de saída para um programa com múltiplos erros:

```
============================================================
 ERROS
------------------------------------------------------------
 Total: 3 lexicos, 2 sintaticos
------------------------------------------------------------
  Linha 2 | SINTATICO | Erro de sintaxe na linha 2: nenhuma regra em M[avaliacao_sequencia][OPERADOR] para o token '+'
  Linha 3 | LEXICO | Numero malformado na linha 3: 10..
  Linha 5 | LEXICO | Erro na linha 5 operador '//' nao existe. Use '/' para divisao inteira ou '|' para real.
  Linha 6 | LEXICO | Token invalido na linha 6 '@' na posicao 5
  Linha 8 | SINTATICO | Erro de sintaxe na linha 8: esperado 'PARENTESE_DIR', encontrado 'OPERADOR' (valor: '^')
============================================================
```

Os arquivos `tests/teste_erro_lexico.txt` e `tests/teste_erro_sintatico.txt` exercitam, respectivamente, a recuperação de erros léxicos e sintáticos.


## Resultados

Para assegurar a conformidade com a norma IEEE 754 de 64 bits (Double Precision) exigida pelo edital, todos os resultados obtidos nos 32 LEDs do simulador foram submetidos a um processo de verificação cruzada.

#### Metodologia de Verificação

* **Cálculo Analítico:** Resolução manual da pilha RPN para determinar o valor decimal esperado de cada expressão.
* **Extração de Bits:** Captura dos estados lógicos dos LEDs (Palavra Alta e Palavra Baixa) a partir da injeção de JavaScript no DOM do CPULator. A montagem das palavras de 64 bits foi validada e comparada com a ferramenta online [BinaryConvert (Double Precision)](https://www.binaryconvert.com/result_double.html).

**Script de Captura (Bit Sniffer):**
```javascript
(function() {
    const leds = document.querySelectorAll('#devff200000 .dev_led_led');
    let binary = Array.from(leds)
        .map(led => led.classList.contains('dev_led_on') ? '1' : '0')
        .join('');
    let hex = parseInt(binary, 2).toString(16).toUpperCase().padStart(8, '0');
    console.log("%c ASMNATOR - Bit Sniffer ", "background: #222; color: #bada55; padding: 2px;");
    console.log("Binário (31-0): " + binary);
    console.log("Hexadecimal: 0x" + hex);
})();
```

#### Evidências de Teste (Casos de Sucesso)

A tabela abaixo detalha os testes realizados para validar a recursividade da gramática e a corretude do gerador de código para fluxos de controle e aritmética complexa.

| Caso de Teste | Expressão RPN Relevante | Resultado Esperado | Palavra Alta (Hex) | Palavra Baixa (Hex) | Status |
| :--- | :--- | :---: | :---: | :---: | :---: |
| **Potência Recursiva (T2)** | `( ( ( ( 2 2 ^ ) 2 ^ ) 2 ^ ) )` | `256.0` | `0x40700000` | `0x00000000` |  OK |
| **Lógica Condicional (T3)** | `( ((0 RES) 500.0 >=) (512.0) (0.0) IFELSE )` | `512.0` | `0x40800000` | `0x00000000` |  OK |
| **Integração Final (T4)** | `( ((CONTADOR) 5 ==) (12 SAIDA) (2026 SAIDA) IFELSE )` | `12.0` | `0x40280000` | `0x00000000` |  OK |

#### Análise de Precisão IEEE 754

No teste de **Lógica Condicional (T3)**, por exemplo, o valor decimal `512.0` é representado em hexadecimal de precisão dupla como `0x4080000000000000`. 
1. Ao pressionar **KEY1**, o simulador exibe a Palavra Alta: `0x40800000`.
2. Ao pressionar **KEY0**, o simulador exibe a Palavra Baixa: `0x00000000`.

A coincidência bit-a-bit entre o cálculo teórico e a saída do hardware valida a implementação da FPU e a sincronização de flags via `VMRS`.

## Saídas da Última Execução

O programa escreve os artefatos **no diretório de onde é executado**, com
caminhos relativos. Portanto:

- Ao executar a partir da **raiz do projeto**
  (`.\build\Release\AnalisadorSemantico.exe .\tests\teste1.txt`), os artefatos
  são gerados **na raiz** - foi assim que as cópias versionadas abaixo foram produzidas.
- Ao executar de dentro de `build/Release/`, os artefatos são gerados ali (diretório
  ignorado pelo `.gitignore`).

Os artefatos versionados no repositório correspondem à **última execução válida**,
gerada com **`tests/teste1.txt`**:

| Arquivo | Descrição |
| :--- | :--- |
| [`tokens.txt`](tokens.txt) | Vetor de tokens (saída do analisador léxico). |
| [`ast_saida.json`](ast_saida.json) | Árvore sintática **atribuída** (com `tipoDado` em cada nó), serializada em JSON. |
| [`ARVORE_ATRIBUIDA.md`](ARVORE_ATRIBUIDA.md) | Árvore atribuída em Markdown (categoria semântica + tipo de cada nó). |
| [`TABELA_SIMBOLOS.md`](TABELA_SIMBOLOS.md) | Tabela de símbolos com tipos inferidos, linha de definição e usos. |
| [`saida.s`](saida.s) | Código Assembly ARMv7 para o simulador CPUlator-ARMv7 DEC1-SOC (v16.1). |
| [`ERROS_SEMANTICOS.md`](ERROS_SEMANTICOS.md) | Relatório de erros da última execução (vazio quando o programa é válido). |

> O Assembly é gerado **somente** quando não há erros léxicos/sintáticos/semânticos.
> A AST é construída por `gerarArvore()` (`include/parser.hpp`) e exportada por
> `exportarAST()` (`include/ast_exporter.hpp`); o Assembly por `gerarAssembly()`
> (`include/armv7_generator.hpp`).

## Como Ler a Tabela de Símbolos e a Árvore Atribuída

### Tabela de símbolos (`TABELA_SIMBOLOS.md`)

A tabela registra cada variável (memória) declarada no programa, uma por linha, com quatro colunas:

- **Variável** - o nome da memória (ex.: `CONTADOR`).
- **Tipo** - o tipo fixado na primeira definição `(V MEM)`: `INT`, `REAL`, `BOOL` ou `DESCONHECIDO`.
- **Linha Definição** - a linha do código-fonte onde a variável recebeu sua primeira definição.
- **Linhas de Uso** - as linhas onde a variável foi lida `(MEM)` ou reatribuída.

A função `construirTabelaSimbolos` cria a tabela e `verificarTipos` preenche os tipos. Ela é a base para detectar uso antes da definição e reatribuição com tipo incompatível.

### Árvore atribuída (`ARVORE_ATRIBUIDA.md` e `ast_saida.json`)

A árvore sintática atribuída é a AST anotada com a informação semântica de cada nó. Cada nó mostra:

- a **categoria semântica** (ex.: `Operador aritmetico`, `Decisao (IFELSE)`, `Uso de variavel (LOAD)`);
- o **operando/valor** associado, quando há (ex.: o operador `+` ou o nome da variável);
- o **tipo inferido** (`_INT_`, `_REAL_`, `_BOOL_` ou `_DESCONHECIDO_`), resultado de `verificarTipos`;
- a **linha** do código-fonte de origem.

A indentação reflete o aninhamento das expressões RPN. O `ARVORE_ATRIBUIDA.md` traz a versão legível em Markdown; o `ast_saida.json` traz a mesma árvore serializada, com o campo `tipoDado` em cada nó. É essa árvore que justifica a geração do Assembly.

## Documentação (Artefatos de Entrega)

| Documento | Conteúdo |
| :--- | :--- |
| [`GRAMATICA.md`](GRAMATICA.md) | Gramática atribuída em EBNF, FIRST/FOLLOW e tabela LL(1). |
| [`REGRAS_TIPOS.md`](REGRAS_TIPOS.md) | Sistema de regras de tipos em cálculo de sequentes. |
| [`TABELA_SIMBOLOS.md`](TABELA_SIMBOLOS.md) | Tabela de símbolos da última execução. |
| [`ARVORE_ATRIBUIDA.md`](ARVORE_ATRIBUIDA.md) | Árvore sintática atribuída da última execução. |
| [`ERROS_SEMANTICOS.md`](ERROS_SEMANTICOS.md) | Relatório de erros da última execução (gerado mesmo se vazio). |

Os testes unitários por módulo (léxico, parser, tabela de símbolos e verificação
de tipos) são executados automaticamente no início de cada execução do programa
(`include/testes.hpp`).
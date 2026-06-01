<!--
Integrantes do grupo (ordem alfabetica):
Lucas Franco de Mello - luk4w
Nome do grupo no Canvas: RA3 2
-->

# Sistema de Regras de Tipos - Cálculo de Sequentes

Documentação formal das regras de validação de tipos da linguagem (Fase 3),
implementadas em `verificarTipos` (`src/semantic_analyzer.cpp`).

## Tipos

A linguagem possui três tipos estáticos:

- `int` - inteiro
- `real` - ponto flutuante de precisão dupla (IEEE 754)
- `bool` - lógico (`TRUE` / `FALSE`)

A tipagem é **estática e forte**, **sem coerção implícita** entre `int` e `real`.
O tipo de uma variável é fixado na sua primeira definição e não pode mudar.

## Notação

O julgamento `Γ ⊢ e : τ` lê-se *"no contexto de tipos `Γ` (a tabela de símbolos), a expressão `e` tem tipo `τ`"*. 
`Γ` mapeia nomes de variáveis (memórias) ao seu tipo inferido. As regras abaixo seguem o formato de cálculo de sequentes (premissas acima da linha, conclusão abaixo).

---

## 1. Literais

```math
\frac{\text{n é uma sequência de dígitos}}{\Gamma \vdash n : int} \quad (T\text{-}Int)
```

```math
\frac{\text{r contém um ponto decimal}}{\Gamma \vdash r : real} \quad (T\text{-}Real)
```

```math
\frac{}{\Gamma \vdash TRUE : bool} \quad (T\text{-}BoolT) \qquad \frac{}{\Gamma \vdash FALSE : bool} \quad (T\text{-}BoolF)
```

## 2. Memórias (Variáveis)

> Construção de `Γ` em duas fases. Antes da tipagem, `construirTabelaSimbolos` percorre a árvore e insere em `Γ` toda memória definida por um `(V MEM)` com o tipo ainda indeterminado, escrito `MEM : ?`
> Em seguida `verificarTipos` percorre o programa na ordem do código-fonte e resolve esses `?` no primeiro `(V MEM)` de cada memória
> Por isso a premissa de "primeira definição" abaixo é `(MEM : ?) \in \Gamma` (tipo ainda não fixado), e não `MEM \notin \Gamma` - refletindo exatamente o teste `sim.tipo == DESCONHECIDO` da implementação

**Uso - `(MEM)`** (`T-Load`)
O tipo vem da tabela. A memória deve ter sido definida e inicializada por um `(V MEM)` anterior; usar uma memória nunca definida é erro semântico (detectado em `construirTabelaSimbolos`):

```math
\frac{(MEM : \tau) \in \Gamma}{\Gamma \vdash (MEM) : \tau} \quad (T\text{-}Load)
```

**Definição / Inferência - `(V MEM)`** (`T-Store-Def`)
Quando o tipo de `MEM` ainda não foi fixado (`?`), infere-se o tipo do valor `V` e grava-se em `Γ`:

```math
\frac{\Gamma \vdash V : \tau \quad (MEM : ?) \in \Gamma \quad \tau \neq ?}{\Gamma[MEM \mapsto \tau] \vdash (V \ MEM) : \tau} \quad (T\text{-}Store\text{-}Def)
```

**Reatribuição Permitida** (`T-Store-Ok`)
Tipo já fixado e o valor tem o mesmo tipo: a tipagem forte é respeitada e o tipo se mantém:

```math
\frac{\Gamma \vdash V : \tau \quad (MEM : \tau) \in \Gamma \quad \tau \neq ?}{\Gamma \vdash (V \ MEM) : \tau} \quad (T\text{-}Store\text{-}Ok)
```

**Reatribuição Inválida** (`T-Store-Err`)
Tipo já fixado e o valor tem tipo diferente (ambos conhecidos): erro semântico:

```math
\frac{\Gamma \vdash V : \tau' \quad (MEM : \tau) \in \Gamma \quad \tau \neq ? \quad \tau' \neq ? \quad \tau \neq \tau'}{\text{Erro Semântico}} \quad (T\text{-}Store\text{-}Err)
```

> Recuperação: se `MEM` já tem tipo fixado e o valor `V` é `?` por um erro anterior nenhuma das regras de reatribuição engatilha
> o `(V MEM)` conserva o tipo já registrado de `MEM` e evita falsos positivos em cascata

## 3. Resultado Anterior - `(N RES)`

`N` deve ser um **inteiro não negativo** (`N ≥ 0`). O tipo do resultado é resolvido estaticamente a partir do histórico `H` dos resultados das expressões. O tipo de `(N RES)` é o tipo do resultado `N` posições atrás (`0` = último resultado).

```math
\frac{\Gamma \vdash N : int \quad N \geq 0 \quad H[\, |H| - 1 - N \,] = \tau}{H ; \Gamma \vdash (N \ RES) : \tau} \quad (T\text{-}Res)
```

Um `N` negativo resulta em **erro semântico**.

## 4. Operadores Aritméticos `+ - *`

Operandos devem possuir o **mesmo tipo numérico**. O resultado preserva o tipo, não havendo coerção entre `int` e `real`.

```math
\frac{\Gamma \vdash a : int \quad \Gamma \vdash b : int}{\Gamma \vdash (a \ b \ op) : int} \quad (T\text{-}Arit\text{-}Int)
```

```math
\frac{\Gamma \vdash a : real \quad \Gamma \vdash b : real}{\Gamma \vdash (a \ b \ op) : real} \quad (T\text{-}Arit\text{-}Real)
```
> Onde $op \in \{+, -, *\}$

Misturar `int` com `real` ou usar operandos do tipo `bool` resulta em **erro semântico**.

## 5. Divisão Real `|` e Potência `^`

**Divisão Real `|`**
Operandos do mesmo tipo numérico. O resultado é sempre `real`:

```math
\frac{\Gamma \vdash a : \tau \quad \Gamma \vdash b : \tau \quad \tau \in \{int, real\}}{\Gamma \vdash (a \ b \ |) : real} \quad (T\text{-}DivReal)
```

**Potência `^`**
A **base** `a` pode ser `int` ou `real`, mas o **expoente** `b` deve ser obrigatoriamente `int`. O resultado preserva o tipo da **base**:

```math
\frac{\Gamma \vdash a : \tau \quad \Gamma \vdash b : int \quad \tau \in \{int, real\}}{\Gamma \vdash (a \ b \ \hat{}) : \tau} \quad (T\text{-}Pow)
```

## 6. Divisão Inteira `/` e Resto `%`

Esses operadores são **exclusivos** de valores inteiros:

```math
\frac{\Gamma \vdash a : int \quad \Gamma \vdash b : int}{\Gamma \vdash (a \ b \ op) : int} \quad (T\text{-}DivInt)
```
> Onde $op \in \{ /, \text{\%} \}$

O uso de `real` ou `bool` com esses operadores gera **erro semântico**.

## 7. Operadores Relacionais

**Ordenação `< > <= >=`**
Operandos numéricos do mesmo tipo; o resultado é sempre lógico (`bool`):

```math
\frac{\Gamma \vdash a : \tau \quad \Gamma \vdash b : \tau \quad \tau \in \{int, real\}}{\Gamma \vdash (a \ b \ op) : bool} \quad (T\text{-}Ord)
```
> Onde $op \in \{<, >, \leq, \geq\}$

**Igualdade `== !=`**
Operandos de **qualquer** tipo, desde que sejam do mesmo tipo. O resultado é sempre lógico (`bool`):

```math
\frac{\Gamma \vdash a : \tau \quad \Gamma \vdash b : \tau \quad \tau \in \{int, real, bool\}}{\Gamma \vdash (a \ b \ op) : bool} \quad (T\text{-}Eq)
```
> Onde $op \in \{==, \neq\}$

Comparações entre tipos diferentes resultam em **erro semântico**.

## 8. Estruturas de Controle

**Decisão `IFELSE`**
A condição deve ser lógica (`bool`), e os ramos verdadeiro/falso devem possuir o **mesmo tipo**, que passará a ser o tipo inferido para o bloco de controle:

```math
\frac{\Gamma \vdash c : bool \quad \Gamma \vdash e_1 : \tau \quad \Gamma \vdash e_2 : \tau}{\Gamma \vdash (c \ e_1 \ e_2 \ IFELSE) : \tau} \quad (T\text{-}IfElse)
```

**Repetição `WHILE`**
A condição deve ser puramente lógica:

```math
\frac{\Gamma \vdash c : bool \quad \Gamma \vdash corpo : \tau}{\Gamma \vdash (c \ corpo \ WHILE) : \tau} \quad (T\text{-}While)
```

Condição diferente de `bool` (ou desajuste nos ramos do `IFELSE`) gera **erro semântico**.

---

## Recuperação de Erros

O tipo `desconhecido` (`?`) funciona como coringa: quando um operando já é `desconhecido` (devido a um erro semântico reportado anteriormente, uso de variável indefinida, ou falha ao resolver um `N RES`), as regras não engatilham novos erros sobre ele, quebrando a cascata de falsos positivos.

Todos os erros de tipo são acumulados internamente (na estrutura `vector<ErroAnalise>` sob a categoria `SEMANTICO`). O código Assembly para a VM **somente** é gerado caso as análises atinjam o final da execução com zero pendências semânticas.
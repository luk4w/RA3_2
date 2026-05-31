<!--
Integrantes do grupo (ordem alfabetica):
Lucas Franco de Mello - luk4w
Nome do grupo no Canvas: RA3 2
-->

# Sistema de Regras de Tipos — Cálculo de Sequentes

Documentação formal das regras de validação de tipos da linguagem (Fase 3),
implementadas em `verificarTipos` (`src/semantic_analyzer.cpp`).

## Tipos

A linguagem possui três tipos estáticos:

- `int` — inteiro
- `real` — ponto flutuante de precisão dupla (IEEE 754)
- `bool` — lógico (`TRUE` / `FALSE`)

A tipagem é **estática e forte**, **sem coerção implícita** entre `int` e `real`.
O tipo de uma variável é fixado na sua primeira definição e não pode mudar.

## Notação

O julgamento `Γ ⊢ e : τ` lê-se "no contexto de tipos `Γ` (a tabela de símbolos),
a expressão `e` tem tipo `τ`". `Γ` mapeia nomes de variáveis (memórias) ao seu tipo
inferido. As regras abaixo seguem o formato de cálculo de sequentes (premissas acima
da linha, conclusão abaixo).

---

## 1. Literais

```
                                  (n sem ponto decimal)
─────────────── (T-Int)         ─────────────── (T-Real)        ───────────────── (T-Bool)
 Γ ⊢ n : int                     Γ ⊢ r : real                    Γ ⊢ TRUE : bool
                                  (r com ponto)                   Γ ⊢ FALSE : bool
```

## 2. Memórias (variáveis)

**Uso — `(MEM)`** (o tipo vem do contexto, exige definição prévia):

```
 (MEM : τ) ∈ Γ
──────────────── (T-Load)
 Γ ⊢ (MEM) : τ
```

**Definição / inferência — `(V MEM)`** (infere o tipo de `MEM` a partir do valor `V`):

```
 Γ ⊢ V : τ      MEM ∉ Γ
──────────────────────────── (T-Store-Def)
 Γ, MEM : τ ⊢ (V MEM) : τ
```

**Reatribuição** (tipagem forte: o tipo não pode mudar):

```
 Γ ⊢ V : τ      (MEM : τ) ∈ Γ
──────────────────────────────── (T-Store-Ok)
 Γ ⊢ (V MEM) : τ

 Γ ⊢ V : τ'     (MEM : τ) ∈ Γ      τ ≠ τ'
────────────────────────────────────────────── (T-Store-Err)  ⇒ erro semântico
```

## 3. Resultado anterior — `(N RES)`

`N` deve ser inteiro; o tipo do resultado é resolvido em tempo de execução
(estaticamente tratado como `desconhecido`, não dispara erro de tipo):

```
 Γ ⊢ N : int
──────────────────── (T-Res)
 Γ ⊢ (N RES) : ?
```

## 4. Operadores aritméticos `+ - *`

Operandos do **mesmo tipo numérico**; o resultado preserva o tipo (sem coerção):

```
 Γ ⊢ a : int     Γ ⊢ b : int                 Γ ⊢ a : real    Γ ⊢ b : real
──────────────────────────────── (T-Arit-Int)  ──────────────────────────────── (T-Arit-Real)
 Γ ⊢ (a b op) : int                            Γ ⊢ (a b op) : real
                                               op ∈ { +, -, * }
```

Misturar `int` com `real`, ou usar `bool`, é **erro semântico**.

## 5. Divisão real `|` e potência `^`

**Divisão real `|`** — operandos do mesmo tipo numérico, resultado sempre `real`:

```
 Γ ⊢ a : τ     Γ ⊢ b : τ     τ ∈ { int, real }
──────────────────────────────────────────────── (T-DivReal)
 Γ ⊢ (a b |) : real
```

**Potência `^`** — operandos do mesmo tipo numérico, resultado preserva o tipo:

```
 Γ ⊢ a : τ     Γ ⊢ b : τ     τ ∈ { int, real }
──────────────────────────────────────────────── (T-Pow)
 Γ ⊢ (a b ^) : τ
```

## 6. Divisão inteira `/` e resto `%`

**Exclusivos de inteiros** (regra dura da especificação §2.1):

```
 Γ ⊢ a : int     Γ ⊢ b : int
──────────────────────────────── (T-DivInt)
 Γ ⊢ (a b op) : int
 op ∈ { /, % }
```

Qualquer operando `real` ou `bool` é **erro semântico**.

## 7. Operadores relacionais

**Ordenação `< > <= >=`** — operandos numéricos do mesmo tipo, resultado `bool`:

```
 Γ ⊢ a : τ     Γ ⊢ b : τ     τ ∈ { int, real }
──────────────────────────────────────────────── (T-Ord)
 Γ ⊢ (a b op) : bool
 op ∈ { <, >, <=, >= }
```

**Igualdade `== !=`** — operandos do mesmo tipo (numérico ou lógico), resultado `bool`:

```
 Γ ⊢ a : τ     Γ ⊢ b : τ     τ ∈ { int, real, bool }
──────────────────────────────────────────────────────── (T-Eq)
 Γ ⊢ (a b op) : bool
 op ∈ { ==, != }
```

Comparar tipos diferentes é **erro semântico**.

## 8. Estruturas de controle

**Decisão `IFELSE`** — condição lógica; ramos do mesmo tipo, que define o tipo do comando:

```
 Γ ⊢ c : bool     Γ ⊢ e₁ : τ     Γ ⊢ e₂ : τ
──────────────────────────────────────────────── (T-IfElse)
 Γ ⊢ (c e₁ e₂ IFELSE) : τ
```

Condição não-`bool`, ou ramos com tipos diferentes, é **erro semântico**.

**Repetição `WHILE`** — condição lógica:

```
 Γ ⊢ c : bool     Γ ⊢ corpo : τ
──────────────────────────────────── (T-While)
 Γ ⊢ (c corpo WHILE) : τ
```

Condição não-`bool` é **erro semântico**.

---

## Recuperação de erros

O tipo `desconhecido` (`?`) funciona como coringa: quando um operando já é
`desconhecido` (por causa de um erro anterior ou de um `(N RES)`), as regras não
disparam novos erros sobre ele, evitando cascatas de falsos positivos. Todos os
erros de tipo são acumulados em `vector<ErroAnalise>` (categoria `SEMANTICO`) e o
Assembly só é gerado se nenhum erro for encontrado.

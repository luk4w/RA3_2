<!--
Integrantes do grupo (ordem alfabetica):
Lucas Franco de Mello - luk4w
Nome do grupo no Canvas: RA3 2
-->

# Gramática Atribuída (EBNF) — Analisador Semântico

Gramática **LL(1)** da linguagem RPN, fatorada à esquerda e livre de conflitos.
Os conjuntos FIRST/FOLLOW e a tabela de parsing abaixo são **calculados
dinamicamente** pelo programa (`src/gramatica.cpp`) e reproduzidos aqui a partir
da saída real da execução.

## 1. Símbolos terminais (tokens)

`PARENTESE_ESQ` `(` · `PARENTESE_DIR` `)` · `START` · `END` · `NUMERO` ·
`IDENTIFICADOR` · `OPERADOR` (`+ - * | / % ^`) · `OPERADOR_RELACIONAL`
(`== != < > <= >=`) · `IFELSE` · `WHILE` · `RES` · `TRUE` · `FALSE`

## 2. Símbolos não-terminais

`programa` · `sequencia_execucao` · `avaliacao_sequencia` · `expressao_aninhada` ·
`operando` · `corpo_expressao` · `complemento_expressao` · `resto_complemento` ·
`operacao`

## 3. Produções em EBNF

```ebnf
programa              = "(" , "START" , ")" , sequencia_execucao ;

sequencia_execucao    = "(" , avaliacao_sequencia ;

avaliacao_sequencia   = "END" , ")"
                      | corpo_expressao , ")" , sequencia_execucao ;

expressao_aninhada    = "(" , corpo_expressao , ")" ;

operando              = NUMERO
                      | TRUE | FALSE
                      | IDENTIFICADOR
                      | RES
                      | expressao_aninhada ;

corpo_expressao       = operando , complemento_expressao ;

complemento_expressao = operando , resto_complemento
                      | ε ;

resto_complemento     = operacao
                      | ε ;

operacao              = OPERADOR
                      | OPERADOR_RELACIONAL
                      | WHILE
                      | operando , "IFELSE" ;
```

> A notação Polonesa Reversa (RPN) é capturada por `corpo_expressao`: os
> operandos sempre **precedem** a operação. A recursão em `operando ->
> expressao_aninhada` permite aninhamento sem limite.

## 4. Atributos semânticos (gramática atribuída)

Cada construção sintática produz um nó na AST (`ASTNode`) e recebe um atributo de
tipo (`tipoDado`) durante `verificarTipos`. As regras de tipo completas estão em
[`REGRAS_TIPOS.md`](REGRAS_TIPOS.md).

| Forma RPN | Token / produção | Nó da AST | Atributo de tipo (`tipoDado`) |
|-----------|------------------|-----------|-------------------------------|
| `42`, `3.14` | `NUMERO` | `NUMERO_LITERAL` | `INT` se sem ponto, `REAL` se com ponto |
| `TRUE`, `FALSE` | `TRUE` / `FALSE` | `BOOL_LITERAL` | `BOOL` |
| `(MEM)` | `IDENTIFICADOR` isolado | `MEMORIA_LOAD` | tipo da variável na tabela de símbolos |
| `(V MEM)` | `operando IDENTIFICADOR` | `MEMORIA_STORE` | tipo inferido de `V` (fixa o tipo da variável) |
| `(N RES)` | `operando RES` | `MEMORIA_RES` | `DESCONHECIDO` (resolvido em runtime); `N` deve ser `INT` |
| `(A B op)` | `OPERADOR` | `INSTRUCAO_VFP` | conforme operador: `+ - * ^` preservam o tipo; `\|` → `REAL`; `/ %` → `INT` |
| `(A B rel)` | `OPERADOR_RELACIONAL` | `INSTRUCAO_CMP` | `BOOL` |
| `(C T E IFELSE)` | `operando IFELSE` | `COMANDO_IFELSE` | tipo comum dos ramos `T`/`E`; `C` deve ser `BOOL` |
| `(C B WHILE)` | `WHILE` | `COMANDO_WHILE` | tipo do corpo `B`; `C` deve ser `BOOL` |

A ação de redução (`reduzirFrame` em `src/parser.cpp`) decide entre `MEMORIA_LOAD`
e `MEMORIA_STORE` conforme o `IDENTIFICADOR` apareça sozinho ou após um valor.

## 5. Conjuntos FIRST

| Não-terminal | FIRST |
|--------------|-------|
| `programa` | `(` |
| `sequencia_execucao` | `(` |
| `avaliacao_sequencia` | `END`, `NUMERO`, `TRUE`, `FALSE`, `IDENTIFICADOR`, `RES`, `(` |
| `expressao_aninhada` | `(` |
| `operando` | `NUMERO`, `TRUE`, `FALSE`, `IDENTIFICADOR`, `RES`, `(` |
| `corpo_expressao` | `NUMERO`, `TRUE`, `FALSE`, `IDENTIFICADOR`, `RES`, `(` |
| `complemento_expressao` | `NUMERO`, `TRUE`, `FALSE`, `IDENTIFICADOR`, `RES`, `(` |
| `resto_complemento` | `OPERADOR`, `OPERADOR_RELACIONAL`, `WHILE`, `NUMERO`, `TRUE`, `FALSE`, `IDENTIFICADOR`, `RES`, `(` |
| `operacao` | `OPERADOR`, `OPERADOR_RELACIONAL`, `WHILE`, `NUMERO`, `TRUE`, `FALSE`, `IDENTIFICADOR`, `RES`, `(` |

## 6. Conjuntos FOLLOW

| Não-terminal | FOLLOW |
|--------------|--------|
| `programa` | `$` |
| `sequencia_execucao` | `$` |
| `avaliacao_sequencia` | `$` |
| `expressao_aninhada` | `OPERADOR`, `OPERADOR_RELACIONAL`, `IFELSE`, `WHILE`, `NUMERO`, `TRUE`, `FALSE`, `IDENTIFICADOR`, `RES`, `(` |
| `operando` | `OPERADOR`, `OPERADOR_RELACIONAL`, `IFELSE`, `WHILE`, `NUMERO`, `TRUE`, `FALSE`, `IDENTIFICADOR`, `RES`, `(` |
| `corpo_expressao` | `)` |
| `complemento_expressao` | `)` |
| `resto_complemento` | `)` |
| `operacao` | `)` |

## 7. Tabela de Parsing LL(1)

`M[Não-terminal, Terminal] = Produção`. Células vazias indicam erro sintático.
`ε` indica derivação para vazio.

```
M[programa, (]                       = ( START ) sequencia_execucao
M[sequencia_execucao, (]             = ( avaliacao_sequencia
M[avaliacao_sequencia, END]          = END )
M[avaliacao_sequencia, ( NUMERO TRUE FALSE IDENTIFICADOR RES] = corpo_expressao ) sequencia_execucao
M[expressao_aninhada, (]             = ( corpo_expressao )
M[operando, NUMERO]                  = NUMERO
M[operando, TRUE]                    = TRUE
M[operando, FALSE]                   = FALSE
M[operando, IDENTIFICADOR]           = IDENTIFICADOR
M[operando, RES]                     = RES
M[operando, (]                       = expressao_aninhada
M[corpo_expressao, ( NUMERO TRUE FALSE IDENTIFICADOR RES] = operando complemento_expressao
M[complemento_expressao, ( NUMERO TRUE FALSE IDENTIFICADOR RES] = operando resto_complemento
M[complemento_expressao, )]          = ε
M[resto_complemento, OPERADOR OPERADOR_RELACIONAL WHILE ( NUMERO TRUE FALSE IDENTIFICADOR RES] = operacao
M[resto_complemento, )]              = ε
M[operacao, OPERADOR]                = OPERADOR
M[operacao, OPERADOR_RELACIONAL]     = OPERADOR_RELACIONAL
M[operacao, WHILE]                   = WHILE
M[operacao, ( NUMERO TRUE FALSE IDENTIFICADOR RES] = operando IFELSE
```

A ausência de colisões em qualquer célula `M[A, t]` prova que a gramática é
**LL(1)** — o parser preditivo descendente decide a produção sem retrocesso.

// Integrantes do grupo (ordem alfabetica):
// Lucas Franco de Mello - luk4w
// Nome do grupo no Canvas: RA3 2

#pragma once
#include <string>
#include <vector>
#include "tokens.hpp"

#include "ast.hpp"
#include <map>

/**
 * @brief Estrutura para representar um simbolo/variavel na tabela de simbolos.
 */
struct Simbolo
{
    std::string nome;
    TipoDado tipo;
    int linhaDefinicao;
    std::vector<int> linhasUso;
    bool inicializada = false;
};

/**
 * @brief Tabela de simbolos para mapear o nome da variavel para os dados
 */
using TabelaSimbolos = std::map<std::string, Simbolo>; // apelido para std::map<std::string, Simbolo>;

/**
 * @brief Remove comentarios *{ ... }* das linhas e preservar as posicoes
 * para nao mudar a numero da linha que acontece o erro
 * comentarios nao fechados geram um ErroAnalise do tipo LEXICO
 * @param linhas as linhas para remover os comentarios
 * @param erros struct para acumular todos os erros
 */
std::vector<std::string> removerComentarios(const std::vector<std::string> &linhas, std::vector<ErroAnalise> &erros);

/**
 * @brief Carrega o arquivo, remove comentarios *{ }* e tokeniza cada linha
 * Erros lexicos sao acumulados em `erros` sem interromper o processamento
 * Retorna os tokens brutos no formato "tipo,linha,valor"
 * @param arquivo caminho do arquivo fonte a ser carregado
 * @param erros struct para acumular todos os erros
 */
std::vector<std::string> prepararEntradaSemantica(const std::string &arquivo, std::vector<ErroAnalise> &erros);

/**
 * @brief Percorre a AST para construir a Tabela de simbolos e validar declaracoes
 * @param raiz Ponteiro para a raiz da AST
 * @param tabela Tabela de simbolos a ser preenchida
 * @param erros Vetor para acumular erros semânticos de declaração/uso
 */
void construirTabelaSimbolos(ASTNode *raiz, TabelaSimbolos &tabela, std::vector<ErroAnalise> &erros);

/**
 * @brief Exporta a tabela de simbolos para um arquivo Markdown
 */
void exportarTabelaSimbolos(const TabelaSimbolos &tabela, const std::string &arquivo);

/**
 * @brief Nome textual de um TipoDado ("INT", "REAL", "BOOL", "DESCONHECIDO")
 */
std::string nomeTipoDado(TipoDado t);

/**
 * @brief Percorre a AST inferindo e validando os tipos de cada no (pos-ordem)
 *
 * Politica de tipos (estrita, sem coercao int<->real):
 * + - *        == operandos do mesmo tipo numerico  (int x int -> int, real x real -> real)
 * | (div real) == operandos do mesmo tipo numerico -> real
 * ^ (potencia) == operandos do mesmo tipo numerico -> esse tipo
 * / %          == somente int x int -> int (regra de especificacao)
 * relacionais  == < > <= >= sobre numericos do mesmo tipo -> bool; == != sobre mesmo tipo -> bool
 * IFELSE       == condicao bool; ramos then/else do mesmo tipo
 * WHILE        == condicao bool
 * (N RES)      == N deve ser int; tipo do resultado e resolvido em runtime (DESCONHECIDO estatico)
 *
 * Anota `tipoDado` em cada no visitado e infere o tipo das variaveis na tabela
 * DESCONHECIDO funciona como coringa na recuperacao de erro para evitar falsos positivos em cascata
 *
 * @param raiz Ponteiro para a raiz da AST
 * @param tabela Tabela de simbolos com os tipos a serem inferidos
 * @param erros Vetor para acumular erros semanticos de tipo
 * @return Tipo inferido da subarvore
 */
TipoDado verificarTipos(ASTNode *raiz, TabelaSimbolos &tabela, std::vector<ErroAnalise> &erros);

/**
 * @brief Exporta a arvore sintatica atribuida e anotada com tipos em Markdown
 * Deve ser chamada apos `verificarTipos`, que anota `tipoDado` em cada no
 * Gera uma arvore indentada onde cada no exibe 
 * sua categoria semantica o operando/valor e o tipo inferido
 * @param raiz Ponteiro para a raiz da AST ja atribuida
 * @param arquivo Caminho do arquivo Markdown de saida
 */
void exportarArvoreAtribuida(ASTNode *raiz, const std::string &arquivo);

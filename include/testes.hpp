// Integrantes do grupo (ordem alfabetica):
// Lucas Franco de Mello - luk4w
// Nome do grupo no Canvas: RA3 2

#pragma once

#include "fsm_scanner.hpp"
#include "parser.hpp"
#include "gramatica.hpp"
#include "ast.hpp"
#include "semantic_analyzer.hpp"
#include <iostream>
#include <cassert>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>

// --- TESTES LEXICO ---

inline void testeComentariosSimples()
{
    std::vector<std::string> linhas = {"10 *{ comentario }* 20"};
    std::vector<ErroAnalise> erros;
    auto limpas = removerComentarios(linhas, erros);
    assert(erros.empty());
    assert(limpas[0].find("10") != std::string::npos);
    assert(limpas[0].find("20") != std::string::npos);
    std::cout << "  -> [OK] Comentario simples\n";
}

inline void testeComentariosMultiLinha()
{
    std::vector<std::string> linhas = {"10 *{ inicio", "continua", "fim }* 20"};
    std::vector<ErroAnalise> erros;
    auto limpas = removerComentarios(linhas, erros);
    assert(erros.empty());
    assert(limpas.size() == 3);
    std::cout << "  -> [OK] Comentario multi-linha\n";
}

inline void testeComentarios_ErroNaoFechado()
{
    std::vector<std::string> linhas = {"*{ erro"};
    std::vector<ErroAnalise> erros;
    removerComentarios(linhas, erros);
    assert(!erros.empty());
    std::cout << "  -> [OK] Erro: Comentario nao fechado\n";
}

inline void testeLexicoNumeros()
{
    std::vector<std::string> tokens;
    parseExpressao("10.5 42", tokens, 1);
    assert(tokens.size() == 2);
    assert(tokens[0] == "0,1,10.5");
    std::cout << "  -> [OK] Lexico: Numeros\n";
}

inline void testeLexico_Operadores()
{
    std::vector<std::string> tokens;
    parseExpressao("+ - * / | % ^ == != < > <= >=", tokens, 1);
    assert(tokens.size() == 13);
    std::cout << "  -> [OK] Lexico: Operadores\n";
}

inline void executarTestesEtapa1()
{
    std::cout << "\nRODANDO TESTES UNITARIOS: LEXICO\n";
    testeComentariosSimples();
    testeComentariosMultiLinha();
    testeComentarios_ErroNaoFechado();
    testeLexicoNumeros();
    testeLexico_Operadores();
    std::cout << "[SUCESSO] lexico validado!\n";
}

// --- HELPERS PARA PARSER ---

inline std::vector<TokenData> _tokenizarEntrada(const std::string &entrada)
{
    std::vector<std::string> linhas;
    std::istringstream ss(entrada);
    std::string linha;
    while (std::getline(ss, linha))
        linhas.push_back(linha);

    std::vector<ErroAnalise> erros;
    auto limpas = removerComentarios(linhas, erros);
    std::vector<std::string> tokens_raw;
    for (int i = 0; i < (int)limpas.size(); ++i)
        parseExpressao(limpas[i], tokens_raw, i + 1);

    std::vector<TokenData> vtokens;
    for (const auto &t : tokens_raw)
    {
        size_t p1 = t.find(',');
        size_t p2 = t.find(',', p1 + 1);
        if (p1 != std::string::npos && p2 != std::string::npos)
        {
            TokenData td;
            td.tipo = t.substr(0, p1);
            td.linha = std::stoi(t.substr(p1 + 1, p2 - p1 - 1));
            td.valor = t.substr(p2 + 1);
            vtokens.push_back(td);
        }
    }
    return vtokens;
}

inline Derivacao _parsearEntrada(const std::string &entrada)
{
    auto vtokens = _tokenizarEntrada(entrada);
    return parsear(vtokens, tabela_ll1);
}

// --- TESTES DO PARSER ---

inline void testeSintatico_Soma()
{
    auto d = _parsearEntrada("(START)\n(3 2 +)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "VADD.F64");
    delete d.raiz;
    std::cout << "  -> [OK] Soma: (3 2 +)\n";
}

inline void testeSintaticoSubtracao()
{
    auto d = _parsearEntrada("(START)\n(10 4 -)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "VSUB.F64");
    delete d.raiz;
    std::cout << "  -> [OK] Subtracao: (10 4 -)\n";
}

inline void testeSintatico_Multiplicacao()
{
    auto d = _parsearEntrada("(START)\n(5 6 *)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "VMUL.F64");
    delete d.raiz;
    std::cout << "  -> [OK] Multiplicacao: (5 6 *)\n";
}

inline void testeSintatico_DivisaoInteira()
{
    auto d = _parsearEntrada("(START)\n(10 3 /)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "DIV_INT");
    delete d.raiz;
    std::cout << "  -> [OK] Divisao inteira: (10 3 /)\n";
}

inline void testeSintatico_DivisaoReal()
{
    auto d = _parsearEntrada("(START)\n(10.0 3.0 |)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "VDIV.F64");
    delete d.raiz;
    std::cout << "  -> [OK] Divisao real: (10.0 3.0 |)\n";
}

inline void testeSintatico_Modulo()
{
    auto d = _parsearEntrada("(START)\n(10 3 %)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "MOD_INT");
    delete d.raiz;
    std::cout << "  -> [OK] Modulo: (10 3 %)\n";
}

inline void testeSintatico_Potencia()
{
    auto d = _parsearEntrada("(START)\n(2 3 ^)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "POW");
    delete d.raiz;
    std::cout << "  -> [OK] Potencia: (2 3 ^)\n";
}

inline void testeSintaticoAninhamento()
{
    auto d = _parsearEntrada("(START)\n((10 5 *) 2 +)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "VADD.F64");
    assert(d.raiz->filhos[0]->filhos[0]->opcode == "VMUL.F64");
    delete d.raiz;
    std::cout << "  -> [OK] Aninhamento: ((10 5 *) 2 +)\n";
}

inline void testeSintatico_RES()
{
    auto d = _parsearEntrada("(START)\n(10 5 +)\n(0 RES)\n(END)");
    assert(d.raiz->filhos[1]->tipo == ASTNodeType::MEMORIA_RES);
    delete d.raiz;
    std::cout << "  -> [OK] RES: (0 RES)\n";
}

inline void testeSintaticoStoreLoad()
{
    auto d = _parsearEntrada("(START)\n(42.0 VAR)\n(VAR)\n(END)");
    assert(d.raiz->filhos[0]->tipo == ASTNodeType::MEMORIA_STORE);
    assert(d.raiz->filhos[1]->tipo == ASTNodeType::MEMORIA_LOAD);
    delete d.raiz;
    std::cout << "  -> [OK] Store/Load: (42 VAR) / (VAR)\n";
}

inline void testeSintaticoIFELSE()
{
    auto d = _parsearEntrada("(START)\n((1 1 ==) (1 RES) (0 RES) IFELSE)\n(END)");
    assert(d.raiz->filhos[0]->tipo == ASTNodeType::COMANDO_IFELSE);
    delete d.raiz;
    std::cout << "  -> [OK] IFELSE\n";
}

inline void testeSintaticoWHILE()
{
    auto d = _parsearEntrada("(START)\n((1 2 <) (10 VAR) WHILE)\n(END)");
    assert(d.raiz->filhos[0]->tipo == ASTNodeType::COMANDO_WHILE);
    delete d.raiz;
    std::cout << "  -> [OK] WHILE\n";
}

inline void testeSintaticoRelacionais()
{
    const std::vector<std::string> ops = {"==", "!=", "<", ">", "<=", ">="};
    for (const auto &op : ops)
    {
        auto d = _parsearEntrada("(START)\n(1 2 " + op + ")\n(END)");
        assert(d.raiz->filhos[0]->tipo == ASTNodeType::INSTRUCAO_CMP);
        assert(d.raiz->filhos[0]->operando == op);
        delete d.raiz;
    }
    std::cout << "  -> [OK] Todos os operadores relacionais\n";
}

inline void testeSintaticoErroMultiploSTART()
{
    bool lancou = false;
    try
    {
        // A logica de START/END unico esta no main.cpp,
        // mas o parser tambem deve reclamar se encontrar START fora de contexto
        auto d = _parsearEntrada("(START)\n(START)\n(10 2 +)\n(END)");
        delete d.raiz;
    }
    catch (const std::exception &)
    {
        lancou = true;
    }
    assert(lancou);
    std::cout << "  -> [OK] Erro: Multiplo (START)\n";
}

inline void testeSintaticoErroMultiploEND()
{
    bool lancou = false;
    try
    {
        auto d = _parsearEntrada("(START)\n(10 2 +)\n(END)\n(END)");
        delete d.raiz;
    }
    catch (const std::exception &)
    {
        lancou = true;
    }
    assert(lancou);
    std::cout << "  -> [OK] Erro: Multiplo (END)\n";
}

inline void executarTestesEtapa2()
{
    std::cout << "\nRODANDO TESTES UNITARIOS: PARSER\n";
    testeSintatico_Soma();
    testeSintaticoSubtracao();
    testeSintatico_Multiplicacao();
    testeSintatico_DivisaoInteira();
    testeSintatico_DivisaoReal();
    testeSintatico_Modulo();
    testeSintatico_Potencia();
    testeSintaticoAninhamento();
    testeSintatico_RES();
    testeSintaticoStoreLoad();
    testeSintaticoIFELSE();
    testeSintaticoWHILE();
    testeSintaticoRelacionais();
    testeSintaticoErroMultiploSTART();
    testeSintaticoErroMultiploEND();

    std::cout << "[SUCESSO] Parser validado!\n";
}

// TESTES DA TABELA DE SIMBOLOS

inline void testeSemanticoUsoSemDefinicao()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    // Simula programa que tenta usar VAR sem definir
    ASTNode *raiz = new ASTNode(ASTNodeType::PROGRAMA, 0, "programa");
    raiz->filhos.push_back(new ASTNode(ASTNodeType::MEMORIA_LOAD, 1, "VLDR.F64", "VAR"));

    construirTabelaSimbolos(raiz, tabela, erros);
    assert(!erros.empty());
    assert(erros[0].tipo == "SEMANTICO");
    assert(erros[0].mensagem.find("Variavel 'VAR' usada sem ser definida") != std::string::npos);

    delete raiz;
    std::cout << "  -> [OK] Semantico: Detecao de uso sem definicao\n";
}

inline void testeSemantico_DefinicaoCorreta()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    // Simula (10 VAR) seguido de (VAR)
    ASTNode *raiz = new ASTNode(ASTNodeType::PROGRAMA, 0, "programa");

    ASTNode *store = new ASTNode(ASTNodeType::MEMORIA_STORE, 1, "VSTR.F64", "X");
    store->filhos.push_back(new ASTNode(ASTNodeType::NUMERO_LITERAL, 1, "NUMERO", "10"));

    raiz->filhos.push_back(store);
    raiz->filhos.push_back(new ASTNode(ASTNodeType::MEMORIA_LOAD, 2, "VLDR.F64", "X"));

    construirTabelaSimbolos(raiz, tabela, erros);
    assert(erros.empty());
    assert(tabela.count("X") > 0);
    assert(tabela["X"].inicializada == true);

    delete raiz;
    std::cout << "  -> [OK] Semantico: Definicao e uso validos\n";
}

inline void testeSemantico_MultiplasVariaveis()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    // (10 X V) (20 Y V) (X Y +)
    ASTNode *raiz = new ASTNode(ASTNodeType::PROGRAMA, 0, "programa");

    ASTNode *stX = new ASTNode(ASTNodeType::MEMORIA_STORE, 1, "VSTR.F64", "X");
    stX->filhos.push_back(new ASTNode(ASTNodeType::NUMERO_LITERAL, 1, "NUMERO", "10"));

    ASTNode *stY = new ASTNode(ASTNodeType::MEMORIA_STORE, 2, "VSTR.F64", "Y");
    stY->filhos.push_back(new ASTNode(ASTNodeType::NUMERO_LITERAL, 2, "NUMERO", "20"));

    ASTNode *soma = new ASTNode(ASTNodeType::INSTRUCAO_VFP, 3, "VADD.F64", "+");
    soma->filhos.push_back(new ASTNode(ASTNodeType::MEMORIA_LOAD, 3, "VLDR.F64", "X"));
    soma->filhos.push_back(new ASTNode(ASTNodeType::MEMORIA_LOAD, 3, "VLDR.F64", "Y"));

    raiz->filhos.push_back(stX);
    raiz->filhos.push_back(stY);
    raiz->filhos.push_back(soma);

    construirTabelaSimbolos(raiz, tabela, erros);
    assert(erros.empty());
    assert(tabela.size() == 2);
    assert(tabela["X"].linhasUso.size() == 1);
    assert(tabela["Y"].linhasUso.size() == 1);

    delete raiz;
    std::cout << "  -> [OK] Semantico: Multiplas variaveis e uso em expressao\n";
}

inline void testeSemanticoErroNoWhile()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    // ((VAR 10 <) (10 VAR V) WHILE) -> VAR usada na condicao antes de ser definida
    ASTNode *raiz = new ASTNode(ASTNodeType::PROGRAMA, 0, "programa");

    ASTNode *whl = new ASTNode(ASTNodeType::COMANDO_WHILE, 1, "WHILE", "WHILE");

    ASTNode *cond = new ASTNode(ASTNodeType::INSTRUCAO_CMP, 1, "<", "<");
    cond->filhos.push_back(new ASTNode(ASTNodeType::MEMORIA_LOAD, 1, "VLDR.F64", "VAR"));
    cond->filhos.push_back(new ASTNode(ASTNodeType::NUMERO_LITERAL, 1, "NUMERO", "10"));

    ASTNode *corpo = new ASTNode(ASTNodeType::MEMORIA_STORE, 2, "VSTR.F64", "VAR");
    corpo->filhos.push_back(new ASTNode(ASTNodeType::NUMERO_LITERAL, 2, "NUMERO", "10"));

    whl->filhos.push_back(cond);
    whl->filhos.push_back(corpo);
    raiz->filhos.push_back(whl);

    construirTabelaSimbolos(raiz, tabela, erros);
    assert(!erros.empty());
    assert(erros[0].linha == 1); // Erro na linha da condicao

    delete raiz;
    std::cout << "  -> [OK] Semantico: Erro de definicao dentro de estrutura WHILE\n";
}

inline void testeSemanticoReatribuicao()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    // (10 X V) (20 X V)
    ASTNode *raiz = new ASTNode(ASTNodeType::PROGRAMA, 0, "programa");
    raiz->filhos.push_back(new ASTNode(ASTNodeType::MEMORIA_STORE, 1, "VSTR.F64", "X"));
    raiz->filhos.push_back(new ASTNode(ASTNodeType::MEMORIA_STORE, 5, "VSTR.F64", "X"));

    construirTabelaSimbolos(raiz, tabela, erros);
    assert(erros.empty());
    assert(tabela["X"].linhaDefinicao == 1);
    assert(tabela["X"].linhasUso.size() == 1);
    assert(tabela["X"].linhasUso[0] == 5);

    delete raiz;
    std::cout << "  -> [OK] Semantico: Reatribuicao de variavel\n";
}

inline void executarTestesEtapa3()
{
    std::cout << "\nRODANDO TESTES UNITARIOS: TABELA DE SIMBOLOS\n";
    testeSemanticoUsoSemDefinicao();
    testeSemantico_DefinicaoCorreta();
    testeSemantico_MultiplasVariaveis();
    testeSemanticoErroNoWhile();
    testeSemanticoReatribuicao();
    std::cout << "[SUCESSO] Tabela de Simbolos validada!\n";
}

inline void executarTodosTestes()
{
    executarTestesEtapa1();
    executarTestesEtapa2();
    executarTestesEtapa3();
}
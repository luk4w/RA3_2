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

// --- TESTES DA ETAPA 1 (LEXICO + COMENTARIOS) ---

inline void testeComentariosSimples() {
    std::vector<std::string> linhas = {"10 *{ comentario }* 20"};
    std::vector<ErroAnalise> erros;
    auto limpas = removerComentarios(linhas, erros);
    assert(erros.empty());
    assert(limpas[0].find("10") != std::string::npos);
    assert(limpas[0].find("20") != std::string::npos);
    std::cout << "  -> [OK] Comentario simples\n";
}

inline void testeComentariosMultiLinha() {
    std::vector<std::string> linhas = {"10 *{ inicio", "continua", "fim }* 20"};
    std::vector<ErroAnalise> erros;
    auto limpas = removerComentarios(linhas, erros);
    assert(erros.empty());
    assert(limpas.size() == 3);
    std::cout << "  -> [OK] Comentario multi-linha\n";
}

inline void testeComentarios_ErroNaoFechado() {
    std::vector<std::string> linhas = {"*{ erro"};
    std::vector<ErroAnalise> erros;
    removerComentarios(linhas, erros);
    assert(!erros.empty());
    std::cout << "  -> [OK] Erro: Comentario nao fechado\n";
}

inline void testeLexicoNumeros() {
    std::vector<std::string> tokens;
    parseExpressao("10.5 42", tokens, 1);
    assert(tokens.size() == 2);
    assert(tokens[0] == "0,1,10.5");
    std::cout << "  -> [OK] Lexico: Numeros\n";
}

inline void testeLexico_Operadores() {
    std::vector<std::string> tokens;
    parseExpressao("+ - * / | % ^ == != < > <= >=", tokens, 1);
    assert(tokens.size() == 13);
    std::cout << "  -> [OK] Lexico: Operadores\n";
}

inline void executarTestesEtapa1() {
    std::cout << "\nRODANDO TESTES UNITARIOS: ETAPA 1 (LEXICO + COMENTARIOS)\n";
    testeComentariosSimples();
    testeComentariosMultiLinha();
    testeComentarios_ErroNaoFechado();
    testeLexicoNumeros();
    testeLexico_Operadores();
    std::cout << "[SUCESSO] Etapa 1 validada!\n";
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
    for (const auto &t : tokens_raw) {
        size_t p1 = t.find(',');
        size_t p2 = t.find(',', p1 + 1);
        if (p1 != std::string::npos && p2 != std::string::npos) {
            TokenData td;
            td.tipo  = t.substr(0, p1);
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

// --- TESTES DA ETAPA 2 (PARSER) ---

inline void testeSintatico_Soma() {
    auto d = _parsearEntrada("(START)\n(3 2 +)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "VADD.F64");
    delete d.raiz;
    std::cout << "  -> [OK] Soma: (3 2 +)\n";
}

inline void testeSintaticoSubtracao() {
    auto d = _parsearEntrada("(START)\n(10 4 -)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "VSUB.F64");
    delete d.raiz;
    std::cout << "  -> [OK] Subtracao: (10 4 -)\n";
}

inline void testeSintatico_Multiplicacao() {
    auto d = _parsearEntrada("(START)\n(5 6 *)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "VMUL.F64");
    delete d.raiz;
    std::cout << "  -> [OK] Multiplicacao: (5 6 *)\n";
}

inline void testeSintatico_DivisaoInteira() {
    auto d = _parsearEntrada("(START)\n(10 3 /)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "DIV_INT");
    delete d.raiz;
    std::cout << "  -> [OK] Divisao inteira: (10 3 /)\n";
}

inline void testeSintatico_DivisaoReal() {
    auto d = _parsearEntrada("(START)\n(10.0 3.0 |)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "VDIV.F64");
    delete d.raiz;
    std::cout << "  -> [OK] Divisao real: (10.0 3.0 |)\n";
}

inline void testeSintatico_Modulo() {
    auto d = _parsearEntrada("(START)\n(10 3 %)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "MOD_INT");
    delete d.raiz;
    std::cout << "  -> [OK] Modulo: (10 3 %)\n";
}

inline void testeSintatico_Potencia() {
    auto d = _parsearEntrada("(START)\n(2 3 ^)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "POW");
    delete d.raiz;
    std::cout << "  -> [OK] Potencia: (2 3 ^)\n";
}

inline void testeSintaticoAninhamento() {
    auto d = _parsearEntrada("(START)\n((10 5 *) 2 +)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "VADD.F64");
    assert(d.raiz->filhos[0]->filhos[0]->opcode == "VMUL.F64");
    delete d.raiz;
    std::cout << "  -> [OK] Aninhamento: ((10 5 *) 2 +)\n";
}

inline void testeSintatico_RES() {
    auto d = _parsearEntrada("(START)\n(10 5 +)\n(0 RES)\n(END)");
    assert(d.raiz->filhos[1]->tipo == ASTNodeType::MEMORIA_RES);
    delete d.raiz;
    std::cout << "  -> [OK] RES: (0 RES)\n";
}

inline void testeSintaticoStoreLoad() {
    auto d = _parsearEntrada("(START)\n(42.0 VAR)\n(VAR)\n(END)");
    assert(d.raiz->filhos[0]->tipo == ASTNodeType::MEMORIA_STORE);
    assert(d.raiz->filhos[1]->tipo == ASTNodeType::MEMORIA_LOAD);
    delete d.raiz;
    std::cout << "  -> [OK] Store/Load: (42 VAR) / (VAR)\n";
}

inline void testeSintaticoIFELSE() {
    auto d = _parsearEntrada("(START)\n((1 1 ==) (1 RES) (0 RES) IFELSE)\n(END)");
    assert(d.raiz->filhos[0]->tipo == ASTNodeType::COMANDO_IFELSE);
    delete d.raiz;
    std::cout << "  -> [OK] IFELSE\n";
}

inline void testeSintaticoWHILE() {
    auto d = _parsearEntrada("(START)\n((1 2 <) (10 VAR) WHILE)\n(END)");
    assert(d.raiz->filhos[0]->tipo == ASTNodeType::COMANDO_WHILE);
    delete d.raiz;
    std::cout << "  -> [OK] WHILE\n";
}

inline void testeSintaticoRelacionais() {
    const std::vector<std::string> ops = {"==", "!=", "<", ">", "<=", ">="};
    for (const auto &op : ops) {
        auto d = _parsearEntrada("(START)\n(1 2 " + op + ")\n(END)");
        assert(d.raiz->filhos[0]->tipo == ASTNodeType::INSTRUCAO_CMP);
        assert(d.raiz->filhos[0]->operando == op);
        delete d.raiz;
    }
    std::cout << "  -> [OK] Todos os operadores relacionais\n";
}

inline void testeSintatico_Erro_MultiploSTART() {
    bool lancou = false;
    try { 
        // A logica de START/END unico esta no main.cpp, 
        // mas o parser tambem deve reclamar se encontrar START fora de contexto
        auto d = _parsearEntrada("(START)\n(START)\n(10 2 +)\n(END)"); 
        delete d.raiz; 
    } catch (const std::exception &) { lancou = true; }
    assert(lancou);
    std::cout << "  -> [OK] Erro: Multiplo (START)\n";
}

inline void testeSintaticoErroMultiploEND() {
    bool lancou = false;
    try { 
        auto d = _parsearEntrada("(START)\n(10 2 +)\n(END)\n(END)"); 
        delete d.raiz; 
    } catch (const std::exception &) { lancou = true; }
    assert(lancou);
    std::cout << "  -> [OK] Erro: Multiplo (END)\n";
}

inline void executarTestesEtapa2() {
    std::cout << "\nRODANDO TESTES UNITARIOS: ETAPA 2 (PARSER)\n";
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
    
    // Novos testes solicitados
    testeSintatico_Erro_MultiploSTART();
    testeSintaticoErroMultiploEND();
    
    std::cout << "[SUCESSO] Etapa 2 (Parser) validada!\n";
}

inline void executarTodosTestes() {
    executarTestesEtapa1();
    executarTestesEtapa2();
}
// Integrantes do grupo (ordem alfabetica):
// Lucas Franco de Mello - luk4w
// Nome do grupo no Canvas: RA3 2

#pragma once

#include "fsm_scanner.hpp"
#include "parser.hpp"
#include "gramatica.hpp"
#include "ast.hpp"
#include "semantic_analyzer.hpp"
#include "armv7_generator.hpp"
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

// TESTES DE VERIFICACAO DE TIPOS

// Helpers para montar nos rapidamente nos testes de tipo.
inline ASTNode *noNum(const std::string &valor, int linha = 1)
{
    return new ASTNode(ASTNodeType::NUMERO_LITERAL, linha, "NUMERO", valor);
}

inline ASTNode *noOp(const std::string &op, ASTNode *a, ASTNode *b, int linha = 1)
{
    ASTNode *n = new ASTNode(ASTNodeType::INSTRUCAO_VFP, linha, op, op);
    n->filhos.push_back(a);
    n->filhos.push_back(b);
    return n;
}

inline void testeTipoSomaInteira()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    ASTNode *soma = noOp("+", noNum("3"), noNum("2")); // (3 2 +)
    TipoDado t = verificarTipos(soma, tabela, erros);
    assert(erros.empty());
    assert(t == TipoDado::INT);
    delete soma;
    std::cout << "  -> [OK] Tipos: Soma int x int -> int\n";
}

inline void testeTipoSomaRealValida()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    ASTNode *soma = noOp("+", noNum("3.0"), noNum("2.5")); // (3.0 2.5 +)
    TipoDado t = verificarTipos(soma, tabela, erros);
    assert(erros.empty());
    assert(t == TipoDado::REAL);
    delete soma;
    std::cout << "  -> [OK] Tipos: Soma real x real -> real\n";
}

inline void testeTipoMisturaIntRealErro()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    ASTNode *soma = noOp("+", noNum("3"), noNum("2.5")); // (3 2.5 +) -> erro
    verificarTipos(soma, tabela, erros);
    assert(!erros.empty());
    assert(erros[0].tipo == "SEMANTICO");
    assert(erros[0].mensagem.find("misturar tipos") != std::string::npos);
    delete soma;
    std::cout << "  -> [OK] Tipos: Mistura int x real dispara erro\n";
}

inline void testeTipoDivisaoInteiraComRealErro()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    ASTNode *div = noOp("/", noNum("10"), noNum("3.0")); // (10 3.0 /) -> erro
    verificarTipos(div, tabela, erros);
    assert(!erros.empty());
    assert(erros[0].mensagem.find("inteiros") != std::string::npos);
    delete div;
    std::cout << "  -> [OK] Tipos: Divisao inteira com real dispara erro\n";
}

inline void testeTipoDivisaoRealResultadoReal()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    ASTNode *div = noOp("|", noNum("10"), noNum("3")); // (10 3 |) -> real
    TipoDado t = verificarTipos(div, tabela, erros);
    assert(erros.empty());
    assert(t == TipoDado::REAL);
    delete div;
    std::cout << "  -> [OK] Tipos: Divisao real (|) int x int -> real\n";
}

inline void testeTipoRelacionalBool()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    ASTNode *cmp = new ASTNode(ASTNodeType::INSTRUCAO_CMP, 1, "<=", "<="); // (5 2 <=)
    cmp->filhos.push_back(noNum("5"));
    cmp->filhos.push_back(noNum("2"));
    TipoDado t = verificarTipos(cmp, tabela, erros);
    assert(erros.empty());
    assert(t == TipoDado::BOOL);
    delete cmp;
    std::cout << "  -> [OK] Tipos: Relacional int x int -> bool\n";
}

inline void testeTipoIfelseCondNaoBoolErro()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    // (5 1 0 IFELSE) -> condicao 5 (int) nao e bool
    ASTNode *ife = new ASTNode(ASTNodeType::COMANDO_IFELSE, 1, "IFELSE", "IFELSE");
    ife->filhos.push_back(noNum("5"));
    ife->filhos.push_back(noNum("1"));
    ife->filhos.push_back(noNum("0"));
    verificarTipos(ife, tabela, erros);
    assert(!erros.empty());
    assert(erros[0].mensagem.find("condicao do IFELSE") != std::string::npos);
    delete ife;
    std::cout << "  -> [OK] Tipos: Condicao de IFELSE nao-bool dispara erro\n";
}

inline void testeTipoReatribuicaoIncompativelErro()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    // (10 X) define X int; (2.0 X) tenta redefinir como real -> erro de tipagem forte
    ASTNode *raiz = new ASTNode(ASTNodeType::PROGRAMA, 0, "programa");

    ASTNode *st1 = new ASTNode(ASTNodeType::MEMORIA_STORE, 1, "VSTR.F64", "X");
    st1->filhos.push_back(noNum("10", 1));
    ASTNode *st2 = new ASTNode(ASTNodeType::MEMORIA_STORE, 2, "VSTR.F64", "X");
    st2->filhos.push_back(noNum("2.0", 2));

    raiz->filhos.push_back(st1);
    raiz->filhos.push_back(st2);

    verificarTipos(raiz, tabela, erros);
    assert(!erros.empty());
    assert(erros[0].mensagem.find("tipagem estatica e forte") != std::string::npos);
    assert(tabela["X"].tipo == TipoDado::INT); // mantem o tipo da primeira definicao
    delete raiz;
    std::cout << "  -> [OK] Tipos: Reatribuicao com tipo incompativel dispara erro\n";
}

inline void testeTipoInferenciaVariavel()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    // (3.0 Y) infere Y como real; (Y 2.0 +) usa Y -> real, sem erro
    ASTNode *raiz = new ASTNode(ASTNodeType::PROGRAMA, 0, "programa");

    ASTNode *st = new ASTNode(ASTNodeType::MEMORIA_STORE, 1, "VSTR.F64", "Y");
    st->filhos.push_back(noNum("3.0", 1));

    ASTNode *load = new ASTNode(ASTNodeType::MEMORIA_LOAD, 2, "VLDR.F64", "Y");
    ASTNode *soma = noOp("+", load, noNum("2.0", 2), 2);

    raiz->filhos.push_back(st);
    raiz->filhos.push_back(soma);

    verificarTipos(raiz, tabela, erros);
    assert(erros.empty());
    assert(tabela["Y"].tipo == TipoDado::REAL);
    assert(soma->tipoDado == TipoDado::REAL);
    delete raiz;
    std::cout << "  -> [OK] Tipos: Inferencia de tipo de variavel pelo contexto\n";
}

inline void testeTipoPotenciaIntInt()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    ASTNode *pow = noOp("^", noNum("2"), noNum("5")); // (2 5 ^) -> int
    TipoDado t = verificarTipos(pow, tabela, erros);
    assert(erros.empty());
    assert(t == TipoDado::INT);
    delete pow;
    std::cout << "  -> [OK] Tipos: Potencia int^int -> int\n";
}

inline void testeTipoPotenciaBaseRealExpoenteInt()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    ASTNode *pow = noOp("^", noNum("2.5"), noNum("3")); // (2.5 3 ^) -> real (base real, expoente int)
    TipoDado t = verificarTipos(pow, tabela, erros);
    assert(erros.empty());
    assert(t == TipoDado::REAL);
    delete pow;
    std::cout << "  -> [OK] Tipos: Potencia real^int -> real\n";
}

inline void testeTipoPotenciaExpoenteRealErro()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    ASTNode *pow = noOp("^", noNum("2.0"), noNum("0.5")); // (2.0 0.5 ^) -> expoente real -> erro
    verificarTipos(pow, tabela, erros);
    assert(!erros.empty());
    assert(erros[0].mensagem.find("expoente") != std::string::npos);
    delete pow;
    std::cout << "  -> [OK] Tipos: Potencia com expoente real dispara erro\n";
}

inline void testeTipoResHerdaTipoDoHistorico()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    // (10 5 +) ; (0 RES) -> (0 RES) herda o tipo do ultimo resultado (INT)
    ASTNode *raiz = new ASTNode(ASTNodeType::PROGRAMA, 0, "programa");
    raiz->filhos.push_back(noOp("+", noNum("10"), noNum("5")));
    ASTNode *res = new ASTNode(ASTNodeType::MEMORIA_RES, 2, "RES", "RES");
    res->filhos.push_back(noNum("0", 2));
    raiz->filhos.push_back(res);
    verificarTipos(raiz, tabela, erros);
    assert(erros.empty());
    assert(res->tipoDado == TipoDado::INT);
    delete raiz;
    std::cout << "  -> [OK] Tipos: (N RES) herda o tipo do resultado no historico\n";
}

inline void testeTipoResRecuperaBoolErro()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    // (TRUE) ; ((0 RES) 5 +) -> recupera bool via RES e soma com int -> erro
    ASTNode *raiz = new ASTNode(ASTNodeType::PROGRAMA, 0, "programa");
    raiz->filhos.push_back(new ASTNode(ASTNodeType::BOOL_LITERAL, 1, "TRUE", "TRUE"));
    ASTNode *res = new ASTNode(ASTNodeType::MEMORIA_RES, 2, "RES", "RES");
    res->filhos.push_back(noNum("0", 2));
    raiz->filhos.push_back(noOp("+", res, noNum("5", 2), 2));
    verificarTipos(raiz, tabela, erros);
    assert(!erros.empty());
    bool achou = false;
    for (const auto &e : erros)
        if (e.mensagem.find("numericos") != std::string::npos &&
            e.mensagem.find("BOOL") != std::string::npos)
            achou = true;
    assert(achou);
    delete raiz;
    std::cout << "  -> [OK] Tipos: bool recuperado por (N RES) em aritmetica dispara erro\n";
}

inline void testeTipoResIndiceNegativoErro()
{
    std::vector<ErroAnalise> erros;
    TabelaSimbolos tabela;
    // (1 2 +) ; (-1 RES) -> indice negativo -> erro
    ASTNode *raiz = new ASTNode(ASTNodeType::PROGRAMA, 0, "programa");
    raiz->filhos.push_back(noOp("+", noNum("1"), noNum("2")));
    ASTNode *res = new ASTNode(ASTNodeType::MEMORIA_RES, 2, "RES", "RES");
    res->filhos.push_back(noNum("-1", 2));
    raiz->filhos.push_back(res);
    verificarTipos(raiz, tabela, erros);
    assert(!erros.empty());
    bool achou = false;
    for (const auto &e : erros)
        if (e.mensagem.find("nao negativo") != std::string::npos)
            achou = true;
    assert(achou);
    delete raiz;
    std::cout << "  -> [OK] Tipos: (N RES) com indice negativo dispara erro\n";
}

inline void executarTestesEtapa4()
{
    std::cout << "\nRODANDO TESTES UNITARIOS: VERIFICACAO DE TIPOS\n";
    testeTipoSomaInteira();
    testeTipoSomaRealValida();
    testeTipoMisturaIntRealErro();
    testeTipoDivisaoInteiraComRealErro();
    testeTipoDivisaoRealResultadoReal();
    testeTipoRelacionalBool();
    testeTipoIfelseCondNaoBoolErro();
    testeTipoReatribuicaoIncompativelErro();
    testeTipoInferenciaVariavel();
    testeTipoPotenciaIntInt();
    testeTipoPotenciaBaseRealExpoenteInt();
    testeTipoPotenciaExpoenteRealErro();
    testeTipoResHerdaTipoDoHistorico();
    testeTipoResRecuperaBoolErro();
    testeTipoResIndiceNegativoErro();
    std::cout << "[SUCESSO] Verificacao de Tipos validada!\n";
}

// TESTES DE GERACAO DE ASSEMBLY

inline void testeAssemblyCondicaoBoolLiteral()
{
    // IFELSE com condicao = literal bool (nao-relacional)
    // gera o push do 1.0/0.0 e testa com zero (VCMP #0.0 + BEQ)
    auto d = _parsearEntrada("(START)\n((TRUE) (100) (200) IFELSE)\n(END)");
    std::string asmCode;
    gerarAssembly(d.raiz, asmCode);
    assert(asmCode.find("Literal Logico") != std::string::npos);
    assert(asmCode.find("VCMP.F64 D0, #0.0") != std::string::npos);
    assert(asmCode.find("BEQ else_label") != std::string::npos);
    delete d.raiz;
    std::cout << "  -> [OK] Assembly: condicao bool nao-relacional (VCMP #0.0 + BEQ)\n";
}

inline void testeAssemblyCondicaoRelacional()
{
    // IFELSE com condicao relacional (==) usa VCMP de dois operandos + branch inverso (BNE)
    auto d = _parsearEntrada("(START)\n((1 1 ==) (100) (200) IFELSE)\n(END)");
    std::string asmCode;
    gerarAssembly(d.raiz, asmCode);
    assert(asmCode.find("VCMP.F64 D0, D1") != std::string::npos);
    assert(asmCode.find("BNE else_label") != std::string::npos);
    delete d.raiz;
    std::cout << "  -> [OK] Assembly: condicao relacional (VCMP D0,D1 + branch inverso)\n";
}

inline void testeAssemblyPotenciaGeraLaco()
{
    // (2 5 ^) gera o laco da potenciacao
    auto d = _parsearEntrada("(START)\n(2 5 ^)\n(END)");
    std::string asmCode;
    gerarAssembly(d.raiz, asmCode);
    assert(asmCode.find("pow_loop_") != std::string::npos);
    delete d.raiz;
    std::cout << "  -> [OK] Assembly: potenciacao gera laco pow_loop\n";
}

inline void executarTestesEtapa5()
{
    std::cout << "\nRODANDO TESTES UNITARIOS: GERACAO DE ASSEMBLY\n";
    testeAssemblyCondicaoBoolLiteral();
    testeAssemblyCondicaoRelacional();
    testeAssemblyPotenciaGeraLaco();
    std::cout << "[SUCESSO] Geracao de Assembly validada!\n";
}

inline void executarTodosTestes()
{
    executarTestesEtapa1();
    executarTestesEtapa2();
    executarTestesEtapa3();
    executarTestesEtapa4();
    executarTestesEtapa5();
}
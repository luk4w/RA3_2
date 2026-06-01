// Integrantes do grupo (ordem alfabetica):
// Lucas Franco de Mello - luk4w
// Nome do grupo no Canvas: RA3 2

#include "parser.hpp"
#include <iostream>
#include <stdexcept>

using namespace std;

// Decodificador: conivelerte TokenData no nome do terminal da gramatica
static string decodificarToken(const TokenData &token, string &valorReal)
{
    if (token.tipo == "$")
    {
        valorReal = "$";
        return "$";
    }

    int tipoInt = stoi(token.tipo);
    valorReal = token.valor;

    switch (tipoInt)
    {
    case 0:
        return "NUMERO";
    case 1:
        return "IDENTIFICADOR";
    case 2:
        return "OPERADOR";
    case 3:
        return valorReal; // START, END, WHILE, IFELSE, RES
    case 4:
        return "PARENTESE_ESQ";
    case 5:
        return "PARENTESE_DIR";
    case 6:
        return "OPERADOR_RELACIONAL";
    default:
        return "";
    }
}

// Opcodes ARMv7 VFP para cada operador aritmético
static string resolverOpcode(const string &op)
{
    if (op == "+")
        return "VADD.F64";
    if (op == "-")
        return "VSUB.F64";
    if (op == "*")
        return "VMUL.F64";
    if (op == "/")
        return "DIV_INT";
    if (op == "|")
        return "VDIV.F64";
    if (op == "%")
        return "MOD_INT";
    if (op == "^")
        return "POW";
    return op;
}

// Detectar se o IDENTIFICADOR é um STORE (V MEM) ou LOAD (MEM)
// se o token anterior na pilha de operandos for um NUMERO_LITERAL ou INSTRUCAO_VFP,
// então o IDENTIFICADOR é destino de armazenamento STORE
// Caso contrário é LOAD.

// Na gramatica, complemento_expressao -> IDENTIFICADOR trata o caso "( V MEM )" onde MEM é o segundo símbolo, então pilhaOp terá o valor V
// E corpo_expressao -> IDENTIFICADOR trata "( MEM )" onde MEM é o único símbolo, então pilhaOp estará vazia para este contexto local
// parsear - parser LL(1) com duas pilhas:
//
//  pilhaLL: pilha de símbolos da gramatica
//  guia a derivação usando a tabela LL(1)
//  valida que a sequência de tokens é sintaticamente correta
//
//  pilhaOp: pilha de operandos (Nos da AST)
//  1. funciona como uma calculadora RPN
//  2. ao encontrar um operador, desempilha os operandos,
//  cria o No com filhos corretos e empilha o resultado
//
// Ao final de cada expressao aninhada (PARENTESE_DIR), o topo de pilhaOp contem o No raiz daquela subexpressao, que vira operando da expressao pai.
//
// Esta funcao retorna uma estrutura Derivacao contendo:
//   - producoes: lista ordenada de producoes aplicadas
//   - raiz: AST pre-construida pelas acoes semanticas em RPN
// A raiz e posteriormente extraida por gerarArvore()
Derivacao parsear(const vector<TokenData> &tokens,
                  const map<string, map<string, vector<string>>> &tabela_ll1)
{
    // Estrutura de derivacao (saida da funcao)
    Derivacao derivacao;

    // Pilha LL(1) de controle sintatico
    stack<string> pilhaLL;
    pilhaLL.push("$");
    pilhaLL.push("programa");

    // Pilha de operandos da construção da AST
    // Cada frame representa um nivel de parenteres abertos
    // Ao abrir "(" empilha um novo frame, vetor de Nos do nivel atual
    // Ao fechar ")" consome o frame e cria o No da expressao.
    stack<vector<ASTNode *>> pilhaFrames;

    // Frame inicial para o nivel do programa
    pilhaFrames.push({});

    size_t indexToken = 0;
    string valorReal;
    int eofLinha = tokens.empty() ? 1 : tokens.back().linha;
    TokenData eofToken = {"$", "$", eofLinha};

    string terminalAtual = tokens.empty()
                               ? "$"
                               : decodificarToken(tokens[indexToken], valorReal);

    // No raiz - será preenchido ao final
    ASTNode *raiz = new ASTNode(ASTNodeType::PROGRAMA, (tokens.empty() ? eofLinha : tokens[0].linha), "programa");

    // Lambda: avança para o próximo token
    auto avancar = [&]()
    {
        indexToken++;
        terminalAtual = (indexToken < tokens.size())
                            ? decodificarToken(tokens[indexToken], valorReal)
                            : decodificarToken(eofToken, valorReal);
    };


    // Lambda: reduz o frame atual de acordo com o operador no topo / implementa a logica RPN de montagem da AST
    //
    // Para um operador binário (OPERADOR, OPERADOR_RELACIONAL)
    //   frame = [operandoA, operandoB, op] -> op(A, B)
    //
    // Para IFELSE
    //   frame = [cond, then, else, IFELSE] -> IFELSE(cond, then, else)
    //
    // Para WHILE
    //   frame = [cond, corpo, WHILE] -> WHILE(cond, corpo)
    //
    // Para RES
    //   frame = [N, RES] -> RES(N)
    //
    // Para IDENTIFICADOR ao final do frame (STORE)
    //   frame = [valor, ID] -> STORE(valor) com operando=ID
    //
    // Para IDENTIFICADOR sozinho no frame (LOAD)
    //   frame = [ID] -> o próprio LOAD
    //
    // Para expressao aninhada sem operador externo / uma subexpressao pura
    //   frame = [resultado] -> empurra resultado pro frame pai
    auto reduzirFrame = [&]()
    {
        if (pilhaFrames.size() < 2)
            throw runtime_error("Erro interno: frame desbalanceado");

        vector<ASTNode *> frame = pilhaFrames.top();
        pilhaFrames.pop();
        vector<ASTNode *> &framePai = pilhaFrames.top();

        if (frame.empty())
        {
            // Frame vazio - não gera nada // (START) // (END)
            return;
        }

        ASTNode *ultimo = frame.back();

        // IFELSE (condicao then else IFELSE)
        if (ultimo->tipo == ASTNodeType::COMANDO_IFELSE)
        {
            if (frame.size() < 4)
                throw runtime_error("Erro: IFELSE requer 3 operandos");
            ASTNode *no = frame.back(); // o IFELSE
            frame.pop_back();
            // Os 3 operandos: cond, then, else (na ordem em que foram empilhados)
            // frame agora tem [cond, then, else]
            // mas podem ser subexpressao, entao pega os ultimos 3
            size_t n = frame.size();
            no->filhos.push_back(frame[n - 3]); // cond
            no->filhos.push_back(frame[n - 2]); // then
            no->filhos.push_back(frame[n - 1]); // else
            framePai.push_back(no);
            return;
        }

        // WHILE: (cond corpo WHILE)
        if (ultimo->tipo == ASTNodeType::COMANDO_WHILE)
        {
            if (frame.size() < 3)
                throw runtime_error("Erro: WHILE requer 2 operandos");
            ASTNode *no = frame.back();
            frame.pop_back();
            size_t n = frame.size();
            no->filhos.push_back(frame[n - 2]); // cond
            no->filhos.push_back(frame[n - 1]); // corpo
            framePai.push_back(no);
            return;
        }

        // RES: (N RES)
        if (ultimo->tipo == ASTNodeType::MEMORIA_RES)
        {
            if (frame.size() < 2)
                throw runtime_error("Erro: RES requer 1 operando (N)");
            ASTNode *no = frame.back();
            frame.pop_back();
            no->filhos.push_back(frame.back()); // N
            framePai.push_back(no);
            return;
        }

        // OPERADOR ou OPERADOR_RELACIONAL binário: (A B op)
        if (ultimo->tipo == ASTNodeType::INSTRUCAO_VFP ||
            ultimo->tipo == ASTNodeType::INSTRUCAO_CMP)
        {
            if (frame.size() < 3)
                throw runtime_error(
                    "Erro: operador '" + ultimo->operando + "' requer 2 operandos");
            ASTNode *no = frame.back();
            frame.pop_back();
            size_t n = frame.size();
            no->filhos.push_back(frame[n - 2]); // A
            no->filhos.push_back(frame[n - 1]); // B
            framePai.push_back(no);
            return;
        }

        // MEMORIA_STORE (V MEM) - identificador ao final com operando antes
        if (ultimo->tipo == ASTNodeType::MEMORIA_LOAD && frame.size() >= 2)
        {
            // Tem um valor antes do identificador -> é um STORE
            ASTNode *idNode = frame.back();
            frame.pop_back();
            ASTNode *no = new ASTNode(ASTNodeType::MEMORIA_STORE, idNode->linha,
                                      "VSTR.F64", idNode->operando);
            // O valor a armazenar é o operando anterior
            no->filhos.push_back(frame.back());
            // Libera o No de identificador temporario (era um LOAD provisorio)
            idNode->filhos.clear();
            delete idNode;
            framePai.push_back(no);
            return;
        }

        // MEMORIA_LOAD sozinho: (MEM)
        if (ultimo->tipo == ASTNodeType::MEMORIA_LOAD && frame.size() == 1)
        {
            framePai.push_back(ultimo);
            return;
        }

        // Número literal ou resultado sozinho (subexpressao pura)
        // Ex: ( (A B *) ) - o frame tem apenas o resultado de uma subexpressao
        if (frame.size() == 1)
        {
            framePai.push_back(frame.back());
            return;
        }

        // Fallback: empurra todos os Nos do frame pro pai
        // Cobre casos como subexpressões parciais não capturadas acima
        for (ASTNode *no : frame)
            framePai.push_back(no);
    };

    // Loop principal do parser LL(1)
    while (!pilhaLL.empty())
    {
        string topo = pilhaLL.top();
        pilhaLL.pop();

        // Condição de aceite
        if (topo == "$")
        {
            if (terminalAtual == "$")
                break;
            else
                throw runtime_error(
                    "Erro de sintaxe: esperado fim de entrada, encontrado '" +
                    valorReal + "'");
        }

        bool isNaoTerminal = (tabela_ll1.find(topo) != tabela_ll1.end());

        if (!isNaoTerminal)
        {
            // Terminal: verifica match e executa acao semantica
            if (topo != terminalAtual)
            {
                int linhaErro = (indexToken < tokens.size()) ? tokens[indexToken].linha : tokens.back().linha;
                throw runtime_error(
                    "Erro de sintaxe na linha " + to_string(linhaErro) +
                    ": esperado '" + topo +
                    "', encontrado '" + terminalAtual +
                    "' (valor: '" + valorReal + "')");
            }

            // acoesa semantica por tipo de terminal
            int linhaAtual = (indexToken < tokens.size()) ? tokens[indexToken].linha : (tokens.empty() ? 0 : tokens.back().linha);
            if (topo == "PARENTESE_ESQ")
            {
                // Abre novo frame de operandos para o nivel
                pilhaFrames.push({});
                avancar();
            }
            else if (topo == "PARENTESE_DIR")
            {
                // Fecha o frame atual e reduz para o frame pai
                reduzirFrame();
                avancar();
            }
            else if (topo == "NUMERO")
            {
                ASTNode *no = new ASTNode(ASTNodeType::NUMERO_LITERAL, linhaAtual,
                                          "NUMERO", valorReal);
                pilhaFrames.top().push_back(no);
                avancar();
            }
            else if (topo == "IDENTIFICADOR")
            {
                // Cria provisoriamente como LOAD; reduzirFrame decide se é STORE
                ASTNode *no = new ASTNode(ASTNodeType::MEMORIA_LOAD, linhaAtual,
                                          "VLDR.F64", valorReal);
                pilhaFrames.top().push_back(no);
                avancar();
            }
            else if (topo == "OPERADOR")
            {
                ASTNode *no = new ASTNode(ASTNodeType::INSTRUCAO_VFP, linhaAtual,
                                          resolverOpcode(valorReal), valorReal);
                pilhaFrames.top().push_back(no);
                avancar();
            }
            else if (topo == "OPERADOR_RELACIONAL")
            {
                ASTNode *no = new ASTNode(ASTNodeType::INSTRUCAO_CMP, linhaAtual,
                                          valorReal, valorReal);
                pilhaFrames.top().push_back(no);
                avancar();
            }
            else if (topo == "WHILE")
            {
                ASTNode *no = new ASTNode(ASTNodeType::COMANDO_WHILE, linhaAtual,
                                          "WHILE", "WHILE");
                pilhaFrames.top().push_back(no);
                avancar();
            }
            else if (topo == "IFELSE")
            {
                ASTNode *no = new ASTNode(ASTNodeType::COMANDO_IFELSE, linhaAtual,
                                          "IFELSE", "IFELSE");
                pilhaFrames.top().push_back(no);
                avancar();
            }
            else if (topo == "TRUE" || topo == "FALSE")
            {
                // O tipo (BOOL) e atribuido na fase semantica por verificarTipos,
                // mantendo a arvore inicial (Fase 2) puramente sintatica: todos os
                // nos nascem com tipoDado=DESCONHECIDO e so a aumentacao semantica os tipa.
                ASTNode *no = new ASTNode(ASTNodeType::BOOL_LITERAL, linhaAtual,
                                          topo, valorReal);
                pilhaFrames.top().push_back(no);
                avancar();
            }
            else if (topo == "RES")
            {
                ASTNode *no = new ASTNode(ASTNodeType::MEMORIA_RES, linhaAtual,
                                          "RES", "RES");
                pilhaFrames.top().push_back(no);
                avancar();
            }
            else
            {
                // START, END e outros terminais de pontuação: somente consome
                avancar();
            }
        }
        else
        {
            // Não-terminal: expande via tabela LL(1)
            auto &linhaTabela = tabela_ll1.at(topo);

            if (linhaTabela.find(terminalAtual) == linhaTabela.end())
            {
                int linhaErro = (indexToken < tokens.size()) ? tokens[indexToken].linha : tokens.back().linha;
                throw runtime_error(
                    "Erro de sintaxe na linha " + to_string(linhaErro) +
                    ": nenhuma regra em M[" + topo +
                    "][" + terminalAtual +
                    "] para o token '" + valorReal + "'");
            }

            const vector<string> &producao = linhaTabela.at(terminalAtual);

            // Registra a producao aplicada na estrutura de derivacao
            derivacao.producoes.push_back({topo, producao});

            // Empilha em ordem reversa, ignorando o EPSILON
            for (int i = static_cast<int>(producao.size()) - 1; i >= 0; --i)
            {
                if (producao[i] != "EPSILON")
                {
                    pilhaLL.push(producao[i]);
                }
            }
        }
    }

    // Coleta os Nos resultantes do frame raiz como filhos do PROGRAMA
    if (!pilhaFrames.empty())
    {
        for (ASTNode *no : pilhaFrames.top())
            raiz->filhos.push_back(no);
    }

    derivacao.raiz = raiz;
    return derivacao;
}

// Extrai a arvore sintatica da estrutura de derivacao produzida por parsear()
ASTNode *gerarArvore(const Derivacao &derivacao)
{
    if (derivacao.raiz == nullptr)
        throw runtime_error("gerarArvore: derivacao sem raiz (parsing falhou)");

    return derivacao.raiz;
}
// Integrantes do grupo (ordem alfabetica):
// Lucas Franco de Mello - luk4w
// Nome do grupo no Canvas: RA3 2

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include "cli_controller.hpp"
#include "fsm_scanner.hpp"
#include "armv7_generator.hpp"
#include "tokens.hpp"
#include "gramatica.hpp"
#include "parser.hpp"
#include "testes.hpp"
#include "ast_exporter.hpp"
#include "semantic_analyzer.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    // Carregar a gramatica // definir regras de producao, calcular FIRST/FOLLOW e a tabela LL(1)
    construirGramatica();

    // imprimir a tabela LL1 no formato de matriz M[nao_terminal, terminal] = regra de prod
    cout << "--- TABELA DE PARSING LL(1) ---\n";
    for (const auto &[nao_terminal, transicoes] : tabela_ll1)
    {
        for (const auto &[terminal, regra] : transicoes)
        {
            cout << "M[" << nao_terminal << ", " << terminal << "] = { ";
            for (const string &s : regra)
                cout << s << " ";
            cout << "}\n";
        }
    }
    cout << "\n\n";

    executarTodosTestes();

    // Validacao do numero de argumentos
    if (argc != 2)
    {
        cerr << "Numero de argumentos invalidos\n";
        cerr << "Uso: " << argv[0] << " <teste.txt>\n";
        return 1;
    }

    string arq = argv[1];

    // Validacao de extensao .txt
    if (arq.length() < 4 || arq.substr(arq.length() - 4) != ".txt")
    {
        cerr << "A extensao do arquivo de entrada deve ser .txt\n";
        return 1;
    }

    // struct para acumular todos os erros
    vector<ErroAnalise> erros;

    // ANALISE LEXICA
    // ler o arquivo, remover comentários "*{" "}*" e tokenizar
    vector<string> tokens_linha;
    try
    {
        tokens_linha = prepararEntradaSemantica(arq, erros);
    }
    catch (const std::exception &e)
    {
        cerr << "Erro ao ler arquivo " << arq << "\n";
        return 1;
    }

    try
    {
        // Salvar os tokens em um arquivo da ultima execucao
        ofstream tokenFile("tokens.txt");
        for (const auto &token : tokens_linha)
        {
            tokenFile << token << "\n";
        }
        tokenFile.close();
    }
    catch (std::exception &e)
    {
        cerr << "Falha ao salvar tokens " << e.what() << "\n";
        return 1;
    }

    std::vector<TokenData> vtokens;
    try
    {
        // Ler o arquivo tokens.txt salvo
        vtokens = lerTokens("tokens.txt");
    }
    catch (std::exception &e)
    {
        cerr << "Falha ao ler arquivo de tokens" << e.what() << "\n";
        return 1;
    }

    // executarTestesLexicos();

    // ANALISE SINTATICA
    // Roda mesmo que o lexico tenha acusado erros
    // as linhas que falharam no lexico ja tiveram seus tokens descartados
    // logo nao chegam aqui e naogeram erros sintaticos falsos
    // recuperacao por linha, sem efeito cascata

    // VALIDACAO ESTRUTURAL DO PROGRAMA
    // (START) e (END) devem aparecer exatamente uma vez, como primeira e ultima linha
    if (!vtokens.empty())
    {
        int countStart = 0, countEnd = 0;
        for (const TokenData &tk : vtokens)
            if (tk.tipo == T_PALAVRA_RES)
            {
                if (tk.valor == "START")
                    countStart++;
                if (tk.valor == "END")
                    countEnd++;
            }

        // (START) gera os tokens PARENTESE_ESQ, START, PARENTESE_DIR.
        // Logo, o primeiro token e PARENTESE_ESQ e o segundo deve ser START.
        bool comecaStart = (vtokens.size() >= 2 &&
                            vtokens[1].tipo == T_PALAVRA_RES &&
                            vtokens[1].valor == "START");

        // (END) gera PARENTESE_ESQ, END, PARENTESE_DIR.
        // Logo, o penultimo token deve ser END.
        bool terminaEnd = (vtokens.size() >= 2 &&
                           vtokens[vtokens.size() - 2].tipo == T_PALAVRA_RES &&
                           vtokens[vtokens.size() - 2].valor == "END");

        if (!comecaStart)
            erros.push_back(ErroAnalise{vtokens.front().linha, "SINTATICO",
                                        "programa deve comecar com a linha (START)"});

        if (!terminaEnd)
            erros.push_back(ErroAnalise{vtokens.back().linha, "SINTATICO",
                                        "programa deve terminar com a linha (END)"});

        if (countStart > 1)
            erros.push_back(ErroAnalise{vtokens.front().linha, "SINTATICO",
                                        "(START) deve aparecer exatamente uma vez, apenas na primeira linha"});

        if (countEnd > 1)
            erros.push_back(ErroAnalise{vtokens.back().linha, "SINTATICO",
                                        "(END) deve aparecer exatamente uma vez, apenas na ultima linha"});
    }

    // Cada linha entre (START) e (END) e uma expressao independente
    // Para coletar todos os erros e nao parar no primeiro
    // cada linha e analisada isoladamente
    // monta-se um miniprograma "(START) <linha> (END)" e chama-se parsear()
    // Se uma linha falha, o erro e registrado e a analise segue para a proxima
    // As subarvores validas sao reunidas numa unica AST
    // Os erros de todas as fases sao acumulados
    // o unico gate fica antes do assembly e nada e gerado se houver qualquer erro
    ASTNode *arvore = new ASTNode(ASTNodeType::PROGRAMA, 0, "programa");
    {
        // Localizar os tokens de (START) e (END) para reaproveitar no envoltorio
        TokenData tParEsq{T_PAREN_ESQ, "(", 0};
        TokenData tParDir{T_PAREN_DIR, ")", 0};
        TokenData tStart{T_PALAVRA_RES, "START", 0};
        TokenData tEnd{T_PALAVRA_RES, "END", 0};

        // Agrupa os tokens por numero de linha, ignorando as linhas de (START)/(END),
        // pois cada linha de expressao sera reembrulhada individualmente.
        std::vector<std::pair<int, std::vector<TokenData>>> linhasExpr;
        std::unordered_set<int> linhasStartEnd;
        for (const TokenData &tk : vtokens)
            if (tk.tipo == T_PALAVRA_RES && (tk.valor == "START" || tk.valor == "END"))
                linhasStartEnd.insert(tk.linha);

        for (const TokenData &tk : vtokens)
        {
            if (linhasStartEnd.count(tk.linha))
                continue; // pula a linha inteira do START/END
            if (linhasExpr.empty() || linhasExpr.back().first != tk.linha)
                linhasExpr.push_back({tk.linha, {}});
            linhasExpr.back().second.push_back(tk);
        }

        // Analisa cada linha de expressao isoladamente.
        for (auto &[numLinha, toksLinha] : linhasExpr)
        {
            if (toksLinha.empty())
                continue;

            // Monta o miniprograma (START) <tokens da linha> (END)
            std::vector<TokenData> programaLinha;
            programaLinha.push_back(tParEsq);
            programaLinha.push_back(tStart);
            programaLinha.push_back(tParDir);
            for (const TokenData &t : toksLinha)
                programaLinha.push_back(t);
            programaLinha.push_back(tParEsq);
            programaLinha.push_back(tEnd);
            programaLinha.push_back(tParDir);

            try
            {
                Derivacao d = parsear(programaLinha, tabela_ll1);
                ASTNode *raizLinha = gerarArvore(d);
                // Move as subarvores desta linha para a arvore-programa final
                if (raizLinha)
                {
                    for (ASTNode *filho : raizLinha->filhos)
                        arvore->filhos.push_back(filho);
                    raizLinha->filhos.clear(); // evita que o destrutor apague os filhos movidos
                    delete raizLinha;
                }
            }

            catch (const std::exception &e)
            {
                std::string msg = e.what();
                while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r'))
                    msg.pop_back();
                erros.push_back(ErroAnalise{numLinha, "SINTATICO", msg});
            }
        }
    }

    // ANALISE SEMANTICA
    TabelaSimbolos tabelaSimbolos;
    construirTabelaSimbolos(arvore, tabelaSimbolos, erros);

    // Verificacao e inferencia de tipos
    // anotar tipoDado nos nós e na tabela
    verificarTipos(arvore, tabelaSimbolos, erros);

    // Exporta a tabela ja com os tipos inferidos pela verificacao
    exportarTabelaSimbolos(tabelaSimbolos, "TABELA_SIMBOLOS.md");

    // Relatorio de erros
    if (!erros.empty())
    {
        // Agrupa os erros por linha
        // lexicos, sintaticos e semanticos lado a lado
        // stable_sort preserva a ordem das fases dentro de uma mesma linha
        std::stable_sort(erros.begin(), erros.end(),
                         [](const ErroAnalise &a, const ErroAnalise &b)
                         { return a.linha < b.linha; });

        // Conta os erros por categoria
        size_t nLexicos = 0, nSintaticos = 0, nSemanticos = 0;
        for (const ErroAnalise &erro : erros)
        {
            if (erro.tipo == "LEXICO")
                nLexicos++;
            else if (erro.tipo == "SINTATICO")
                nSintaticos++;
            else if (erro.tipo == "SEMANTICO")
                nSemanticos++;
        }

        std::string resumo;
        if (nLexicos > 0)
            resumo = std::to_string(nLexicos) + " lexico" + (nLexicos == 1 ? "" : "s");
        if (nSintaticos > 0)
            resumo += (resumo.empty() ? "" : ", ") +
                      std::to_string(nSintaticos) + " sintatico" + (nSintaticos == 1 ? "" : "s");
        if (nSemanticos > 0)
            resumo += (resumo.empty() ? "" : ", ") +
                      std::to_string(nSemanticos) + " semantico" + (nSemanticos == 1 ? "" : "s");

        cerr << "\n";
        cerr << "============================================================\n";
        cerr << " ERROS\n";
        cerr << "------------------------------------------------------------\n";
        cerr << " Total: " << resumo << "\n";
        cerr << "------------------------------------------------------------\n";

        for (const ErroAnalise &erro : erros)
            cerr << "  Linha " << erro.linha
                 << " | " << erro.tipo
                 << " | " << erro.mensagem << "\n";
        cerr << "============================================================\n";

        // Encerra APOS reportar TODOS os erros encontrados.
        delete arvore;
        return 1;
    }

    // Exportar a AST 
    // ja atribuida com os tipos inferidos por verificarTipos
    if (arvore)
    {
        exportarAST(arvore, "ast_saida.json");
        exportarArvoreAtribuida(arvore, "ARVORE_ATRIBUIDA.md");
    }

    // Geracao de codigo Assembly ARMv7 para Cpulator-ARMv7 DEC1-SOC(v16.1)
    // a partir da AST
    try
    {
        std::string codigoAssembly;
        gerarAssembly(arvore, codigoAssembly);

        ofstream asmFile("saida.s");
        if (asmFile.is_open())
        {
            asmFile << codigoAssembly;
            asmFile.close();
            cout << "Assembly gerado com sucesso em: saida.s\n";
        }
        else
        {
            cerr << "Erro ao criar arquivo de saida Assembly.\n";
        }
    }
    catch (const std::exception &e)
    {
        cerr << "Erro na geracao do Assembly: " << e.what() << "\n";
        delete arvore;
        return 1;
    }

    delete arvore;
    return 0;
}

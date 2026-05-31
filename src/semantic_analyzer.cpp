// Integrantes do grupo (ordem alfabetica):
// Lucas Franco de Mello - luk4w
// Nome do grupo no Canvas: RA3 2

#include "semantic_analyzer.hpp"
#include "cli_controller.hpp"
#include "fsm_scanner.hpp"
#include <stdexcept>
#include <fstream>
#include <iomanip>

std::vector<std::string> removerComentarios(const std::vector<std::string> &linhas, std::vector<ErroAnalise> &erros)
{
    std::vector<std::string> resultado(linhas.size());
    bool dentro = false;
    int linha_abertura = -1;

    for (size_t i = 0; i < linhas.size(); ++i)
    {
        const std::string &linha = linhas[i];
        std::string &saida = resultado[i];
        saida.reserve(linha.size());
        int numero_linha = (int)(i + 1);

        size_t pos = 0;
        while (pos < linha.size())
        {
            if (!dentro) // nao ta dentro do comentario
            {
                // detectar a abertura de comentario *{
                if (pos + 1 < linha.size() && linha[pos] == '*' && linha[pos + 1] == '{')
                {
                    dentro = true;
                    linha_abertura = numero_linha;
                    saida += "  "; // preservar a largura para manter posicoes de erro
                    pos += 2;
                }
                // detecta fechamento sem abertura correspondente }*
                else if (pos + 1 < linha.size() && linha[pos] == '}' && linha[pos + 1] == '*')
                {
                    erros.push_back(ErroAnalise{numero_linha, "LEXICO",
                                                "fechamento de comentario '}*' sem abertura '*{' correspondente"});
                    saida += "  ";
                    pos += 2;
                }
                else
                {
                    saida += linha[pos];
                    pos++;
                }
            }
            else // dentro do comentario
            {
                // detectar fechamento }*
                if (pos + 1 < linha.size() && linha[pos] == '}' && linha[pos + 1] == '*')
                {
                    dentro = false;
                    linha_abertura = -1;
                    saida += "  ";
                    pos += 2;
                }
                else
                {
                    saida += ' '; // substitui conteúdo do comentario por espaços
                    pos++;
                }
            }
        }
    }

    if (dentro)
    {
        // erro de comentario mal formado sem fechar
        erros.push_back(ErroAnalise{linha_abertura, "LEXICO",
                                    "comentario '*{' aberto na linha " + std::to_string(linha_abertura) +
                                        " nao foi fechado com '}*'"});
    }

    return resultado;
}

std::vector<std::string> prepararEntradaSemantica(const std::string &arquivo, std::vector<ErroAnalise> &erros)
{
    std::vector<std::string> linhas;
    lerArquivo(arquivo, linhas); // lança exceção se o arquivo nao existir ou estiver vazio

    // Remove comentarios *{ }* antes de tokenizar
    std::vector<std::string> linhas_limpas = removerComentarios(linhas, erros);

    // Tokeniza cada linha limpa, acumulando erros léxicos sem interromper
    std::vector<std::string> tokens;
    for (size_t i = 0; i < linhas_limpas.size(); ++i)
    {
        size_t marca = tokens.size();
        try
        {
            parseExpressao(linhas_limpas[i], tokens, (int)(i + 1));
        }
        catch (const std::exception &e)
        {
            tokens.resize(marca); // descarta tokens parciais da linha com erro
            std::string msg = e.what();
            while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r'))
                msg.pop_back();
            erros.push_back(ErroAnalise{(int)(i + 1), "LEXICO", msg});
        }
    }

    return tokens;
}

void construirTabelaSimbolos(ASTNode *raiz, TabelaSimbolos &tabela, std::vector<ErroAnalise> &erros)
{
    if (!raiz)
        return;

    // Se for um STORE (V MEM)
    if (raiz->tipo == ASTNodeType::MEMORIA_STORE)
    {
        std::string nome = raiz->operando;
        if (tabela.find(nome) == tabela.end())
        {
            // Primeira vez que o comp viu a variavel define
            tabela[nome] = {nome, TipoDado::DESCONHECIDO, raiz->linha, {}, true};
        }
        else
        {
            // reatribuicao -> uso/escrita
            tabela[nome].inicializada = true;
            tabela[nome].linhasUso.push_back(raiz->linha);
        }
    }
    // Se for um LOAD (MEM)
    else if (raiz->tipo == ASTNodeType::MEMORIA_LOAD)
    {
        std::string nome = raiz->operando;
        // rerificar se é uma palavra reservada
        if (nome != "TRUE" && nome != "FALSE")
        {
            if (tabela.find(nome) == tabela.end() || !tabela[nome].inicializada)
            {
                // nao devem entrar na tabela como variaveis
                erros.push_back(ErroAnalise{raiz->linha, "SEMANTICO",
                                            "Variavel '" + nome + "' usada sem ser definida previamente com (V " + nome + ")"});
                // Registra mesmo com erro para evitar erros duplicados
                if (tabela.find(nome) == tabela.end())
                    tabela[nome] = {nome, TipoDado::DESCONHECIDO, -1, {raiz->linha}, false};
            }
            else
            {
                tabela[nome].linhasUso.push_back(raiz->linha);
            }
        }
    }

    // recursao nos filhos para garantir que toda a arvore seja processada
    for (ASTNode *filho : raiz->filhos)
    {
        construirTabelaSimbolos(filho, tabela, erros);
    }
}

std::string nomeTipoDado(TipoDado t)
{
    switch (t)
    {
    case TipoDado::INT:
        return "INT";
    case TipoDado::REAL:
        return "REAL";
    case TipoDado::BOOL:
        return "BOOL";
    default:
        return "DESCONHECIDO";
    }
}

void exportarTabelaSimbolos(const TabelaSimbolos &tabela, const std::string &arquivo)
{
    std::ofstream out(arquivo);
    if (!out.is_open())
        return;

    out << "# Tabela de Símbolos\n\n";
    out << "| Variável | Tipo | Linha Definição | Linhas de Uso |\n";
    out << "|----------|------|-----------------|---------------|\n";

    for (const auto &[nome, sim] : tabela)
    {
        std::string tipoStr = nomeTipoDado(sim.tipo);

        std::string linhasUso;
        for (size_t i = 0; i < sim.linhasUso.size(); ++i)
        {
            linhasUso += std::to_string(sim.linhasUso[i]);
            if (i < sim.linhasUso.size() - 1)
                linhasUso += ", ";
        }

        out << "| " << nome << " | " << tipoStr << " | "
            << (sim.linhaDefinicao != -1 ? std::to_string(sim.linhaDefinicao) : "N/A")
            << " | " << (linhasUso.empty() ? "-" : linhasUso) << " |\n";
    }

    out.close();
}

// Verificacao de tipos
namespace
{
    // Verdadeiro para int ou real
    bool isNumerico(TipoDado t)
    {
        return t == TipoDado::INT || t == TipoDado::REAL;
    }

    // Acrescenta um erro semantico ao acumulador
    void erroTipo(std::vector<ErroAnalise> &erros, int linha, const std::string &msg)
    {
        erros.push_back(ErroAnalise{linha, "SEMANTICO", msg});
    }

    // Valida um operador aritmetico/exponencial binario e devolve o tipo do resultado
    // Politica estrita
    // operandos devem ter o mesmo tipo numerico
    TipoDado tiparAritmetico(const std::string &op, TipoDado a, TipoDado b, int linha, std::vector<ErroAnalise> &erros)
    {
        // Divisao inteira e resto
        // exclusivos de inteiros (regra de especificacao)
        if (op == "/" || op == "%")
        {
            bool aRuim = (a != TipoDado::INT && a != TipoDado::DESCONHECIDO);
            bool bRuim = (b != TipoDado::INT && b != TipoDado::DESCONHECIDO);
            if (aRuim || bRuim)
            {
                std::string nomeOp = (op == "/") ? "divisao inteira '/'" : "resto '%'";
                erroTipo(erros, linha,
                         "operador de " + nomeOp + " exige operandos inteiros, encontrado " +
                             nomeTipoDado(a) + " e " + nomeTipoDado(b));
            }
            return TipoDado::INT; // assume int na recuperacao de erro
        }

        // Demais operadores + - * | ^
        // exigem operandos numericos do mesmo tipo
        if (a == TipoDado::DESCONHECIDO && b == TipoDado::DESCONHECIDO)
            return TipoDado::DESCONHECIDO;

        bool aNaoNum = (a != TipoDado::DESCONHECIDO && !isNumerico(a));
        bool bNaoNum = (b != TipoDado::DESCONHECIDO && !isNumerico(b));
        if (aNaoNum || bNaoNum)
        {
            erroTipo(erros, linha,
                     "operador '" + op + "' exige operandos numericos, encontrado " +
                         nomeTipoDado(a) + " e " + nomeTipoDado(b));
            return TipoDado::DESCONHECIDO;
        }

        // Ambos numericos ou um deles DESCONHECIDO
        // nao pode misturar int com real
        if (a != TipoDado::DESCONHECIDO && b != TipoDado::DESCONHECIDO && a != b)
        {
            erroTipo(erros, linha,
                     "operador '" + op + "' nao permite misturar tipos: " +
                         nomeTipoDado(a) + " com " + nomeTipoDado(b) +
                         " (tipagem forte, sem coercao)");
            return TipoDado::DESCONHECIDO;
        }

        // Tipo numerico conhecido 
        // resolve mesmo se um lado for DESCONHECIDO
        TipoDado num = (a != TipoDado::DESCONHECIDO) ? a : b;

        if (op == "|") // divisao real com resultado sempre real
            return TipoDado::REAL;
        return num; // + - * ^ preservam o tipo dos operandos
    }

    // Valida um operador relacional e devolve bool
    TipoDado tiparRelacional(const std::string &op, TipoDado a, TipoDado b,
                             int linha, std::vector<ErroAnalise> &erros)
    {
        if (a == TipoDado::DESCONHECIDO || b == TipoDado::DESCONHECIDO)
            return TipoDado::BOOL;

        bool ordenacao = (op == "<" || op == ">" || op == "<=" || op == ">=");
        if (ordenacao && (!isNumerico(a) || !isNumerico(b)))
        {
            erroTipo(erros, linha,
                     "operador relacional '" + op + "' exige operandos numericos, encontrado " +
                         nomeTipoDado(a) + " e " + nomeTipoDado(b));
            return TipoDado::BOOL;
        }

        if (a != b)
        {
            erroTipo(erros, linha,
                     "operador relacional '" + op + "' nao permite comparar tipos diferentes: " +
                         nomeTipoDado(a) + " com " + nomeTipoDado(b));
        }
        return TipoDado::BOOL;
    }
}

TipoDado verificarTipos(ASTNode *raiz, TabelaSimbolos &tabela, std::vector<ErroAnalise> &erros)
{
    if (!raiz)
        return TipoDado::DESCONHECIDO;

    switch (raiz->tipo)
    {
    case ASTNodeType::PROGRAMA:
    case ASTNodeType::SEQUENCIA:
        // Processa cada expressao na ordem do codigo-fonte 
        // para que os STORE infiram o tipo das variaveis antes dos LOAD subsequentes
        for (ASTNode *filho : raiz->filhos)
            verificarTipos(filho, tabela, erros);
        raiz->tipoDado = TipoDado::DESCONHECIDO;
        return TipoDado::DESCONHECIDO;

    case ASTNodeType::NUMERO_LITERAL:
        // Literal real se contiver ponto decimal, se nao inteiro
        raiz->tipoDado = (raiz->operando.find('.') != std::string::npos)
                             ? TipoDado::REAL
                             : TipoDado::INT;
        return raiz->tipoDado;

    case ASTNodeType::BOOL_LITERAL:
        raiz->tipoDado = TipoDado::BOOL;
        return TipoDado::BOOL;

    case ASTNodeType::MEMORIA_LOAD:
    {
        // (MEM) o tipo vem da tabela de simbolos 
        // ja preenchida pelo STORE
        auto it = tabela.find(raiz->operando);
        raiz->tipoDado = (it != tabela.end()) ? it->second.tipo : TipoDado::DESCONHECIDO;
        return raiz->tipoDado;
    }

    case ASTNodeType::MEMORIA_STORE:
    {
        // (V MEM)
        // infere/valida o tipo da variavel a partir do valor armazenado
        TipoDado tipoValor = raiz->filhos.empty()
                                 ? TipoDado::DESCONHECIDO
                                 : verificarTipos(raiz->filhos[0], tabela, erros);

        Simbolo &sim = tabela[raiz->operando];
        if (sim.tipo == TipoDado::DESCONHECIDO)
        {
            sim.tipo = tipoValor; // primeira definicao fixa o tipo
        }
        else if (tipoValor != TipoDado::DESCONHECIDO && sim.tipo != tipoValor)
        {
            // Tipagem forte
            // o tipo de uma variavel nao pode mudar apos a definicao.
            erroTipo(erros, raiz->linha,
                     "variavel '" + raiz->operando + "' definida como " + nomeTipoDado(sim.tipo) +
                         " nao pode receber valor do tipo " + nomeTipoDado(tipoValor) +
                         " (tipagem estatica e forte)");
        }
        raiz->tipoDado = sim.tipo;
        return raiz->tipoDado;
    }

    case ASTNodeType::MEMORIA_RES:
    {
        // (N RES)
        // N deve ser inteiro 
        // o tipo do resultado e resolvido em runtime
        if (!raiz->filhos.empty())
        {
            TipoDado tipoN = verificarTipos(raiz->filhos[0], tabela, erros);
            if (tipoN != TipoDado::INT && tipoN != TipoDado::DESCONHECIDO)
                erroTipo(erros, raiz->linha,
                         "o indice N de (N RES) deve ser inteiro, encontrado " + nomeTipoDado(tipoN));
        }
        raiz->tipoDado = TipoDado::DESCONHECIDO;
        return TipoDado::DESCONHECIDO;
    }

    case ASTNodeType::INSTRUCAO_VFP:
    {
        // Operador aritmetico binario: + - * | / % ^
        TipoDado a = raiz->filhos.size() > 0 ? verificarTipos(raiz->filhos[0], tabela, erros) : TipoDado::DESCONHECIDO;
        TipoDado b = raiz->filhos.size() > 1 ? verificarTipos(raiz->filhos[1], tabela, erros) : TipoDado::DESCONHECIDO;
        raiz->tipoDado = tiparAritmetico(raiz->operando, a, b, raiz->linha, erros);
        return raiz->tipoDado;
    }

    case ASTNodeType::INSTRUCAO_CMP:
    {
        // Operador relacional binario
        TipoDado a = raiz->filhos.size() > 0 ? verificarTipos(raiz->filhos[0], tabela, erros) : TipoDado::DESCONHECIDO;
        TipoDado b = raiz->filhos.size() > 1 ? verificarTipos(raiz->filhos[1], tabela, erros) : TipoDado::DESCONHECIDO;
        raiz->tipoDado = tiparRelacional(raiz->operando, a, b, raiz->linha, erros);
        return raiz->tipoDado;
    }

    case ASTNodeType::COMANDO_IFELSE:
    {
        // (cond then else IFELSE)
        // condicao logica; ramos do mesmo tipo
        TipoDado cond = raiz->filhos.size() > 0 ? verificarTipos(raiz->filhos[0], tabela, erros) : TipoDado::DESCONHECIDO;
        TipoDado entao = raiz->filhos.size() > 1 ? verificarTipos(raiz->filhos[1], tabela, erros) : TipoDado::DESCONHECIDO;
        TipoDado senao = raiz->filhos.size() > 2 ? verificarTipos(raiz->filhos[2], tabela, erros) : TipoDado::DESCONHECIDO;

        if (cond != TipoDado::BOOL && cond != TipoDado::DESCONHECIDO)
            erroTipo(erros, raiz->linha,
                     "a condicao do IFELSE deve ser logica (BOOL), encontrado " + nomeTipoDado(cond));

        // Tipo do IFELSE 
        // tipo comum dos ramos
        // com recuperacao via DESCONHECIDO
        if (entao == TipoDado::DESCONHECIDO)
            raiz->tipoDado = senao;
        else if (senao == TipoDado::DESCONHECIDO)
            raiz->tipoDado = entao;
        else if (entao == senao)
            raiz->tipoDado = entao;
        else
        {
            erroTipo(erros, raiz->linha,
                     "os ramos do IFELSE tem tipos diferentes: " + nomeTipoDado(entao) +
                         " (entao) e " + nomeTipoDado(senao) + " (senao)");
            raiz->tipoDado = TipoDado::DESCONHECIDO;
        }
        return raiz->tipoDado;
    }

    case ASTNodeType::COMANDO_WHILE:
    {
        // (cond corpo WHILE)
        // condicao logica
        TipoDado cond = raiz->filhos.size() > 0 ? verificarTipos(raiz->filhos[0], tabela, erros) : TipoDado::DESCONHECIDO;
        TipoDado corpo = raiz->filhos.size() > 1 ? verificarTipos(raiz->filhos[1], tabela, erros) : TipoDado::DESCONHECIDO;

        if (cond != TipoDado::BOOL && cond != TipoDado::DESCONHECIDO)
            erroTipo(erros, raiz->linha,
                     "a condicao do WHILE deve ser logica (BOOL), encontrado " + nomeTipoDado(cond));

        raiz->tipoDado = corpo;
        return raiz->tipoDado;
    }

    default:
        raiz->tipoDado = TipoDado::DESCONHECIDO;
        return TipoDado::DESCONHECIDO;
    }
}

// Exportar arvore sintatica atribuida
namespace
{
    // Categoria semantica legivel de cada tipo de no.
    std::string categoriaSemantica(ASTNodeType t)
    {
        switch (t)
        {
        case ASTNodeType::PROGRAMA: return "Programa";
        case ASTNodeType::SEQUENCIA: return "Sequencia";
        case ASTNodeType::INSTRUCAO_VFP: return "Operador aritmetico";
        case ASTNodeType::INSTRUCAO_CMP: return "Operador relacional";
        case ASTNodeType::MEMORIA_LOAD: return "Uso de variavel (LOAD)";
        case ASTNodeType::MEMORIA_STORE: return "Definicao de variavel (STORE)";
        case ASTNodeType::MEMORIA_RES: return "Resultado anterior (RES)";
        case ASTNodeType::NUMERO_LITERAL: return "Literal numerico";
        case ASTNodeType::BOOL_LITERAL: return "Literal logico";
        case ASTNodeType::COMANDO_IFELSE: return "Decisao (IFELSE)";
        case ASTNodeType::COMANDO_WHILE: return "Repeticao (WHILE)";
        default: return "Desconhecido";
        }
    }

    // Imprime um no e seus filhos como uma arvore indentada em Markdown
    void escreverNoAtribuido(std::ostream &out, ASTNode *no, int nivel)
    {
        if (!no)
            return;

        std::string indent(nivel * 2, ' ');
        out << indent << "- **" << categoriaSemantica(no->tipo) << "**";

        if (!no->operando.empty())
            out << " `" << no->operando << "`";

        out << " : _" << nomeTipoDado(no->tipoDado) << "_";

        if (no->linha > 0)
            out << " (linha " << no->linha << ")";

        out << "\n";

        for (ASTNode *filho : no->filhos)
            escreverNoAtribuido(out, filho, nivel + 1);
    }
}

void exportarArvoreAtribuida(ASTNode *raiz, const std::string &arquivo)
{
    std::ofstream out(arquivo);
    if (!out.is_open())
        return;

    out << "# Arvore Sintatica Atribuida\n\n";
    out << "Arvore anotada com a categoria semantica, o operando/valor e o tipo "
           "inferido de cada no (resultado de `verificarTipos`).\n\n";

    if (!raiz)
    {
        out << "_(arvore vazia)_\n";
        out.close();
        return;
    }

    escreverNoAtribuido(out, raiz, 0);
    out.close();
}

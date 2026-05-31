// Integrantes do grupo (ordem alfabetica):
// Lucas Franco de Mello - luk4w
// Nome do grupo no Canvas: RA3 2

#include "gramatica.hpp"

#include <iostream>

using namespace std;

// prot de funcao
void printConjunto(std::string, std::map<std::string, std::set<std::string>>);

// Representação da gramatica
std::map<string, std::vector<std::vector<std::string>>> gramatica;

// Armazenamento real dos conjuntos
std::map<std::string, std::set<string>> first_sets;
std::map<std::string, std::set<string>> follow_sets;
map<string, map<string, vector<string>>> tabela_ll1;

// helper
bool isTerminal(const string &simbolo)
{
    return (simbolo == "PARENTESE_ESQ" || simbolo == "PARENTESE_DIR" ||
            simbolo == "START" || simbolo == "END" || simbolo == "NUMERO" ||
            simbolo == "IDENTIFICADOR" || simbolo == "OPERADOR" ||
            simbolo == "OPERADOR_RELACIONAL" || simbolo == "WHILE" ||
            simbolo == "IFELSE" || simbolo == "RES" ||
            simbolo == "TRUE" || simbolo == "FALSE");
}

// Calcula os conjuntos FIRST algoritmicamente
std::map<std::string, std::set<std::string>> calcularFirst()
{
    bool mudou = true;

    // Fica rodando até os conjuntos pararem de crescer
    while (mudou)
    {
        mudou = false;

        for (const auto &[nao_terminal, producoes] : gramatica)
        {
            for (const auto &producao : producoes)
            {
                if (producao.empty())
                    continue; // Se for Vazio (Lambda)

                string primeiro_simbolo = producao[0];
                size_t tamanho_antes = first_sets[nao_terminal].size();

                if (isTerminal(primeiro_simbolo))
                {
                    // Regra 1: Se começa com terminal, o terminal entra no FIRST
                    first_sets[nao_terminal].insert(primeiro_simbolo);
                }
                else
                {
                    // Regra 2: Se começa com Variável, herda o FIRST dela
                    for (const string &f : first_sets[primeiro_simbolo])
                    {
                        first_sets[nao_terminal].insert(f);
                    }
                }

                // Se o conjunto cresceu, precisamos rodar o while mais uma vez
                if (first_sets[nao_terminal].size() > tamanho_antes)
                {
                    mudou = true;
                }
            }
        }
    }
    return first_sets;
}

// Calcula os conjuntos FOLLOW algoritmicamente
std::map<std::string, std::set<std::string>> calcularFollow()
{
    // Regra 1 do FOLLOW: O simbolo inicial sempre recebe "$" (EOF)
    follow_sets["programa"].insert("$");

    bool mudou = true;
    while (mudou)
    {
        mudou = false;

        for (const auto &[A, producoes] : gramatica)
        {
            for (const auto &producao : producoes)
            {

                // Varre a regra da esquerda para a direita
                for (size_t i = 0; i < producao.size(); ++i)
                {
                    string B = producao[i];
                    if (isTerminal(B))
                        continue; // FOLLOW só existe para Não-Terminal

                    size_t tamanho_antes = follow_sets[B].size();

                    // Se B não é o ultimo simbolo da regra
                    if (i + 1 < producao.size())
                    {
                        string beta = producao[i + 1];

                        if (isTerminal(beta))
                        {
                            follow_sets[B].insert(beta);
                        }
                        else
                        {
                            // Pega o FIRST de quem vem depois
                            for (const string &f : first_sets[beta])
                            {
                                follow_sets[B].insert(f);
                            }
                        }
                    }
                    // Se B é o ultimo simbolo (Ex: A -> alfa B)
                    if (i + 1 == producao.size())
                    {
                        // Herda o FOLLOW do pai (A)
                        for (const string &f : follow_sets[A])
                        {
                            follow_sets[B].insert(f);
                        }
                    }

                    if (follow_sets[B].size() > tamanho_antes)
                    {
                        mudou = true;
                    }
                }
            }
        }
    }
    return follow_sets;
}

// Constroi a tabela LL(1) dinamicamente
void construirTabelaLL1()
{
    // Varre todas as regras de todos os não-terminais
    for (const auto &[nao_terminal, producoes] : gramatica)
    {
        for (const auto &producao : producoes)
        {
            if (producao.empty())
                continue;

            string primeiro_simbolo = producao[0];

            // TRATAMENTO DE NULABILIDADE (PRODUÇÃO VAZIA)
            if (primeiro_simbolo == "EPSILON")
            {
                // Se A -> EPSILON, para todo terminal t em FOLLOW(A), adicione a produção em M[A, t]
                for (const string &terminal : follow_sets[nao_terminal])
                {
                    tabela_ll1[nao_terminal][terminal] = producao;
                }
                continue; // Vai para a próxima regra
            }

            set<string> terminais_alvo;

            // Se o primeiro símbolo for terminal, o alvo da tabela é ele mesmo
            if (isTerminal(primeiro_simbolo))
            {
                terminais_alvo.insert(primeiro_simbolo);
            }
            // Se for Não-Terminal, os alvos são todos os terminais no FIRST dele
            else
            {
                terminais_alvo = first_sets[primeiro_simbolo];
            }

            // Preenche as células da Tabela LL(1) com a regra
            for (const string &terminal : terminais_alvo)
            {
                // M[Não-Terminal, Terminal] = Produção
                tabela_ll1[nao_terminal][terminal] = producao;
            }
        }
    }
}

// Construir a grmatica
void construirGramatica()
{
    gramatica["programa"] = {{"PARENTESE_ESQ", "START", "PARENTESE_DIR", "sequencia_execucao"}};

    gramatica["sequencia_execucao"] = {{"PARENTESE_ESQ", "avaliacao_sequencia"}};

    gramatica["avaliacao_sequencia"] = {
        {"END", "PARENTESE_DIR"},
        {"corpo_expressao", "PARENTESE_DIR", "sequencia_execucao"}};

    gramatica["expressao_aninhada"] = {{"PARENTESE_ESQ", "corpo_expressao", "PARENTESE_DIR"}};

    gramatica["operando"] = {{"NUMERO"}, {"TRUE"}, {"FALSE"}, {"IDENTIFICADOR"}, {"RES"}, {"expressao_aninhada"}};

    gramatica["corpo_expressao"] = {
        {"operando", "complemento_expressao"}};

    gramatica["complemento_expressao"] = {
        {"operando", "resto_complemento"},
        {"EPSILON"}};

    gramatica["resto_complemento"] = {
        {"operacao"},
        {"EPSILON"}};

    gramatica["operacao"] = {{"OPERADOR"}, {"OPERADOR_RELACIONAL"}, {"WHILE"}, {"operando", "IFELSE"}};

    auto first = calcularFirst();
    auto follow = calcularFollow();

    printConjunto("FIRST", first);
    printConjunto("FOLLOW", follow);

    construirTabelaLL1();
}

void printConjunto(string nome, map<string, set<string>> cc)
{
    // Imprimir os conjuntos FIRST
    cout << "--- CONJUNTO " << nome << " ---\n";
    for (const auto &[nao_terminal, conjunto] : cc)
    {
        cout << nome << "(" << nao_terminal << ") = { ";
        bool primeiro_item = true;
        for (const string &terminal : conjunto)
        {
            if (!primeiro_item)
                cout << ", ";
            cout << terminal;
            primeiro_item = false;
        }
        cout << " }\n";
    }
    cout << "\n";
}
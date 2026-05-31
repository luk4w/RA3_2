// Integrantes do grupo (ordem alfabetica):
// Lucas Franco de Mello - luk4w
// Nome do grupo no Canvas: RA3 2

#pragma once
#include "ast.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// Converte o enum TipoDado para string para legibilidade no JSON
inline std::string tipoDadoToStringJson(TipoDado t) {
    switch (t) {
        case TipoDado::INT: return "INT";
        case TipoDado::REAL: return "REAL";
        case TipoDado::BOOL: return "BOOL";
        default: return "DESCONHECIDO";
    }
}

// Converte o enum ASTNodeType para string para legibilidade no JSON
inline std::string nodeTypeToString(ASTNodeType tipo) {
    switch (tipo) {
        case ASTNodeType::PROGRAMA: return "PROGRAMA";
        case ASTNodeType::SEQUENCIA: return "SEQUENCIA";
        case ASTNodeType::INSTRUCAO_VFP: return "INSTRUCAO_VFP";
        case ASTNodeType::MEMORIA_LOAD: return "MEMORIA_LOAD";
        case ASTNodeType::MEMORIA_STORE: return "MEMORIA_STORE";
        case ASTNodeType::MEMORIA_RES: return "MEMORIA_RES";
        case ASTNodeType::NUMERO_LITERAL: return "NUMERO_LITERAL";
        case ASTNodeType::COMANDO_WHILE: return "COMANDO_WHILE";
        case ASTNodeType::COMANDO_IFELSE: return "COMANDO_IFELSE";
        case ASTNodeType::INSTRUCAO_CMP: return "INSTRUCAO_CMP";
        default: return "UNKNOWN";
    }
}

// Gera a estrutura JSON recursivamente
inline std::string buildJson(ASTNode* node, int indent = 0) {
    if (!node) return "null";

    std::string space(indent, ' ');
    std::string json = "{\n";
    json += space + "  \"tipo\": \"" + nodeTypeToString(node->tipo) + "\",\n";
    json += space + "  \"tipoDado\": \"" + tipoDadoToStringJson(node->tipoDado) + "\",\n";

    if (!node->operando.empty())
        json += space + "  \"operando\": \"" + node->operando + "\",\n";
    
    if (!node->opcode.empty())
        json += space + "  \"opcode\": \"" + node->opcode + "\",\n";

    json += space + "  \"filhos\": [";
    
    if (!node->filhos.empty()) {
        json += "\n";
        for (size_t i = 0; i < node->filhos.size(); ++i) {
            json += space + "    " + buildJson(node->filhos[i], indent + 4);
            if (i < node->filhos.size() - 1) json += ",";
            json += "\n";
        }
        json += space + "  ";
    }
    json += "]\n" + space + "}";
    return json;
}

// Salva a arveore em arquivo e imprime no console para validação
inline void exportarAST(ASTNode* raiz, const std::string& filename) {
    std::string content = buildJson(raiz);
    
    std::ofstream out(filename);
    if (out.is_open()) {
        out << content;
        out.close();
        std::cout << "AST exportada com sucesso para: " << filename << "\n";
    } else {
        std::cerr << "Erro ao criar arquivo de exportacao da AST.\n";
    }
}
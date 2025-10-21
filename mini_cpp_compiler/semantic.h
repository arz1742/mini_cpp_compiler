#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <iostream>
#include <map>
#include <string>
#include "parser.h"
using namespace std;

// Represents an entry in the symbol table
struct Symbol {
    string name;
    string type;
};

// Semantic Analyzer class: performs symbol checks, type checks, etc.
class SemanticAnalyzer {
private:
    map<string, Symbol> symbolTable;  // variable name → symbol info
    bool hasError = false;            // flag for semantic errors

    // Internal helper functions
    string inferType(const shared_ptr<ASTNode> &node);
    void analyzeNode(const shared_ptr<ASTNode> &node);

public:
    // Run semantic analysis on the AST
    void analyze(const shared_ptr<ASTNode> &root);

    // Query: did we encounter semantic errors?
    bool hasErrors() const { return hasError; }

    // Optional: print symbol table (helpful for debugging/report)
    void printSymbolTable() const {
        cout << "\nSymbol Table:\n";
        for (const auto &entry : symbolTable) {
            cout << "  " << entry.first << " : " << entry.second.type << "\n";
        }
    }

    // Allow semantic.cpp to set error flag when a semantic error occurs
    void setErrorFlag() { hasError = true; }
};

#endif

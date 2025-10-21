#include "semantic.h"
using namespace std;

string SemanticAnalyzer::inferType(const shared_ptr<ASTNode> &node) {
    if (!node) return "unknown";

    if (node->kind == "Number") {
        return (node->value.find('.') != string::npos) ? "float" : "int";
    }
    else if (node->kind == "Var") {
        if (symbolTable.find(node->value) == symbolTable.end()) {
            cout << "Semantic Error: Variable '" << node->value
                 << "' used before declaration.\n";
            setErrorFlag();
            return "unknown";
        }
        return symbolTable[node->value].type;
    }
    else if (node->kind == "BinaryOp" || node->kind == "RelOp") {
        string leftType = inferType(node->children[0]);
        string rightType = inferType(node->children[1]);
        if (leftType == "unknown" || rightType == "unknown")
            return "unknown";
        return (leftType == "float" || rightType == "float") ? "float" : "int";
    }
    else if (node->kind == "Assign") {
        string varName = node->children[0]->value;

        // Check undeclared variable
        if (symbolTable.find(varName) == symbolTable.end()) {
            cout << "Semantic Error: Variable '" << varName
                 << "' used before declaration.\n";
            setErrorFlag();
            inferType(node->children[1]); // still check RHS
            return "unknown";
        }

        // Check assignment type
        string varType = symbolTable[varName].type;
        string exprType = inferType(node->children[1]);
        if (exprType != "unknown" && varType != exprType) {
            cout << "Type Mismatch: Cannot assign " << exprType
                 << " to variable '" << varName << "' of type "
                 << varType << ".\n";
            setErrorFlag();
        }
        return varType;
    }
    else {
        for (auto &c : node->children)
            inferType(c);
        return "unknown";
    }
}

void SemanticAnalyzer::analyzeNode(const shared_ptr<ASTNode> &node) {
    if (!node) return;

    // Handle declarations
    if (node->kind == "Decl") {
        string varName = node->value;
        string varType = node->children[0]->value;

        if (symbolTable.count(varName)) {
            cout << "Semantic Error: Variable '" << varName << "' redeclared.\n";
            setErrorFlag();
        } else {
            symbolTable[varName] = {varName, varType};
        }

        // If initialized, check initializer expression
        if (node->children.size() > 1)
            inferType(node->children[1]);
    }

    // Recursively analyze children
    for (auto &c : node->children)
        analyzeNode(c);

    // Perform type inference for key nodes
    if (node->kind == "If" || node->kind == "Return" || node->kind == "Block")
        inferType(node);
}

void SemanticAnalyzer::analyze(const shared_ptr<ASTNode> &root) {
    cout << "\n===== SEMANTIC ANALYSIS =====\n";
    analyzeNode(root);

    cout << "\nSymbol Table:\n";
    for (auto &entry : symbolTable)
        cout << "  " << entry.second.name << " : " << entry.second.type << "\n";

    if (!hasErrors())
        cout << "No semantic errors found.\n";
    else
        cout << "Semantic errors detected. See messages above.\n";
}

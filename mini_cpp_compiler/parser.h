#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "token.h"
using namespace std;

// --------------------- AST Node ---------------------
struct ASTNode {
    string kind;                                // e.g., "BinaryOp", "Var", "Number", etc.
    string value;                               // e.g., "+", variable name, literal
    vector<shared_ptr<ASTNode>> children;

    // ✅ Constructors
    ASTNode() {}
    ASTNode(const string &k) : kind(k) {}
    ASTNode(const string &k, const string &v) : kind(k), value(v) {}
};

// --------------------- Parser Class ---------------------
class Parser {
private:
    vector<Token> tokens;
    size_t pos = 0;
    bool hasError = false;

    // Utility methods
    Token peek();
    Token advance();
    bool match(const string &type, const string &value = "");
    void expect(const string &type, const string &value = "");
    bool isAtEnd();
    void error(const string &msg);
    void setErrorFlag() { hasError = true; }

    // Grammar rules
    shared_ptr<ASTNode> parseProgram();
    shared_ptr<ASTNode> parseFunction();
    shared_ptr<ASTNode> parseCompoundStmt();
    shared_ptr<ASTNode> parseStmt();
    shared_ptr<ASTNode> parseDecl();
    shared_ptr<ASTNode> parseAssign();
    shared_ptr<ASTNode> parseIf();
    shared_ptr<ASTNode> parseReturn();
    shared_ptr<ASTNode> parseExpr();
    shared_ptr<ASTNode> parseAddExpr();
    shared_ptr<ASTNode> parseTerm();
    shared_ptr<ASTNode> parseFactor();

public:
    Parser(const vector<Token> &toks);

    shared_ptr<ASTNode> parse();
    void printAST(const shared_ptr<ASTNode> &node, int indent = 0);

    // ✅ Expose error flag for main.cpp
    bool hasErrors() const { return hasError; }
};

#endif

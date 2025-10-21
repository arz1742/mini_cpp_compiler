#include "parser.h"
using namespace std;

// --------------------- Parser constructor ---------------------
Parser::Parser(const vector<Token> &toks) : tokens(toks), pos(0) {}

// --------------------- Utility methods ---------------------
Token Parser::peek() {
    if (pos < tokens.size()) return tokens[pos];
    return {"EOF", ""};
}

Token Parser::advance() {
    if (pos < tokens.size()) return tokens[pos++];
    return {"EOF", ""};
}

bool Parser::match(const string &type, const string &value) {
    if (pos < tokens.size() && tokens[pos].type == type &&
        (value.empty() || tokens[pos].value == value)) {
        pos++;
        return true;
    }
    return false;
}

bool Parser::isAtEnd() {
    return pos >= tokens.size();
}

void Parser::error(const string &msg) {
    cout << "Syntax Error: " << msg << "\n";
    setErrorFlag();
}

// Expect a specific token or flag an error
void Parser::expect(const string &type, const string &value) {
    if (!match(type, value)) {
        string got = isAtEnd() ? "end-of-file" : "'" + tokens[pos].value + "'";
        error("expected '" + (value.empty() ? type : value) + "' but got " + got);
    }
}

// --------------------- Grammar Implementation ---------------------
shared_ptr<ASTNode> Parser::parse() {
    auto program = parseProgram();
    return program;
}

shared_ptr<ASTNode> Parser::parseProgram() {
    auto func = parseFunction();
    auto root = make_shared<ASTNode>();
    root->kind = "Program";
    root->children.push_back(func);
    return root;
}

shared_ptr<ASTNode> Parser::parseFunction() {
    if (!match("KEYWORD", "int")) {
        error("expected 'int' at function start");
        return nullptr;
    }
    if (!match("KEYWORD", "main")) {
        error("expected 'main' after 'int'");
        return nullptr;
    }
    expect("SYMBOL", "(");
    expect("SYMBOL", ")");
    auto body = parseCompoundStmt();

    auto fn = make_shared<ASTNode>();
    fn->kind = "Function";
    fn->value = "main";
    fn->children.push_back(body);
    return fn;
}

shared_ptr<ASTNode> Parser::parseCompoundStmt() {
    expect("SYMBOL", "{");
    auto node = make_shared<ASTNode>();
    node->kind = "Block";

    while (!isAtEnd() && !(peek().type == "SYMBOL" && peek().value == "}")) {
        node->children.push_back(parseStmt());
        if (hasError) break; // stop parsing if already errored
    }

    if (!isAtEnd())
        expect("SYMBOL", "}");
    else
        error("unexpected end-of-file inside block");

    return node;
}

shared_ptr<ASTNode> Parser::parseStmt() {
    if (peek().type == "KEYWORD" && (peek().value == "int" || peek().value == "float"))
        return parseDecl();
    else if (peek().type == "KEYWORD" && peek().value == "if")
        return parseIf();
    else if (peek().type == "KEYWORD" && peek().value == "return")
        return parseReturn();
    else if (peek().type == "IDENTIFIER")
        return parseAssign();
    else if (peek().type == "SYMBOL" && peek().value == "{")
        return parseCompoundStmt();

    error("unexpected token '" + peek().value + "' in statement");
    advance();
    return nullptr;
}

shared_ptr<ASTNode> Parser::parseDecl() {
    string typ = advance().value;
    if (peek().type != "IDENTIFIER") {
        error("expected identifier after type declaration");
        return nullptr;
    }

    string id = advance().value;
    auto node = make_shared<ASTNode>("Decl", id);
    node->children.push_back(make_shared<ASTNode>("Type", typ));

    if (peek().type == "OPERATOR" && peek().value == "=") {
        advance();
        node->children.push_back(parseExpr());
    }

    expect("SYMBOL", ";");
    return node;
}

shared_ptr<ASTNode> Parser::parseAssign() {
    string id = advance().value;
    if (peek().type != "OPERATOR") {
        error("expected operator after identifier in assignment");
        return nullptr;
    }

    string op = advance().value;
    auto node = make_shared<ASTNode>("Assign", op);
    node->children.push_back(make_shared<ASTNode>("Var", id));
    node->children.push_back(parseExpr());
    expect("SYMBOL", ";");
    return node;
}

shared_ptr<ASTNode> Parser::parseIf() {
    expect("KEYWORD", "if");
    expect("SYMBOL", "(");
    auto cond = parseExpr();
    expect("SYMBOL", ")");

    shared_ptr<ASTNode> thenNode;
    if (peek().type == "SYMBOL" && peek().value == "{")
        thenNode = parseCompoundStmt();
    else
        thenNode = parseStmt();

    auto node = make_shared<ASTNode>("If");
    node->children.push_back(cond);
    node->children.push_back(thenNode);

    if (peek().type == "KEYWORD" && peek().value == "else") {
        advance();
        if (peek().type == "SYMBOL" && peek().value == "{")
            node->children.push_back(parseCompoundStmt());
        else
            node->children.push_back(parseStmt());
    }

    return node;
}

shared_ptr<ASTNode> Parser::parseReturn() {
    expect("KEYWORD", "return");
    auto expr = parseExpr();
    expect("SYMBOL", ";");

    auto node = make_shared<ASTNode>("Return");
    node->children.push_back(expr);
    return node;
}

// --------------------- Expressions ---------------------
shared_ptr<ASTNode> Parser::parseExpr() {
    auto left = parseAddExpr();
    while (peek().type == "OPERATOR" &&
           (peek().value == "<" || peek().value == "<=" || peek().value == ">" ||
            peek().value == ">=" || peek().value == "==" || peek().value == "!=")) {
        string op = advance().value;
        auto newNode = make_shared<ASTNode>("RelOp", op);
        newNode->children.push_back(left);
        newNode->children.push_back(parseAddExpr());
        left = newNode;
    }
    return left;
}

shared_ptr<ASTNode> Parser::parseAddExpr() {
    auto left = parseTerm();
    while (peek().type == "OPERATOR" && (peek().value == "+" || peek().value == "-")) {
        string op = advance().value;
        auto newNode = make_shared<ASTNode>("BinaryOp", op);
        newNode->children.push_back(left);
        newNode->children.push_back(parseTerm());
        left = newNode;
    }
    return left;
}

shared_ptr<ASTNode> Parser::parseTerm() {
    auto left = parseFactor();
    while (peek().type == "OPERATOR" && (peek().value == "*" || peek().value == "/")) {
        string op = advance().value;
        auto newNode = make_shared<ASTNode>("BinaryOp", op);
        newNode->children.push_back(left);
        newNode->children.push_back(parseFactor());
        left = newNode;
    }
    return left;
}

shared_ptr<ASTNode> Parser::parseFactor() {
    if (peek().type == "IDENTIFIER") {
        string id = advance().value;
        return make_shared<ASTNode>("Var", id);
    } else if (peek().type == "NUMBER") {
        string num = advance().value;
        return make_shared<ASTNode>("Number", num);
    } else if (peek().type == "SYMBOL" && peek().value == "(") {
        advance();
        auto node = parseExpr();
        expect("SYMBOL", ")");
        return node;
    }

    error("unexpected token in expression: '" + peek().value + "'");
    advance();
    return nullptr;
}

// --------------------- AST Printing ---------------------
void Parser::printAST(const shared_ptr<ASTNode> &node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; ++i) cout << "  ";
    if (node->value.empty())
        cout << node->kind << "\n";
    else
        cout << node->kind << " : " << node->value << "\n";
    for (auto &c : node->children)
        printAST(c, indent + 1);
}

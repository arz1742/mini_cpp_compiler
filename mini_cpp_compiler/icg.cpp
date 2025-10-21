#include "icg.h"
#include <sstream>

using namespace std;

ICGGenerator::ICGGenerator() {
    tempCount = 0;
    labelCount = 0;
    code.clear();
}

void ICGGenerator::reset() {
    tempCount = 0;
    labelCount = 0;
    code.clear();
}

string ICGGenerator::newTemp() {
    return "t" + to_string(++tempCount);
}

string ICGGenerator::newLabel() {
    return "L" + to_string(++labelCount);
}

string ICGGenerator::genExpr(const shared_ptr<ASTNode> &node) {
    if (!node) return "";

    // Number literal or variable
    if (node->kind == "Number" || node->kind == "Var") {
        return node->value;
    }

    // Binary arithmetic or relational operations
    if (node->kind == "BinaryOp" || node->kind == "RelOp") {
        string left = genExpr(node->children[0]);
        string right = genExpr(node->children[1]);
        string tmp = newTemp();
        code.push_back(tmp + " = " + left + " " + node->value + " " + right);
        return tmp;
    }

    // Fallback: if node has children, recurse on first child
    if (!node->children.empty()) {
        return genExpr(node->children[0]);
    }

    return "";
}

void ICGGenerator::genStmt(const shared_ptr<ASTNode> &node) {
    if (!node) return;

    if (node->kind == "Program") {
        if (!node->children.empty()) genStmt(node->children[0]);
    }
    else if (node->kind == "Function") {
        if (!node->children.empty()) genStmt(node->children[0]);
    }
    else if (node->kind == "Block") {
        for (auto &c : node->children) genStmt(c);
    }
    else if (node->kind == "Decl") {
        // declaration with optional initializer
        if (node->children.size() > 1) {
            string rhs = genExpr(node->children[1]);
            string lhs = node->value; // variable name
            code.push_back(lhs + " = " + rhs);
        }
    }
    else if (node->kind == "Assign") {
        string op = node->value;
        string lhs = node->children[0]->value;

        if (op == "=") {
            string rhs = genExpr(node->children[1]);
            code.push_back(lhs + " = " + rhs);
        }
        else {
            // compound assignment e.g. +=
            string baseOp;
            if (op == "+=") baseOp = "+";
            else if (op == "-=") baseOp = "-";
            else if (op == "*=") baseOp = "*";
            else if (op == "/=") baseOp = "/";
            else baseOp = "";

            if (!baseOp.empty()) {
                string right = genExpr(node->children[1]);
                string tmp = newTemp();
                code.push_back(tmp + " = " + lhs + " " + baseOp + " " + right);
                code.push_back(lhs + " = " + tmp);
            } else {
                string rhs = genExpr(node->children[1]);
                code.push_back(lhs + " = " + rhs);
            }
        }
    }
    else if (node->kind == "If") {
        // children: [cond, thenNode, (elseNode)?]
        string condTmp = genExpr(node->children[0]);
        string Ltrue = newLabel();
        string Lfalse = newLabel();
        string Lend = newLabel();

        code.push_back("if " + condTmp + " goto " + Ltrue);
        code.push_back("goto " + Lfalse);

        code.push_back(Ltrue + ":");
        genStmt(node->children[1]);
        code.push_back("goto " + Lend);

        code.push_back(Lfalse + ":");
        if (node->children.size() > 2) {
            genStmt(node->children[2]); // else block
        }
        code.push_back(Lend + ":");
    }
    else if (node->kind == "Return") {
        string val = genExpr(node->children[0]);
        code.push_back("return " + val);
    }
    else {
        // recurse by default
        for (auto &c : node->children) genStmt(c);
    }
}

void ICGGenerator::generate(const shared_ptr<ASTNode> &root) {
    // reset internal state and generate
    reset();
    genStmt(root);
}

const vector<string>& ICGGenerator::getCode() const {
    return code;
}

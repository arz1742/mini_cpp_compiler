#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <vector>
#include <string>
#include "token.h"
using namespace std;

class Lexer {
private:
    vector<Token> tokens;
    string code;
    size_t pos = 0;
    bool hasError = false;

    void skipWhitespace();
    void skipComments();
    string readIdentifier();
    string readNumber();
    string readOperator();
    string readString();

public:
    Lexer(const string &input);
    vector<Token> tokenize();
    void printTokens();

    bool hasErrors() const { return hasError; }
    void setErrorFlag() { hasError = true; }
};

#endif

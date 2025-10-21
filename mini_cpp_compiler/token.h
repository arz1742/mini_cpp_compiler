#ifndef TOKEN_H
#define TOKEN_H

#include <string>

// Simple token struct used by lexer & parser
struct Token {
    std::string type;   // e.g. "KEYWORD", "IDENTIFIER", "NUMBER", "OPERATOR", "SYMBOL"
    std::string value;  // actual lexeme
};

#endif // TOKEN_H

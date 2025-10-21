#include "lexer.h"
#include <unordered_set>
#include <iostream>
#include <cctype>

using namespace std;

// -----------------------------
// Keyword and Symbol Definitions
// -----------------------------
static unordered_set<string> keywords = {
    "int", "float", "if", "else", "while", "return", "for", "main"
};

static unordered_set<char> symbols = {
    ';', ',', '{', '}', '(', ')'
};

// -----------------------------
// Constructor
// -----------------------------
Lexer::Lexer(const string &input) : code(input), pos(0) {}

// -----------------------------
// Utility Functions
// -----------------------------
void Lexer::skipWhitespace() {
    while (pos < code.size() && isspace((unsigned char)code[pos])) pos++;
}

void Lexer::skipComments() {
    if (code[pos] == '/' && pos + 1 < code.size()) {
        if (code[pos + 1] == '/') {
            pos += 2;
            while (pos < code.size() && code[pos] != '\n') pos++;
        } else if (code[pos + 1] == '*') {
            pos += 2;
            while (pos + 1 < code.size() &&
                   !(code[pos] == '*' && code[pos + 1] == '/')) pos++;
            if (pos + 1 < code.size()) pos += 2;
        }
    }
}

string Lexer::readIdentifier() {
    string result;
    while (pos < code.size() &&
           (isalnum((unsigned char)code[pos]) || code[pos] == '_'))
        result += code[pos++];
    return result;
}

string Lexer::readNumber() {
    string result;
    while (pos < code.size() &&
           (isdigit((unsigned char)code[pos]) || code[pos] == '.'))
        result += code[pos++];
    return result;
}

string Lexer::readOperator() {
    string op;
    op += code[pos++];
    if (pos < code.size()) {
        char next = code[pos];
        string two = op + next;
        if (two == "==" || two == "!=" ||
            two == "<=" || two == ">=" ||
            two == "+=" || two == "-=" ||
            two == "*=" || two == "/=") {
            op = two;
            pos++;
        }
    }
    return op;
}

string Lexer::readString() {
    string result;
    pos++; // Skip initial quote
    while (pos < code.size() && code[pos] != '"') {
        if (code[pos] == '\\' && pos + 1 < code.size()) {
            result += code[pos++]; // Keep escape
        }
        result += code[pos++];
    }
    if (pos < code.size() && code[pos] == '"') pos++;
    return result;
}

// -----------------------------
// Tokenization (Core Function)
// -----------------------------
vector<Token> Lexer::tokenize() {
    tokens.clear();
    pos = 0;

    while (pos < code.size()) {
        skipWhitespace();
        if (pos >= code.size()) break;

        // Skip comments first
        if (code[pos] == '/' && pos + 1 < code.size() &&
            (code[pos + 1] == '/' || code[pos + 1] == '*')) {
            skipComments();
            continue;
        }

        char current = code[pos];

        if (isalpha((unsigned char)current) || current == '_') {
            string word = readIdentifier();
            if (keywords.count(word))
                tokens.push_back({"KEYWORD", word});
            else
                tokens.push_back({"IDENTIFIER", word});
        }
        else if (isdigit((unsigned char)current)) {
            tokens.push_back({"NUMBER", readNumber()});
        }
        else if (current == '"') {
            tokens.push_back({"STRING", readString()});
        }
        else if (string("+-*/=<>!").find(current) != string::npos) {
            tokens.push_back({"OPERATOR", readOperator()});
        }
        else if (symbols.count(current)) {
            tokens.push_back({"SYMBOL", string(1, current)});
            pos++;
        }
        else {
            // -----------------------------
            // UNKNOWN / INVALID CHARACTER
            // -----------------------------
            cout << "Lexical Error: Unknown symbol '" << current
                 << "' at position " << pos << "\n";
            tokens.push_back({"UNKNOWN", string(1, current)});
            setErrorFlag(); // mark lexical error
            pos++;
        }
    }

    return tokens;
}

// -----------------------------
// Token Printout
// -----------------------------
void Lexer::printTokens() {
    cout << "===== LEXICAL ANALYSIS =====\n";
    cout << "Type\t\tValue\n";
    cout << "----------------------------\n";
    for (auto &t : tokens) {
        cout << t.type << "\t\t" << t.value << "\n";
    }
}

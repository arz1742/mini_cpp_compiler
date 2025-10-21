#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "icg.h"
#include "opt.h"
#include "codegen.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: mini_compiler <source_file>\n";
        return 1;
    }

    string filename = argv[1];

    cout << "=============================================\n";
    cout << "        Mini C++ Compiler - Phase 1 to 6\n";
    cout << "=============================================\n";
    cout << "Compiling file: " << filename << "\n\n";

    // ===== PHASE 1: Lexical Analysis =====
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << "\n";
        return 1;
    }

    stringstream buffer;
    buffer << file.rdbuf();
    string sourceCode = buffer.str();

    Lexer lexer(sourceCode);
    vector<Token> tokens = lexer.tokenize();

    lexer.printTokens();

    // Stop if lexical errors occurred
    if (lexer.hasErrors()) {
        cout << "\nCompilation stopped due to lexical errors.\n";
        return 0;
    }

    // ===== PHASE 2: Syntax Analysis (Parsing & AST) =====
    cout << "\n===== SYNTAX ANALYSIS (AST) =====\n";
    Parser parser(tokens);
    auto ast = parser.parse();
    parser.printAST(ast);

    // Stop if syntax errors occurred
    if (parser.hasErrors()) {
        cout << "\nCompilation stopped due to syntax errors.\n";
        return 0;
    }

    // ===== PHASE 3: Semantic Analysis =====
    SemanticAnalyzer semantic;
    semantic.analyze(ast);

    if (semantic.hasErrors()) {
        cout << "\nCompilation stopped due to semantic errors.\n";
        return 0;
    }

    // ===== PHASE 4: Intermediate Code Generation (ICG) =====
    ICGGenerator icg;
    icg.generate(ast);

    vector<string> tac = icg.getCode();

    cout << "\n===== INTERMEDIATE CODE (Before Optimization) =====\n";
    for (auto &line : tac)
        cout << line << "\n";

    // ===== PHASE 5: Optimization =====
    optimizeTAC(tac);

    cout << "\n===== INTERMEDIATE CODE (After Optimization) =====\n";
    for (auto &line : tac)
        cout << line << "\n";

    // ===== PHASE 6: Target Code Generation =====
    cout << "\n===== TARGET PSEUDO-ASSEMBLY =====\n";
    CodeGen cg;
    vector<string> asmCode = cg.generate(tac);

    for (auto &line : asmCode)
        cout << line << "\n";

    cout << "\nCompilation stages completed: "
         << "Lexical + Syntax + Semantic + ICG + OPT + CODEGEN\n";

    return 0;
}

#ifndef CODEGEN_H
#define CODEGEN_H

#include <string>
#include <vector>

class CodeGen {
public:
    // Generate pseudo-assembly from TAC lines
    // Input: tac (vector of TAC strings like "t1 = a + b", "if t1 goto L1", "L1:", "return a")
    // Output: assembly-like lines (MOV/ADD/SUB/MUL/DIV/CMP/JNE/JMP/LABEL/RETURN)
    std::vector<std::string> generate(const std::vector<std::string> &tac);
};

#endif // CODEGEN_H

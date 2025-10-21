#ifndef ICG_H
#define ICG_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "parser.h"

class ICGGenerator {
private:
    int tempCount = 0;
    int labelCount = 0;
    std::vector<std::string> code;

    std::string newTemp();
    std::string newLabel();
    std::string genExpr(const std::shared_ptr<ASTNode> &node);
    void genStmt(const std::shared_ptr<ASTNode> &node);

public:
    ICGGenerator();
    // generate into internal vector (no printing)
    void generate(const std::shared_ptr<ASTNode> &root);

    // retrieve generated code (by reference) for optimization / printing
    const std::vector<std::string>& getCode() const;

    // convenience: clear/reset before generating
    void reset();
};

#endif // ICG_H

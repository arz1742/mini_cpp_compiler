#ifndef OPT_H
#define OPT_H

#include <string>
#include <vector>

// Performs optimization passes on the generated TAC
// - Multi-pass constant folding & propagation
// - Algebraic simplifications (x+0, x*1, x*0, x/1, etc.)
// - Temporary elimination & single-use inlining
void optimizeTAC(std::vector<std::string> &code);

#endif // OPT_H

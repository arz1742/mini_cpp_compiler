#include "codegen.h"
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

using namespace std;

// simple tokenizer (space separated)
static vector<string> splitTok(const string &line) {
    vector<string> out;
    istringstream iss(line);
    string t;
    while (iss >> t) out.push_back(t);
    return out;
}

// Check if token is a number (integer or float)
static bool isNumber(const string &s) {
    if (s.empty()) return false;
    bool hasDot = false;
    size_t i = 0;
    if (s[0] == '+' || s[0] == '-') i = 1;
    for (; i < s.size(); ++i) {
        if (s[i] == '.') {
            if (hasDot) return false;
            hasDot = true;
            continue;
        }
        if (!isdigit((unsigned char)s[i])) return false;
    }
    return true;
}

static string regForTempName(const string &t) {
    // convert t1 -> r1, t2 -> r2, etc.
    if (t.size() >= 2 && t[0] == 't') return string("r") + t.substr(1);
    // fall back
    return string("r_") + t;
}

vector<string> CodeGen::generate(const vector<string> &tac) {
    vector<string> out;
    if (tac.empty()) return out;

    // Map temps to registers; variables are memory symbols (we'll use their names directly)
    unordered_map<string,string> tempReg; // t1 -> r1
    unordered_map<string,bool> declaredVars; // variable names we saw (for possible prologue)
    int maxRegUsed = 0;

    // helper to get operand into a register or immediate string
    auto operandToRegOrImmediate = [&](const string &op)->string {
        if (isNumber(op)) return op;       // immediate
        if (!op.empty() && op[0]=='t') {  // temp -> register
            auto it = tempReg.find(op);
            if (it == tempReg.end()) {
                string r = regForTempName(op);
                tempReg[op] = r;
                // track max reg
            }
            return tempReg[op];
        }
        // variable -> treat as memory symbol; we'll load into a register when needed
        return op; // caller will detect variable string (not starting with r or numeric)
    };

    // emit prologue: collect variable names first (scan TAC)
    for (auto &line : tac) {
        auto toks = splitTok(line);
        if (toks.size() >= 3 && toks[1] == "=") {
            string lhs = toks[0];
            // if lhs is not temp, it's a variable to declare
            if (lhs.empty()) continue;
            if (lhs[0] != 't') declaredVars[lhs] = true;
            // also consider tokens on rhs that are not temps/numbers as vars
            for (size_t i=2;i<toks.size();++i) {
                string tk = toks[i];
                if (!tk.empty() && tk[0] != 't' && !isNumber(tk) && tk != "+" && tk!="-"
                    && tk!="*" && tk!="/" && tk!="<" && tk!=">" && tk!="<=" && tk!=">="
                    && tk!="==" && tk!="!=") {
                    declaredVars[tk] = true;
                }
            }
        } else if (toks.size() >= 1) {
            if (toks[0] == "return" && toks.size() >= 2) {
                string r = toks[1];
                if (!r.empty() && r[0] != 't' && !isNumber(r)) declaredVars[r] = true;
            }
            else if (toks[0].back() == ':') {
                // label
            }
            else if (toks[0] == "if" && toks.size() >= 2) {
                string cond = toks[1];
                if (!cond.empty() && cond[0] != 't' && !isNumber(cond)) declaredVars[cond] = true;
            }
        }
    }

    // Emit comment / prologue
    out.push_back("; --- generated pseudo-assembly ---");
    out.push_back("; Variables (memory):");
    for (auto &p : declaredVars) {
        out.push_back("; var: " + p.first);
    }
    out.push_back("");

    // Process each TAC line
    for (size_t i=0;i<tac.size();++i) {
        string line = tac[i];
        auto toks = splitTok(line);
        if (toks.empty()) continue;

        // Label (format: "L1:" or "L3:")
        if (toks.size() == 1 && toks[0].back() == ':') {
            out.push_back(toks[0]); // emit label as-is
            continue;
        }

        // Goto: "goto L2"
        if (toks.size() == 2 && toks[0] == "goto") {
            out.push_back("JMP " + toks[1]);
            continue;
        }

        // If: "if t1 goto L1" OR "if 1 goto L1"
        if (toks.size() == 4 && toks[0] == "if" && toks[2] == "goto") {
            string cond = toks[1];
            string label = toks[3];
            string condReg = operandToRegOrImmediate(cond);
            if (isNumber(condReg)) {
                // immediate boolean: compare to zero
                if (condReg == "0") {
                    // always false -> no jump (no-op)
                    // emit nothing
                } else {
                    out.push_back("JMP " + label); // always true
                }
            } else if (!condReg.empty() && condReg[0] == 'r') {
                out.push_back("CMP " + condReg + ", 0");
                out.push_back("JNE " + label);
            } else {
                // cond is a variable name: load it into a temp register first
                string reg = "r_load";
                out.push_back("MOV " + reg + ", " + condReg);
                out.push_back("CMP " + reg + ", 0");
                out.push_back("JNE " + label);
            }
            continue;
        }

        // Return: "return x"
        if (toks[0] == "return") {
            if (toks.size() >= 2) {
                string rv = toks[1];
                string rvReg = operandToRegOrImmediate(rv);
                if (isNumber(rvReg)) {
                    out.push_back("MOV ret, " + rvReg);
                } else if (!rvReg.empty() && rvReg[0] == 'r') {
                    out.push_back("MOV ret, " + rvReg);
                } else {
                    out.push_back("MOV ret, " + rvReg);
                }
            } else {
                out.push_back("MOV ret, 0");
            }
            out.push_back("RETURN");
            continue;
        }

        // Assignment patterns: lhs = rhs...
        if (toks.size() >= 3 && toks[1] == "=") {
            string lhs = toks[0];
            // RHS can be:
            //  - single operand: "5" or "t3" or "a"
            //  - binary op: "A op B" (three tokens)
            if (toks.size() == 3) {
                string rhs = toks[2];
                // simple assign
                if (isNumber(rhs)) {
                    // a = 5  -> MOV a, 5
                    if (lhs.size()>0 && lhs[0]=='t') {
                        // temp = imm -> map temp->reg then MOV reg, imm
                        string r = regForTempName(lhs);
                        tempReg[lhs] = r;
                        out.push_back("MOV " + r + ", " + rhs);
                    } else {
                        out.push_back("MOV " + lhs + ", " + rhs);
                    }
                } else if (!rhs.empty() && rhs[0] == 't') {
                    // a = t3  or t2 = t3
                    string rsrc;
                    if (tempReg.count(rhs)) rsrc = tempReg[rhs];
                    else { rsrc = regForTempName(rhs); tempReg[rhs] = rsrc; }
                    if (lhs.size()>0 && lhs[0]=='t') {
                        string rdest = regForTempName(lhs);
                        tempReg[lhs] = rdest;
                        out.push_back("MOV " + rdest + ", " + rsrc);
                    } else {
                        out.push_back("MOV " + lhs + ", " + rsrc);
                    }
                } else {
                    // rhs is a variable name
                    if (lhs.size()>0 && lhs[0]=='t') {
                        string rdest = regForTempName(lhs);
                        tempReg[lhs] = rdest;
                        out.push_back("MOV " + rdest + ", " + rhs);
                    } else {
                        out.push_back("MOV " + lhs + ", " + rhs);
                    }
                }
            }
            else if (toks.size() == 5) {
                // binary op: tok[2] op tok[3] tok[4]
                string A = toks[2], op = toks[3], B = toks[4];

                // Get A into register aReg (or immediate)
                string aRegOrImm = operandToRegOrImmediate(A);
                string bRegOrImm = operandToRegOrImmediate(B);

                // Ensure temp dest has a register
                if (lhs.size() > 0 && lhs[0] == 't') {
                    string rdest = regForTempName(lhs);
                    tempReg[lhs] = rdest;
                    // compute into rdest
                    // load A into rdest if A is not already rdest
                    if (isNumber(aRegOrImm)) {
                        out.push_back("MOV " + rdest + ", " + aRegOrImm);
                    } else if (!aRegOrImm.empty() && aRegOrImm[0] == 'r' && aRegOrImm != rdest) {
                        out.push_back("MOV " + rdest + ", " + aRegOrImm);
                    } else if (!(aRegOrImm.size()>0 && aRegOrImm[0]=='r')) {
                        // variable
                        out.push_back("MOV " + rdest + ", " + aRegOrImm);
                    }
                    // emit operation with B
                    if (op == "+") out.push_back("ADD " + rdest + ", " + (isNumber(bRegOrImm) ? bRegOrImm : bRegOrImm));
                    else if (op == "-") out.push_back("SUB " + rdest + ", " + bRegOrImm);
                    else if (op == "*") out.push_back("MUL " + rdest + ", " + bRegOrImm);
                    else if (op == "/") out.push_back("DIV " + rdest + ", " + bRegOrImm);
                    // relational ops produce 1/0 -> implement via conditional sequence
                    else if (op == "<" || op == "<=" || op == ">" || op == ">=" || op == "==" || op == "!=") {
                        // create two labels
                        static int labCounter = 0;
                        string lt = "__Ltrue" + to_string(labCounter);
                        string le = "__Lend" + to_string(labCounter);
                        labCounter++;
                        // compare rdest and B (ensure B in register or immediate)
                        if (!(isNumber(bRegOrImm) || (!bRegOrImm.empty() && bRegOrImm[0]=='r'))) {
                            // load B to a temp reg
                            string rb = "r_cmp";
                            out.push_back("MOV " + rb + ", " + bRegOrImm);
                            out.push_back("CMP " + rdest + ", " + rb);
                        } else {
                            out.push_back("CMP " + rdest + ", " + bRegOrImm);
                        }
                        string jop;
                        if (op == "<") jop = "JL";
                        else if (op == "<=") jop = "JLE";
                        else if (op == ">") jop = "JG";
                        else if (op == ">=") jop = "JGE";
                        else if (op == "==") jop = "JE";
                        else if (op == "!=") jop = "JNE";
                        out.push_back(jop + " " + lt);
                        out.push_back("MOV " + rdest + ", 0");
                        out.push_back("JMP " + le);
                        out.push_back(lt + ":");
                        out.push_back("MOV " + rdest + ", 1");
                        out.push_back(le + ":");
                    }
                } else {
                    // lhs is a variable, compute expression into a temp register then MOV to var
                    string rtemp = "r_tmp";
                    // load A
                    if (isNumber(aRegOrImm)) out.push_back("MOV " + rtemp + ", " + aRegOrImm);
                    else out.push_back("MOV " + rtemp + ", " + aRegOrImm);
                    if (op == "+") out.push_back("ADD " + rtemp + ", " + bRegOrImm);
                    else if (op == "-") out.push_back("SUB " + rtemp + ", " + bRegOrImm);
                    else if (op == "*") out.push_back("MUL " + rtemp + ", " + bRegOrImm);
                    else if (op == "/") out.push_back("DIV " + rtemp + ", " + bRegOrImm);
                    out.push_back("MOV " + lhs + ", " + rtemp);
                }
            }
            continue;
        }

        // If nothing matched just emit comment
        out.push_back("; unrecognized TAC: " + line);
    }

    // Epilogue (optional)
    out.push_back("");
    out.push_back("; --- end of generated assembly ---");
    return out;
}

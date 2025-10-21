#include "opt.h"
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <iostream>

using namespace std;

// Helper: is numeric (int or float)
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

// Split a line into tokens (space-separated)
static vector<string> splitTok(const string &line) {
    vector<string> out;
    istringstream iss(line);
    string t;
    while (iss >> t) out.push_back(t);
    return out;
}

// Join tokens with spaces
static string joinTok(const vector<string> &toks) {
    string s;
    for (size_t i = 0; i < toks.size(); ++i) {
        if (i) s += " ";
        s += toks[i];
    }
    return s;
}

// Try to compute simple binary op for numeric constants
static bool computeConst(const string &a, const string &op, const string &b, string &out) {
    if (!isNumber(a) || !isNumber(b)) return false;
    bool isFloat = (a.find('.') != string::npos) || (b.find('.') != string::npos);
    try {
        if (isFloat) {
            double da = stod(a);
            double db = stod(b);
            double res = 0.0;
            if (op == "+") res = da + db;
            else if (op == "-") res = da - db;
            else if (op == "*") res = da * db;
            else if (op == "/") { if (db == 0) return false; res = da / db; }
            else if (op == "<") { out = (da < db) ? "1" : "0"; return true; }
            else if (op == "<=") { out = (da <= db) ? "1" : "0"; return true; }
            else if (op == ">") { out = (da > db) ? "1" : "0"; return true; }
            else if (op == ">=") { out = (da >= db) ? "1" : "0"; return true; }
            else if (op == "==") { out = (da == db) ? "1" : "0"; return true; }
            else if (op == "!=") { out = (da != db) ? "1" : "0"; return true; }
            else return false;
            ostringstream oss;
            oss << std::setprecision(12) << res;
            out = oss.str();
            if (out.find('.') != string::npos) {
                while (!out.empty() && out.back() == '0') out.pop_back();
                if (!out.empty() && out.back() == '.') out.pop_back();
            }
            return true;
        } else {
            long long ia = stoll(a);
            long long ib = stoll(b);
            long long r = 0;
            if (op == "+") r = ia + ib;
            else if (op == "-") r = ia - ib;
            else if (op == "*") r = ia * ib;
            else if (op == "/") { if (ib == 0) return false; r = ia / ib; }
            else if (op == "<") { out = (ia < ib) ? "1" : "0"; return true; }
            else if (op == "<=") { out = (ia <= ib) ? "1" : "0"; return true; }
            else if (op == ">") { out = (ia > ib) ? "1" : "0"; return true; }
            else if (op == ">=") { out = (ia >= ib) ? "1" : "0"; return true; }
            else if (op == "==") { out = (ia == ib) ? "1" : "0"; return true; }
            else if (op == "!=") { out = (ia != ib) ? "1" : "0"; return true; }
            else return false;
            out = to_string(r);
            return true;
        }
    } catch (...) { return false; }
}

// One pass of optimization. Returns true if code was changed.
static bool optimizationPass(vector<string> &code) {
    if (code.empty()) return false;
    bool changedAny = false;

    // 1) Build definitions for temporaries: tX -> (line index, tokens)
    unordered_map<string,int> defLine;
    unordered_map<string, vector<string>> defTokens;

    for (size_t i = 0; i < code.size(); ++i) {
        auto toks = splitTok(code[i]);
        if (toks.size() >= 3 && toks[1] == "=") {
            string lhs = toks[0];
            if (!lhs.empty() && lhs[0] == 't') {
                defLine[lhs] = (int)i;
                defTokens[lhs] = toks;
            }
        }
    }

    // containers for changes
    unordered_set<int> removeLines;                 // lines to remove
    unordered_map<string,string> replaceWithConst;  // temp -> replacement token (const or var)

    // 2) Constant folding & algebraic simplification for temp definitions
    for (auto &p : defTokens) {
        const string &temp = p.first;
        const vector<string> &toks = p.second;
        if (toks.size() == 5) {
            string a = toks[2], op = toks[3], b = toks[4];
            string result;

            // Constant folding
            if (computeConst(a, op, b, result)) {
                replaceWithConst[temp] = result;
                removeLines.insert(defLine[temp]);
                changedAny = true;
                continue;
            }

            // Algebraic simplifications (x + 0, x * 1, x * 0, x / 1, etc.)
            if (op == "+" && b == "0") { replaceWithConst[temp] = a; removeLines.insert(defLine[temp]); changedAny = true; }
            else if (op == "+" && a == "0") { replaceWithConst[temp] = b; removeLines.insert(defLine[temp]); changedAny = true; }
            else if (op == "-" && b == "0") { replaceWithConst[temp] = a; removeLines.insert(defLine[temp]); changedAny = true; }
            else if (op == "*" && b == "1") { replaceWithConst[temp] = a; removeLines.insert(defLine[temp]); changedAny = true; }
            else if (op == "*" && a == "1") { replaceWithConst[temp] = b; removeLines.insert(defLine[temp]); changedAny = true; }
            else if (op == "*" && (a == "0" || b == "0")) { replaceWithConst[temp] = "0"; removeLines.insert(defLine[temp]); changedAny = true; }
            else if (op == "/" && b == "1") { replaceWithConst[temp] = a; removeLines.insert(defLine[temp]); changedAny = true; }
        } else if (toks.size() == 3) {
            // t = X  (direct copy) -> can be propagated
            replaceWithConst[temp] = toks[2];
            removeLines.insert(defLine[temp]);
            changedAny = true;
        }
    }

    // 3) Apply replaceWithConst to all lines (replace tokens equal to temp)
    for (size_t i = 0; i < code.size(); ++i) {
        if (removeLines.count((int)i)) continue; // will be removed
        string before = code[i];
        auto toks = splitTok(code[i]);
        bool changedLine = false;
        for (size_t j = 0; j < toks.size(); ++j) {
            auto it = replaceWithConst.find(toks[j]);
            if (it != replaceWithConst.end()) {
                toks[j] = it->second;
                changedLine = true;
            }
        }
        if (changedLine) {
            code[i] = joinTok(toks);
            changedAny = true;
        }
    }

    // 4) Recompute defTokens/defLine for remaining temps (since we removed some)
    defLine.clear();
    defTokens.clear();
    for (size_t i = 0; i < code.size(); ++i) {
        if (removeLines.count((int)i)) continue;
        auto toks = splitTok(code[i]);
        if (toks.size() >= 3 && toks[1] == "=") {
            string lhs = toks[0];
            if (!lhs.empty() && lhs[0] == 't') {
                defLine[lhs] = (int)i;
                defTokens[lhs] = toks;
            }
        }
    }

    // 5) Count uses of temps
    unordered_map<string,int> useCount;
    for (size_t i = 0; i < code.size(); ++i) {
        if (removeLines.count((int)i)) continue;
        auto toks = splitTok(code[i]);
        for (auto &tk : toks) {
            if (!tk.empty() && tk[0] == 't') useCount[tk]++;
        }
    }

    // 6) Inline temps used exactly once
    for (auto &p : defTokens) {
        const string &temp = p.first;
        const vector<string> &toks = p.second;
        if (useCount[temp] == 1 && defLine.count(temp)) {
            int defIdx = defLine[temp];

            // find the single use and replace
            for (size_t i = 0; i < code.size(); ++i) {
                if (removeLines.count((int)i)) continue;
                if ((int)i == defIdx) continue;
                auto useToks = splitTok(code[i]);
                bool changedLine = false;
                for (size_t j = 0; j < useToks.size(); ++j) {
                    if (useToks[j] == temp) {
                        if (toks.size() == 5) { // t = A op B
                            // replace temp token by "A op B" (as three tokens)
                            // splice tokens
                            vector<string> newT;
                            for (size_t k=0;k<j;++k) newT.push_back(useToks[k]);
                            newT.push_back(toks[2]);
                            newT.push_back(toks[3]);
                            newT.push_back(toks[4]);
                            for (size_t k=j+1;k<useToks.size();++k) newT.push_back(useToks[k]);
                            useToks.swap(newT);
                            changedLine = true;
                        } else if (toks.size() == 3) { // t = X
                            useToks[j] = toks[2];
                            changedLine = true;
                        }
                        // don't break; continue scanning in case multiple occurrences (though useCount==1)
                    }
                }
                if (changedLine) {
                    string oldLine = code[i];
                    code[i] = joinTok(useToks);
                    if (code[i] != oldLine) changedAny = true;
                }
            }

            // schedule temp definition removal
            removeLines.insert(defIdx);
        }
    }

    // 7) Rebuild code skipping removed lines
    vector<string> newCode;
    for (size_t i = 0; i < code.size(); ++i) {
        if (removeLines.count((int)i)) continue;
        newCode.push_back(code[i]);
    }

    if (newCode.size() != code.size()) changedAny = true;
    code.swap(newCode);

    return changedAny;
}

void optimizeTAC(vector<string> &code) {
    if (code.empty()) return;

    // Run multiple passes until no change
    while (true) {
        bool changed = optimizationPass(code);
        if (!changed) break;
    }
}

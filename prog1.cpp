/*
 * prog1.cpp
 *
 * Lexer driver: prints every token in a SADAL source file.
 * Usage: sadal-lex <file>
 */

#include <fstream>
#include "lex.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <file>" << endl;
        return 1;
    }

    ifstream file(argv[1]);
    if (!file.is_open()) {
        cerr << "CANNOT OPEN THE FILE " << argv[1] << endl;
        return 1;
    }

    int line = 1;
    int count = 0;
    LexItem tok;
    while ((tok = getNextToken(file, line)) != DONE) {
        cout << tok << endl;
        if (tok == ERR) {
            return 1;
        }
        count++;
    }

    cout << endl << "Lines: " << line - 1 << endl;
    cout << "Total Tokens: " << count << endl;
    return 0;
}

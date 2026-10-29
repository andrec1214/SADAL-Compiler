/*
 * prog2.cpp
 *
 * Parser driver: checks a SADAL source file for syntax errors.
 * Usage: sadal-parse <file>
 */

#include <fstream>
#include "parser.h"

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
    bool status = Prog(file, line);

    if (!status) {
        cout << "\nUnsuccessful Parsing" << endl;
        cout << "Number of Syntax Errors " << ErrCount() << endl;
        return 1;
    }
    cout << "\nSuccessful Parsing" << endl;
    return 0;
}

/*
 * prog3.cpp
 *
 * Interpreter driver: runs a SADAL program. Get statements read from
 * standard input.
 * Usage: sadal <file>
 */

#include <fstream>
#include "parserInterp.h"

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
        cout << "\nUnsuccessful Interpretation" << endl;
        cout << "Number of Errors " << ErrCount() << endl;
        return 1;
    }
    return 0;
}

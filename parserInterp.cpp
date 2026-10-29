/*
 * parserInterp.cpp
 *
 * Recursive descent interpreter for SADAL. Each production both checks
 * syntax and evaluates. Branches of an IF that are not taken are still
 * parsed, but with `executing` turned off so they have no effect.
 */

#include <set>
#include <vector>
#include "parserInterp.h"

using namespace std;

map<string, bool> defVar;          // declared variables
map<string, Token> SymTable;       // variable -> declared type
map<string, Value> TempsResults;   // variable -> current value
set<string> constants;             // variables declared CONSTANT

static string progName;
static bool executing = true;

void ParseError(int line, string msg);

namespace Parser {
    bool pushed_back = false;
    LexItem pushed_token;

    static LexItem GetNextToken(istream& in, int& line) {
        if (pushed_back) {
            pushed_back = false;
            return pushed_token;
        }
        LexItem tok = getNextToken(in, line);
        if (tok == ERR) {
            ParseError(line, "Unrecognized Input Pattern (" + tok.GetLexeme() + ")");
        }
        return tok;
    }

    static void PushBackToken(LexItem& t) {
        if (pushed_back) {
            abort();
        }
        pushed_back = true;
        pushed_token = t;
    }
}

static int error_count = 0;

int ErrCount() {
    return error_count;
}

void ParseError(int line, string msg) {
    ++error_count;
    cout << line << ": " << msg << endl;
}

static void RunTimeError(int line, string msg) {
    ParseError(line, "Run-Time Error-" + msg);
}

static bool TypeMatches(Token type, const Value& v) {
    switch (type) {
    case INT:    return v.IsInt();
    case FLOAT:  return v.IsReal();
    case BOOL:   return v.IsBool();
    case STRING: return v.IsString();
    case CHAR:   return v.IsChar();
    default:     return false;
    }
}

// Applies a binary operator to lhs and rhs, storing the result in lhs.
// Does nothing when not executing.
static bool ApplyOp(int line, Token op, Value& lhs, const Value& rhs) {
    if (!executing) {
        return true;
    }

    if ((op == DIV || op == MOD) && rhs.IsInt() && rhs.GetInt() == 0) {
        RunTimeError(line, "Division by zero.");
        return false;
    }
    if (op == DIV && rhs.IsReal() && rhs.GetReal() == 0.0) {
        RunTimeError(line, "Division by zero.");
        return false;
    }

    Value result;
    switch (op) {
    case PLUS:   result = lhs + rhs; break;
    case MINUS:  result = lhs - rhs; break;
    case MULT:   result = lhs * rhs; break;
    case DIV:    result = lhs / rhs; break;
    case MOD:    result = lhs % rhs; break;
    case EXP:    result = lhs.Exp(rhs); break;
    case CONCAT: result = lhs.Concat(rhs); break;
    case EQ:     result = lhs == rhs; break;
    case NEQ:    result = lhs != rhs; break;
    case LTHAN:  result = lhs < rhs; break;
    case LTE:    result = lhs <= rhs; break;
    case GTHAN:  result = lhs > rhs; break;
    case GTE:    result = lhs >= rhs; break;
    case AND:    result = lhs && rhs; break;
    case OR:     result = lhs || rhs; break;
    default:     break;
    }

    if (result.IsErr()) {
        RunTimeError(line, "Illegal operand type(s) for the operation.");
        return false;
    }
    lhs = result;
    return true;
}

// Prog ::= PROCEDURE ProcName IS ProcBody
bool Prog(istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != PROCEDURE) {
        ParseError(line, "Incorrect compilation file.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != IDENT) {
        ParseError(line, "Missing Procedure Name.");
        return false;
    }
    progName = tok.GetLexeme();

    tok = Parser::GetNextToken(in, line);
    if (tok != IS) {
        ParseError(line, "Missing IS after procedure name.");
        return false;
    }

    if (!ProcBody(in, line)) {
        ParseError(line, "Incorrect Procedure Definition.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != DONE) {
        ParseError(line, "Unexpected input after end of procedure.");
        return false;
    }
    return true;
}

// ProcBody ::= DeclPart BEGIN StmtList END ProcName ;
bool ProcBody(istream& in, int& line) {
    if (!DeclPart(in, line)) {
        ParseError(line, "Incorrect procedure body.");
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != BEGIN) {
        ParseError(line, "Missing BEGIN in procedure body.");
        return false;
    }

    if (!StmtList(in, line)) {
        ParseError(line, "Incorrect procedure body.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != END) {
        ParseError(line, "Missing END in procedure body.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != IDENT || tok.GetLexeme() != progName) {
        ParseError(line, "Procedure name mismatch in closing END.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line, "Missing semicolon after END " + progName + ".");
        return false;
    }
    return true;
}

// DeclPart ::= DeclStmt { DeclStmt }
bool DeclPart(istream& in, int& line) {
    if (!DeclStmt(in, line)) {
        ParseError(line, "Non-recognizable Declaration Part.");
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    while (tok != BEGIN) {
        Parser::PushBackToken(tok);
        if (!DeclStmt(in, line)) {
            ParseError(line, "Non-recognizable Declaration Part.");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(tok);
    return true;
}

// DeclStmt ::= IDENT {, IDENT } : [CONSTANT] Type [(Range)] [:= Expr] ;
bool DeclStmt(istream& in, int& line) {
    vector<string> names;
    LexItem tok;

    do {
        tok = Parser::GetNextToken(in, line);
        if (tok != IDENT) {
            ParseError(line, "Incorrect identifiers list in Declaration Statement.");
            return false;
        }

        string name = tok.GetLexeme();
        if (defVar.count(name) || name == progName) {
            ParseError(line, "Variable Redefinition");
            ParseError(line, "Incorrect identifiers list in Declaration Statement.");
            return false;
        }
        defVar[name] = true;
        names.push_back(name);

        tok = Parser::GetNextToken(in, line);
    } while (tok == COMMA);

    if (tok == IDENT) {
        ParseError(line, "Missing comma in declaration statement.");
        ParseError(line, "Incorrect identifiers list in Declaration Statement.");
        return false;
    }
    if (tok != COLON) {
        ParseError(line, "Invalid name for an Identifier: (" + tok.GetLexeme() + ")");
        ParseError(line, "Incorrect identifiers list in Declaration Statement.");
        return false;
    }

    bool isConst = false;
    tok = Parser::GetNextToken(in, line);
    if (tok == CONST) {
        isConst = true;
    }
    else {
        Parser::PushBackToken(tok);
    }

    Token type;
    if (!Type(in, line, type)) {
        return false;
    }

    // An optional (Range) constraint, e.g. STRING(1..10). It is checked
    // for syntax but not enforced.
    tok = Parser::GetNextToken(in, line);
    if (tok == LPAREN) {
        Value lo, hi;
        bool isSlice;
        if (!Range(in, line, lo, hi, isSlice)) {
            ParseError(line, "Invalid range in declaration.");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
        if (tok != RPAREN) {
            ParseError(line, "Missing right parenthesis in declaration.");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
    }

    for (const string& name : names) {
        SymTable[name] = type;
        if (isConst) {
            constants.insert(name);
        }
    }

    if (tok == ASSOP) {
        Value init;
        if (!Expr(in, line, init)) {
            ParseError(line, "Invalid initialization expression.");
            return false;
        }
        if (!TypeMatches(type, init)) {
            RunTimeError(line, "Illegal type for initialization of declared variable.");
            return false;
        }
        for (const string& name : names) {
            TempsResults[name] = init;
        }
        tok = Parser::GetNextToken(in, line);
    }
    else if (isConst) {
        ParseError(line, "Missing initialization for a constant.");
        return false;
    }

    if (tok != SEMICOL) {
        ParseError(line - 1, "Missing semicolon at end of statement");
        ParseError(line, "Incorrect Declaration Statement.");
        return false;
    }
    return true;
}

// Type ::= INTEGER | FLOAT | BOOLEAN | STRING | CHARACTER
bool Type(istream& in, int& line, Token& type) {
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != INT && tok != FLOAT && tok != BOOL && tok != CHAR && tok != STRING) {
        ParseError(line, "Incorrect Declaration Type.");
        return false;
    }
    type = tok.GetToken();
    return true;
}

// StmtList ::= Stmt { Stmt }
bool StmtList(istream& in, int& line) {
    if (!Stmt(in, line)) {
        ParseError(line, "Syntactic error in statement list.");
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    while (tok != END && tok != ELSIF && tok != ELSE) {
        Parser::PushBackToken(tok);
        if (!Stmt(in, line)) {
            ParseError(line, "Syntactic error in statement list.");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(tok);
    return true;
}

// Stmt ::= AssignStmt | PrintStmts | GetStmt | IfStmt
bool Stmt(istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    Parser::PushBackToken(tok);

    switch (tok.GetToken()) {
    case IDENT:
        return AssignStmt(in, line);
    case PUT:
    case PUTLN:
        return PrintStmts(in, line);
    case GET:
        return GetStmt(in, line);
    case IF:
        return IfStmt(in, line);
    default:
        ParseError(line, "Invalid statement.");
        return false;
    }
}

// PrintStmts ::= (PutLine | Put) ( Expr ) ;
bool PrintStmts(istream& in, int& line) {
    LexItem kind = Parser::GetNextToken(in, line);

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != LPAREN) {
        ParseError(line, "Missing Left Parenthesis");
        ParseError(line, "Invalid put statement.");
        return false;
    }

    Value val;
    if (!Expr(in, line, val)) {
        ParseError(line, "Invalid put statement.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != RPAREN) {
        ParseError(line, "Missing Right Parenthesis");
        ParseError(line, "Invalid put statement.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line - 1, "Missing semicolon at end of statement");
        ParseError(line, "Invalid put statement.");
        return false;
    }

    if (executing) {
        cout << val;
        if (kind == PUTLN) {
            cout << endl;
        }
    }
    return true;
}

// GetStmt ::= Get ( Var ) ;
bool GetStmt(istream& in, int& line) {
    Parser::GetNextToken(in, line);

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != LPAREN) {
        ParseError(line, "Missing Left Parenthesis");
        ParseError(line, "Invalid get statement.");
        return false;
    }

    LexItem idtok;
    if (!Var(in, line, idtok)) {
        ParseError(line, "Invalid get statement.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != RPAREN) {
        ParseError(line, "Missing Right Parenthesis");
        ParseError(line, "Invalid get statement.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line - 1, "Missing semicolon at end of statement");
        ParseError(line, "Invalid get statement.");
        return false;
    }

    if (!executing) {
        return true;
    }

    string name = idtok.GetLexeme();
    if (constants.count(name)) {
        RunTimeError(line, "Illegal input to a constant.");
        return false;
    }

    // Numbers, characters and booleans are read as one whitespace
    // delimited word; strings take the rest of the input line.
    Value val;
    string word;
    switch (SymTable[name]) {
    case INT: {
        int i;
        if (cin >> i) val = Value(i);
        break;
    }
    case FLOAT: {
        double d;
        if (cin >> d) val = Value(d);
        break;
    }
    case CHAR: {
        char c;
        if (cin >> c) val = Value(c);
        break;
    }
    case BOOL:
        if (cin >> word) {
            for (char& c : word) c = tolower(c);
            if (word == "true") val = Value(true);
            else if (word == "false") val = Value(false);
        }
        break;
    case STRING:
        if (cin >> ws && getline(cin, word)) val = Value(word);
        break;
    default:
        break;
    }

    if (val.IsErr()) {
        RunTimeError(line, "Invalid input for variable " + name + ".");
        return false;
    }
    TempsResults[name] = val;
    return true;
}

// Parses an IF/ELSIF condition and decides whether its branch runs.
static bool Condition(istream& in, int& line, bool& run) {
    Value cond;
    if (!Expr(in, line, cond)) {
        ParseError(line, "Missing if statement condition");
        return false;
    }

    run = false;
    if (executing) {
        if (!cond.IsBool()) {
            RunTimeError(line, "Illegal type for if statement condition.");
            return false;
        }
        run = cond.GetBool();
    }

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != THEN) {
        ParseError(line, "Missing THEN in if statement.");
        return false;
    }
    return true;
}

// Parses a branch body, executing it only when run is true.
static bool Branch(istream& in, int& line, bool run, bool saved) {
    executing = run;
    bool status = StmtList(in, line);
    executing = saved;
    return status;
}

// IfStmt ::= IF Expr THEN StmtList { ELSIF Expr THEN StmtList } [ ELSE StmtList ] END IF ;
bool IfStmt(istream& in, int& line) {
    Parser::GetNextToken(in, line);

    bool saved = executing;
    bool taken = false;
    bool run;

    if (!Condition(in, line, run)) {
        ParseError(line, "Invalid If statement.");
        return false;
    }
    if (!Branch(in, line, run, saved)) {
        ParseError(line, "Missing Statement for If-Stmt Then-clause");
        ParseError(line, "Invalid If statement.");
        return false;
    }
    taken = run;

    LexItem tok = Parser::GetNextToken(in, line);
    while (tok == ELSIF) {
        // once a branch has run, later conditions are parsed but not evaluated
        executing = saved && !taken;
        bool ok = Condition(in, line, run);
        executing = saved;
        if (!ok) {
            ParseError(line, "Invalid Elsif clause.");
            return false;
        }
        if (!Branch(in, line, run, saved)) {
            ParseError(line, "Missing Statement for If-Stmt Elsif-clause");
            return false;
        }
        taken = taken || run;
        tok = Parser::GetNextToken(in, line);
    }

    if (tok == ELSE) {
        if (!Branch(in, line, saved && !taken, saved)) {
            ParseError(line, "Missing Statement for If-Stmt Else-clause");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
    }

    if (tok != END) {
        ParseError(line, "Missing closing END IF for If-statement.");
        ParseError(line, "Invalid If statement.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != IF) {
        ParseError(line, "Missing IF after END in If-statement.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line - 1, "Missing semicolon at end of statement");
        ParseError(line, "Invalid If statement.");
        return false;
    }
    return true;
}

// Var ::= IDENT
bool Var(istream& in, int& line, LexItem& idtok) {
    idtok = Parser::GetNextToken(in, line);
    if (idtok != IDENT) {
        ParseError(line, "Missing variable name.");
        return false;
    }
    if (!defVar.count(idtok.GetLexeme())) {
        ParseError(line, "Undeclared Variable");
        return false;
    }
    return true;
}

// AssignStmt ::= Var := Expr ;
bool AssignStmt(istream& in, int& line) {
    LexItem idtok;
    if (!Var(in, line, idtok)) {
        ParseError(line, "Invalid assignment statement.");
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != ASSOP) {
        ParseError(line, "Missing Assignment Operator");
        ParseError(line, "Invalid assignment statement.");
        return false;
    }

    Value val;
    if (!Expr(in, line, val)) {
        ParseError(line, "Illegal expression for an assignment statement");
        ParseError(line, "Invalid assignment statement.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line - 1, "Missing semicolon at end of statement");
        ParseError(line, "Invalid assignment statement.");
        return false;
    }

    if (executing) {
        string name = idtok.GetLexeme();
        if (constants.count(name)) {
            RunTimeError(line, "Illegal assignment to a constant.");
            return false;
        }
        if (!TypeMatches(SymTable[name], val)) {
            RunTimeError(line, "Illegal mixed-mode assignment operation.");
            return false;
        }
        TempsResults[name] = val;
    }
    return true;
}

// Expr ::= Relation { (AND | OR) Relation }
bool Expr(istream& in, int& line, Value& retVal) {
    if (!Relation(in, line, retVal)) {
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    while (tok == AND || tok == OR) {
        Value rhs;
        if (!Relation(in, line, rhs)) {
            ParseError(line, "Missing operand after " + tok.GetLexeme() + ".");
            return false;
        }
        if (!ApplyOp(line, tok.GetToken(), retVal, rhs)) {
            return false;
        }
        tok = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(tok);
    return true;
}

// Relation ::= SimpleExpr [ ( = | /= | < | <= | > | >= ) SimpleExpr ]
bool Relation(istream& in, int& line, Value& retVal) {
    if (!SimpleExpr(in, line, retVal)) {
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok == EQ || tok == NEQ || tok == LTHAN || tok == GTHAN || tok == LTE || tok == GTE) {
        Value rhs;
        if (!SimpleExpr(in, line, rhs)) {
            ParseError(line, "Missing operand after relational operator.");
            return false;
        }
        return ApplyOp(line, tok.GetToken(), retVal, rhs);
    }
    Parser::PushBackToken(tok);
    return true;
}

// SimpleExpr ::= STerm { ( + | - | & ) STerm }
bool SimpleExpr(istream& in, int& line, Value& retVal) {
    if (!STerm(in, line, retVal)) {
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    while (tok == PLUS || tok == MINUS || tok == CONCAT) {
        Value rhs;
        if (!STerm(in, line, rhs)) {
            ParseError(line, "Missing operand after " + tok.GetLexeme() + ".");
            return false;
        }
        if (!ApplyOp(line, tok.GetToken(), retVal, rhs)) {
            return false;
        }
        tok = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(tok);
    return true;
}

// STerm ::= [ ( + | - ) ] Term
bool STerm(istream& in, int& line, Value& retVal) {
    LexItem tok = Parser::GetNextToken(in, line);
    bool hasSign = (tok == PLUS || tok == MINUS);
    if (!hasSign) {
        Parser::PushBackToken(tok);
    }

    if (!Term(in, line, retVal)) {
        return false;
    }

    if (hasSign && executing) {
        if (!retVal.IsNumeric()) {
            RunTimeError(line, "Illegal operand type for sign operator.");
            return false;
        }
        if (tok == MINUS) {
            retVal = -retVal;
        }
    }
    return true;
}

// Term ::= Factor { ( * | / | MOD ) Factor }
bool Term(istream& in, int& line, Value& retVal) {
    if (!Factor(in, line, retVal)) {
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    while (tok == MULT || tok == DIV || tok == MOD) {
        Value rhs;
        if (!Factor(in, line, rhs)) {
            ParseError(line, "Missing operand after " + tok.GetLexeme() + ".");
            return false;
        }
        if (!ApplyOp(line, tok.GetToken(), retVal, rhs)) {
            return false;
        }
        tok = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(tok);
    return true;
}

// Factor ::= Primary [ ** [(+ | -)] Primary ] | NOT Primary
bool Factor(istream& in, int& line, Value& retVal) {
    LexItem tok = Parser::GetNextToken(in, line);

    if (tok == NOT) {
        if (!Primary(in, line, retVal)) {
            ParseError(line, "Missing operand after NOT.");
            return false;
        }
        if (executing) {
            retVal = !retVal;
            if (retVal.IsErr()) {
                RunTimeError(line, "Illegal operand type for NOT operator.");
                return false;
            }
        }
        return true;
    }

    Parser::PushBackToken(tok);
    if (!Primary(in, line, retVal)) {
        ParseError(line, "Incorrect operand");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != EXP) {
        Parser::PushBackToken(tok);
        return true;
    }

    LexItem sign = Parser::GetNextToken(in, line);
    if (sign != PLUS && sign != MINUS) {
        Parser::PushBackToken(sign);
    }

    Value power;
    if (!Primary(in, line, power)) {
        ParseError(line, "Missing exponent operand.");
        return false;
    }
    if (executing && sign == MINUS) {
        power = -power;
    }
    return ApplyOp(line, EXP, retVal, power);
}

// Primary ::= Name | ICONST | FCONST | SCONST | BCONST | CCONST | ( Expr )
bool Primary(istream& in, int& line, Value& retVal) {
    LexItem tok = Parser::GetNextToken(in, line);

    switch (tok.GetToken()) {
    case IDENT:
        Parser::PushBackToken(tok);
        return Name(in, line, retVal);
    case ICONST:
        // stod accepts exponents like 12E3, which stoi would not
        retVal = Value((int)stod(tok.GetLexeme()));
        return true;
    case FCONST:
        retVal = Value(stod(tok.GetLexeme()));
        return true;
    case SCONST:
        retVal = Value(tok.GetLexeme());
        return true;
    case CCONST:
        retVal = Value(tok.GetLexeme()[0]);
        return true;
    case BCONST:
        retVal = Value(tok.GetLexeme() == "true");
        return true;
    case LPAREN:
        if (!Expr(in, line, retVal)) {
            ParseError(line, "Invalid expression in parentheses.");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
        if (tok != RPAREN) {
            ParseError(line, "Missing right parenthesis after expression");
            return false;
        }
        return true;
    default:
        ParseError(line, "Invalid Expression.");
        return false;
    }
}

// Name ::= IDENT [ ( Range ) ]
// Indexing applies to STRING variables and is 1-based, as in Ada:
// s(i) is a CHARACTER and s(i..j) is a STRING slice.
bool Name(istream& in, int& line, Value& retVal) {
    LexItem tok = Parser::GetNextToken(in, line);
    string name = tok.GetLexeme();

    if (!defVar.count(name)) {
        ParseError(line, "Using Undefined Variable");
        ParseError(line, "Invalid reference to a variable.");
        return false;
    }

    if (executing && !TempsResults.count(name)) {
        RunTimeError(line, "Using uninitialized variable " + name + ".");
        return false;
    }
    if (executing) {
        retVal = TempsResults[name];
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != LPAREN) {
        Parser::PushBackToken(tok);
        return true;
    }

    Value lo, hi;
    bool isSlice;
    if (!Range(in, line, lo, hi, isSlice)) {
        ParseError(line, "Invalid index for variable " + name + ".");
        return false;
    }
    tok = Parser::GetNextToken(in, line);
    if (tok != RPAREN) {
        ParseError(line, "Missing right parenthesis after index.");
        return false;
    }

    if (!executing) {
        return true;
    }

    if (!retVal.IsString()) {
        RunTimeError(line, "Only STRING variables can be indexed.");
        return false;
    }
    if (!lo.IsInt() || (isSlice && !hi.IsInt())) {
        RunTimeError(line, "String index must be an INTEGER.");
        return false;
    }

    string str = retVal.GetString();
    int len = str.length();
    int first = lo.GetInt();
    int last = isSlice ? hi.GetInt() : first;

    // an empty slice (last < first) is legal regardless of bounds
    if (isSlice && last < first) {
        retVal = Value(string());
        return true;
    }
    if (first < 1 || last > len) {
        RunTimeError(line, "String index out of range.");
        return false;
    }

    if (isSlice) {
        retVal = Value(str.substr(first - 1, last - first + 1));
    }
    else {
        retVal = Value(str[first - 1]);
    }
    return true;
}

// Range ::= SimpleExpr [ .. SimpleExpr ]
bool Range(istream& in, int& line, Value& lo, Value& hi, bool& isSlice) {
    if (!SimpleExpr(in, line, lo)) {
        return false;
    }

    isSlice = false;
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != DOT) {
        Parser::PushBackToken(tok);
        return true;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != DOT) {
        ParseError(line, "Invalid range operator, expected '..'.");
        return false;
    }

    isSlice = true;
    return SimpleExpr(in, line, hi);
}

#include <queue>
#include "parser.h"

using namespace std;
map<string, bool> defVar;

void ParseError(int line, string msg);

namespace Parser {
    bool pushed_back = false;
    LexItem	pushed_token;

    static LexItem GetNextToken(istream& in, int& line) {
        if( pushed_back ) {
            pushed_back = false;
            return pushed_token;
        }
        LexItem tok = getNextToken(in, line);
        if (tok == ERR) {
            ParseError(line, "Unrecognized Input Pattern (" + tok.GetLexeme() + ")");
        }
        return tok;
    }

    static void PushBackToken(LexItem & t) {
        if( pushed_back ) {
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

// StmtList ::= Stmt { Stmt }
bool StmtList(istream& in, int& line) {

    bool status;
    LexItem tok;
    status = Stmt(in, line);
    tok = Parser::GetNextToken(in, line);
    while(status && (tok != END && tok != ELSIF && tok != ELSE))
    {
        Parser::PushBackToken(tok);
        status = Stmt(in, line);
        tok = Parser::GetNextToken(in, line);
    }
    if(!status)
    {
        ParseError(line, "Syntactic error in statement list.");
        return false;
    }
    Parser::PushBackToken(tok);
    return true;
}

// DeclPart ::= DeclStmt { DeclStmt }
bool DeclPart(istream& in, int& line) {
  bool status = false;
  LexItem tok;
  status = DeclStmt(in, line);
  if(status)
  {
    tok = Parser::GetNextToken(in, line);
    if(tok == BEGIN )
    {
      Parser::PushBackToken(tok);
      return true;
    }
    else
    {
      Parser::PushBackToken(tok);
      status = DeclPart(in, line);
    }
  }
  else
  {
    ParseError(line, "Non-recognizable Declaration Part.");
    return false;
  }
  return true;
}

// Prog ::= PROCEDURE ProcName IS ProcBody
bool Prog(istream& in, int& line) {
  LexItem tok;
  tok = Parser::GetNextToken(in, line);

  if (tok != PROCEDURE) {
    ParseError(line, "Incorrect compilation file.");
    return false;
  }

  tok = Parser::GetNextToken(in, line);
  if (tok != IDENT) {
    ParseError(line, "Missing Procedure Name.");
    return false;
  }

  defVar[tok.GetLexeme()] = true;

  tok = Parser::GetNextToken(in, line);
  if (tok != IS) {
    ParseError(line, "Missing IS after procedure name.");
    return false;
  }

  if (!ProcBody(in, line)) {
    ParseError(line, "Incorrect Procedure Definition.");
    return false;
  }

  cout << "Declared Variables:" << endl;
  for (auto it = defVar.begin(); it != defVar.end(); ++it) {
    cout << it->first;
    if (next(it) != end(defVar)) {
      cout << ", ";
    }
  }

  cout << endl;
  cout << "\n(DONE)" << endl;
  return true;
}

// ProcBody ::= DeclPart BEGIN StmtList END ProcName ;
bool ProcBody(istream& in, int& line) {
  LexItem tok;
  if (!DeclPart(in, line)) {
    ParseError(line, "Incorrect procedure body.");
    return false;
  }

  tok = Parser::GetNextToken(in, line);
  if (tok != BEGIN) {
    ParseError(line, "Incorrect procedure body.");
    return false;
  }

  if (!StmtList(in, line)) {
    ParseError(line, "Incorrect procedure body.");
    return false;
  }

  tok = Parser::GetNextToken(in, line);
  if (tok != END) {
    ParseError(line, "Incorrect procedure body.");
    return false;
  }

  tok = Parser::GetNextToken(in, line);
  if (tok != IDENT) {
    ParseError(line, "Incorrect procedure body.");
    return false;
  }

  tok = Parser::GetNextToken(in, line);
  if (tok != SEMICOL) {
    ParseError(line, "Incorrect procedure body.");
    return false;
  }

  return true;
}

// DeclStmt ::= IDENT {, IDENT } : [CONSTANT] Type [(Range)] [:= Expr] ;
bool DeclStmt(istream& in, int& line) {
  LexItem tok;
  tok = Parser::GetNextToken(in, line);
  string name = tok.GetLexeme();

  if (tok != IDENT) {
    ParseError(line, "Incorrect identifiers list in Declaration Statement.");
    return false;
  }

  if (defVar.find(name) != defVar.end()) {
    ParseError(line, "Variable Redefinition");
    ParseError(line, "Incorrect identifiers list in Declaration Statement.");
    return false;
  }

  // put the first ident in
  defVar[name] = true;

  // after first ident
  tok = Parser::GetNextToken(in, line);
  while (tok == COMMA) {

    tok = Parser::GetNextToken(in, line);
    name = tok.GetLexeme();

    if (tok != IDENT) {
      ParseError(line, "Missing ident.");
      ParseError(line, "Incorrect identifiers list in Declaration Statement.");
      return false;
    }

    if (defVar.find(name) != defVar.end()) {
      ParseError(line, "Variable Redefinition");
      ParseError(line, "Incorrect identifiers list in Declaration Statement.");

      return false;
    }

    defVar[name] = true;
    tok = Parser::GetNextToken(in, line);
  }

  if (tok == IDENT) {
    ParseError(line, "Missing comma in declaration statement.");
    ParseError(line, "Incorrect identifiers list in Declaration Statement.");
    return false;
  }

  if (tok != COLON) {
    ParseError(line, "Invalid name for an Identifier:");
    cout << "(" + tok.GetLexeme() + ")" << endl;
    ParseError(line, "Incorrect identifiers list in Declaration Statement.");
    return false;
  }

  tok = Parser::GetNextToken(in, line);

  if (tok != CONST) {
    Parser::PushBackToken(tok);
  }

  if (!Type(in, line)) {
    return false;
  }

  tok = Parser::GetNextToken(in, line);
  if (tok == LPAREN) {
    if (!Range(in, line)) {
      ParseError(line, "Invalid range in declaration.");
      return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != RPAREN) {
      ParseError(line, "No closing parenthesis");
      return false;
    }

    tok = Parser::GetNextToken(in, line);
  }

  if (tok == ASSOP) {
    if (!Expr(in, line)) {
      ParseError(line, "Invalid initialization expression.");
      return false;
    }

    tok = Parser::GetNextToken(in, line);
  }

  if (tok != SEMICOL) {
    ParseError(line - 1, "Missing semicolon at end of statement");
    return false;
  }

  return true;
}

// Type ::= INTEGER | FLOAT | BOOLEAN | STRING | CHARACTER
bool Type(istream& in, int& line) {
 LexItem tok = Parser::GetNextToken(in, line);
 if (tok != INT && tok != FLOAT && tok != BOOL && tok != CHAR && tok != STRING) {
   ParseError(line, "Incorrect Declaration Type.");
   return false;
 }
 return true;
}

// Stmt ::= AssignStmt | PrintStmts | GetStmt | IfStmt
bool Stmt(istream& in, int& line) {
  LexItem tok = Parser::GetNextToken(in, line);

  if (tok == IDENT) {
    Parser::PushBackToken(tok);
    return AssignStmt(in, line);
  }

  else if (tok == PUT || tok == PUTLN) {
    return PrintStmts(in, line);
  }

  else if (tok == GET) {
    return GetStmt(in, line);
  }

  else if (tok == IF) {
    return IfStmt(in, line);
  }

  ParseError(line, "Invalid statement.");
  return false;
}

// PrintStmts ::= (PutLine | Put) ( Expr) ;
bool PrintStmts(istream& in, int& line) {
  LexItem tok = Parser::GetNextToken(in, line);

  if (tok == LPAREN) {
    if (!Expr(in, line)) {
      ParseError(line, "Invalid put statement.");
      return false;
    }

    tok = Parser::GetNextToken(in, line);

    if (tok != RPAREN) {
      ParseError(line, "Missing Right Parenthesis");
      ParseError(line, "Invalid put statement.");
      return false;
    }

    else {
      tok = Parser::GetNextToken(in, line);
    }
  }
  else {
    ParseError(line, "Missing Left Parenthesis");
    ParseError(line, "Invalid put statement.");
    return false;
  }

  if (tok != SEMICOL) {
    ParseError(line - 1, "Missing semicolon at end of statement");
    ParseError(line, "Invalid put statement.");
    return false;
  }

  return true;
}

// GetStmt := Get (Var) ;
bool GetStmt(istream& in, int& line) {
  LexItem tok = Parser::GetNextToken(in, line);

  if (tok == LPAREN) {
    if (!Var(in, line)) {
      ParseError(line, "Invalid get statement.");
      return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != RPAREN) {
      ParseError(line, "Missing Right Parenthesis");
      return false;
    }

    tok = Parser::GetNextToken(in, line);
  }

  else {
    ParseError(line, "Missing Left Parenthesis");
    return false;
  }

  if (tok != SEMICOL) {
    ParseError(line - 1, "Missing semicolon at end of statement");
    return false;
  }

  return true;
}

// IfStmt ::= IF Expr THEN StmtList { ELSIF Expr THEN StmtList } [ ELSE StmtList ] END IF ;
bool IfStmt(istream& in, int& line) {
  LexItem tok = Parser::GetNextToken(in, line);
  Parser::PushBackToken(tok);

  if (!Expr(in, line)) {
    ParseError(line, "Missing if statement condition");
    ParseError(line, "Invalid If statement.");
    return false;
  }

  tok = Parser::GetNextToken(in, line);
  if (tok != THEN) {
    ParseError(line, "Invalid If statement.");
    return false;
  }

  if (!StmtList(in, line)) {
    ParseError(line, "Missing Statement for If-Stmt Then-clause");
    ParseError(line, "Invalid If statement.");
    return false;
  }

  tok = Parser::GetNextToken(in, line);
  while (tok == ELSIF) {
    if (!Expr(in, line)) {
      ParseError(line, "Missing Elsif clause condition");
      return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != THEN) {
      ParseError(line, "Missing THEN in Elsif clause.");
      return false;
    }

    if (!StmtList(in, line)) {
      ParseError(line, "Missing Statement for If-Stmt Elsif-clause");
      return false;
    }

    tok = Parser::GetNextToken(in, line);
  }

  if (tok == ELSE) {
    if (!StmtList(in, line)) {
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
    return false;
  }

  return true;
}

// Var ::= IDENT
bool Var(istream& in, int& line) {
  LexItem tok = Parser::GetNextToken(in, line);

  if (tok != IDENT) {
    ParseError(line, "Missing variable name.");
    return false;
  }

  if (defVar.find(tok.GetLexeme()) == defVar.end()) {
    ParseError(line, "Undeclared Variable");
    return false;
  }

  return true;
}

// AssignStmt ::= Var := Expr ;
bool AssignStmt(istream& in, int& line) {
  if (!Var(in, line)) {
    ParseError(line, "Invalid assignment statement.");
    return false;
  }

  LexItem tok = Parser::GetNextToken(in, line);
  if (tok != ASSOP) {
    ParseError(line, "Missing Assignment Operator");
    ParseError(line, "Invalid assignment statement.");
    return false;
  }

  if (!Expr(in, line)) {
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

  return true;
}

// Expr ::= Relation {(AND | OR) Relation }
bool Expr(istream& in, int& line) {
  LexItem tok;

  // first relation
  if (!Relation(in, line)) {
    return false;
  }

  // next token
  tok = Parser::GetNextToken(in, line);

  // optional and/or relation
  while (tok == AND || tok == OR) {
    if (!Relation(in, line)) {
      ParseError(line, "Missing operand after logical operator.");
      return false;
    }

    tok = Parser::GetNextToken(in, line);
  }

  Parser::PushBackToken(tok);
  return true;
}

// Relation ::= SimpleExpr [ ( = | /= | < | <= | > | >= ) SimpleExpr ]
bool Relation(istream& in, int& line) {
  LexItem tok;

  if (!SimpleExpr(in, line)) {
    return false;
  }

  tok = Parser::GetNextToken(in, line);
  if (tok == EQ || tok == NEQ || tok == LTHAN || tok == GTHAN || tok == LTE || tok == GTE) {
    if (!SimpleExpr(in, line)) {
      ParseError(line, "Missing operand after relational operator.");
      return false;
    }
  }
  else {
    Parser::PushBackToken(tok);
  }

  return true;
}

//SimpleExpr ::= STerm { ( + | - | & ) STerm }
bool SimpleExpr(istream& in, int& line) {
  LexItem tok = Parser::GetNextToken(in, line);
  Parser::PushBackToken(tok);
  if (!STerm(in, line)) {
    return false;
  }

  tok = Parser::GetNextToken(in, line);
  while (tok == PLUS || tok == MINUS || tok == CONCAT) {
    if (!STerm(in, line)) {
      ParseError(line, "Missing operand after additive operator.");
      return false;
    }

    tok = Parser::GetNextToken(in, line);
  }

  Parser::PushBackToken(tok);
  return true;
}

// STerm ::= [ ( + | - ) ] Term
bool STerm(istream& in, int& line) {
  LexItem tok = Parser::GetNextToken(in, line);
  int sign = 1;

  if (tok == MINUS) {
    sign = -1;
  }
  else if (tok != PLUS) {
    Parser::PushBackToken(tok);
  }

  if (!Term(in, line, sign)) {
    return false;
  }

  return true;
}

// Term ::= Factor { ( * | / | MOD ) Factor }
bool Term(istream& in, int& line, int sign) {
  LexItem tok;

  if (!Factor(in, line, sign)) {
    return false;
  }

  tok = Parser::GetNextToken(in, line);
  while (tok == MULT || tok == DIV || tok == MOD) {
    if (!Factor(in, line, sign)) {
      ParseError(line, "Missing operand after multiplicative operator.");
      return false;
    }

    tok = Parser::GetNextToken(in, line);
  }

  Parser::PushBackToken(tok);
  return true;
}

// Factor ::= Primary [** [(+ | -)] Primary ] | NOT Primary
bool Factor(istream& in, int& line, int sign) {
  LexItem tok = Parser::GetNextToken(in, line);

  // not block
  if (tok == NOT) {
    if (!Primary(in, line, sign)) {
      ParseError(line, "Missing operand after NOT.");
      return false;
    }
    return true;
  }

  // not not block
  else {
    Parser::PushBackToken(tok);

    if (!Primary(in, line, sign)) {
      ParseError(line, "Incorrect operand");
      return false;
    }

    tok = Parser::GetNextToken(in, line);
    // optional clause 1
    if (tok == EXP) {
      tok = Parser::GetNextToken(in, line);
      // optional clause 2
      if (tok != PLUS && tok != MINUS) {
        Parser::PushBackToken(tok);
      }

      if (!Primary(in, line, sign)) {
        ParseError(line, "Missing exponent operand.");
        return false;
      }
    }
    else {
      Parser::PushBackToken(tok);
    }

    return true;
  }
}

// Primary ::= Name | ICONST | FCONST | SCONST | BCONST | CCONST | (Expr)
bool Primary(istream& in, int& line, int sign) {
  LexItem tok = Parser::GetNextToken(in, line);

  if (tok == IDENT) {
    if (defVar.find(tok.GetLexeme()) == defVar.end()) {
      ParseError(line, "Using Undefined Variable");
      ParseError(line, "Invalid reference to a variable.");
      return false;
    }

    Parser::PushBackToken(tok);

    if (!Name(in, line)) {
      ParseError(line, "Invalid reference to a variable.");
      return false;
    }

    else if (tok == LPAREN) {
      if (!Expr(in, line)) {
        ParseError(line, "Invalid expression.");
        return false;
      }
    }

    else {
      return true;
    }
  }

  if (tok == ICONST || tok == FCONST || tok == SCONST || tok == BCONST || tok == CCONST) {
    return true;
  }

  if (tok != LPAREN) {
    ParseError(line, "Invalid Expression.");
    return false;
  }

  if (!Expr(in, line)) {
    ParseError(line, "Invalid expression in parentheses.");
    return false;
  }

  tok = Parser::GetNextToken(in, line);
  if (tok != RPAREN) {
    ParseError(line, "Missing right parenthesis after expression");
    return false;
  }

  return true;
}

// Name ::= IDENT [ ( Range ) ]
bool Name(istream& in, int& line) {
  LexItem tok = Parser::GetNextToken(in, line);

  if (tok != IDENT) {
    ParseError(line, "Missing variable name.");
    return false;
  }

  tok = Parser::GetNextToken(in, line);
  if (tok == LPAREN) {
    if (!Range(in, line)) {
      ParseError(line, "Invalid index for variable.");
      return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != RPAREN) {
      ParseError(line, "Missing right parenthesis after index.");
      return false;
    }
  }
  else {
    Parser::PushBackToken(tok);
  }

  return true;
}

// Range ::= SimpleExpr [. . SimpleExpr ]
bool Range(istream& in, int& line) {
  LexItem tok;

  if (!SimpleExpr(in, line)) {
    ParseError(line, "Missing lower bound in range.");
    return false;
  }

  tok = Parser::GetNextToken(in, line);
  if (tok == DOT) {
    tok = Parser::GetNextToken(in, line);
    if (tok != DOT) {
      ParseError(line, "Invalid range operator, expected '..'.");
      return false;
    }

    if (!SimpleExpr(in, line)) {
      ParseError(line, "Missing upper bound in range.");
      return false;
    }
  }
  else {
    Parser::PushBackToken(tok);
  }

  return true;
}
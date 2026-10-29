/*
 * parserInterp.h
 *
 * Recursive descent parser and interpreter for SADAL. Statements are
 * executed as they are parsed.
 */

#ifndef PARSE_INTERP_H_
#define PARSE_INTERP_H_

#include <iostream>

using namespace std;

#include "lex.h"
#include "val.h"

extern bool Prog(istream& in, int& line);
extern bool ProcBody(istream& in, int& line);
extern bool DeclPart(istream& in, int& line);
extern bool DeclStmt(istream& in, int& line);
extern bool Type(istream& in, int& line, Token& type);
extern bool StmtList(istream& in, int& line);
extern bool Stmt(istream& in, int& line);
extern bool PrintStmts(istream& in, int& line);
extern bool GetStmt(istream& in, int& line);
extern bool IfStmt(istream& in, int& line);
extern bool AssignStmt(istream& in, int& line);
extern bool Var(istream& in, int& line, LexItem& idtok);
extern bool Expr(istream& in, int& line, Value& retVal);
extern bool Relation(istream& in, int& line, Value& retVal);
extern bool SimpleExpr(istream& in, int& line, Value& retVal);
extern bool STerm(istream& in, int& line, Value& retVal);
extern bool Term(istream& in, int& line, Value& retVal);
extern bool Factor(istream& in, int& line, Value& retVal);
extern bool Primary(istream& in, int& line, Value& retVal);
extern bool Name(istream& in, int& line, Value& retVal);
extern bool Range(istream& in, int& line, Value& lo, Value& hi, bool& isSlice);
extern int ErrCount();

#endif /* PARSE_INTERP_H_ */

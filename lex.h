/*
 * lex.h
 *
 * Token definitions and lexical analyzer interface for SADAL
 * (Simple ADA-Like language).
 */

#ifndef LEX_H_
#define LEX_H_

#include <string>
#include <iostream>
#include <map>

using namespace std;

// Definition of all the possible token types
enum Token {
    // keywords or reserved words
    IF, ELSE, ELSIF, PUT, PUTLN, GET, INT, FLOAT,
    CHAR, STRING, BOOL, PROCEDURE, TRUE, FALSE, END,
    IS, BEGIN, THEN, CONST,

    // identifiers
    IDENT,

    // integer, real, string, boolean and character constants
    ICONST, FCONST, SCONST, BCONST, CCONST,

    // arithmetic, relational and logic operators
    PLUS, MINUS, MULT, DIV, ASSOP, EQ, NEQ,
    EXP, CONCAT, GTHAN, LTHAN, LTE, GTE, AND, OR, NOT, MOD,

    // delimiters
    COMMA, SEMICOL, LPAREN, RPAREN, DOT, COLON,

    // any error returns this token
    ERR,

    // signals end of file
    DONE,
};

class LexItem {
    Token   token;
    string  lexeme;
    int     lnum;

public:
    LexItem() {
        token = ERR;
        lnum = -1;
    }
    LexItem(Token token, string lexeme, int line) {
        this->token = token;
        this->lexeme = lexeme;
        this->lnum = line;
    }

    bool operator==(const Token token) const { return this->token == token; }
    bool operator!=(const Token token) const { return this->token != token; }

    Token   GetToken() const { return token; }
    string  GetLexeme() const { return lexeme; }
    int     GetLinenum() const { return lnum; }
};

extern ostream& operator<<(ostream& out, const LexItem& tok);
extern LexItem id_or_kw(const string& lexeme, int linenum);
extern LexItem getNextToken(istream& in, int& linenum);

#endif /* LEX_H_ */

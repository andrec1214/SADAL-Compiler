/*
 * lex.cpp
 *
 * Lexical analyzer for SADAL. Keywords are case-insensitive, identifiers
 * are case-sensitive, and comments start with "--" and run to end of line.
 */

#include <cctype>
#include <algorithm>
#include "lex.h"

static const map<string, Token> kwmap = {
    {"if", IF},
    {"else", ELSE},
    {"elsif", ELSIF},
    {"put", PUT},
    {"putline", PUTLN},
    {"get", GET},
    {"integer", INT},
    {"float", FLOAT},
    {"character", CHAR},
    {"string", STRING},
    {"boolean", BOOL},
    {"procedure", PROCEDURE},
    {"true", TRUE},
    {"false", FALSE},
    {"end", END},
    {"is", IS},
    {"begin", BEGIN},
    {"then", THEN},
    {"constant", CONST},
    {"and", AND},
    {"or", OR},
    {"not", NOT},
    {"mod", MOD},
};

static const map<Token, string> tokname = {
    {IF, "IF"}, {ELSE, "ELSE"}, {ELSIF, "ELSIF"}, {PUT, "PUT"},
    {PUTLN, "PUTLN"}, {GET, "GET"}, {INT, "INT"}, {FLOAT, "FLOAT"},
    {CHAR, "CHAR"}, {STRING, "STRING"}, {BOOL, "BOOL"},
    {PROCEDURE, "PROCEDURE"}, {TRUE, "TRUE"}, {FALSE, "FALSE"},
    {END, "END"}, {IS, "IS"}, {BEGIN, "BEGIN"}, {THEN, "THEN"},
    {CONST, "CONST"},
    {IDENT, "IDENT"},
    {ICONST, "ICONST"}, {FCONST, "FCONST"}, {SCONST, "SCONST"},
    {BCONST, "BCONST"}, {CCONST, "CCONST"},
    {PLUS, "PLUS"}, {MINUS, "MINUS"}, {MULT, "MULT"}, {DIV, "DIV"},
    {ASSOP, "ASSOP"}, {EQ, "EQ"}, {NEQ, "NEQ"}, {EXP, "EXP"},
    {CONCAT, "CONCAT"}, {GTHAN, "GTHAN"}, {LTHAN, "LTHAN"},
    {LTE, "LTE"}, {GTE, "GTE"}, {AND, "AND"}, {OR, "OR"},
    {NOT, "NOT"}, {MOD, "MOD"},
    {COMMA, "COMMA"}, {SEMICOL, "SEMICOL"}, {LPAREN, "LPAREN"},
    {RPAREN, "RPAREN"}, {DOT, "DOT"}, {COLON, "COLON"},
    {ERR, "ERR"},
    {DONE, "DONE"},
};

ostream& operator<<(ostream& out, const LexItem& tok) {
    Token t = tok.GetToken();
    out << tokname.at(t);

    switch (t) {
    case IDENT: case ICONST: case FCONST: case BCONST:
        out << ": <" << tok.GetLexeme() << ">";
        break;
    case SCONST:
        out << ": \"" << tok.GetLexeme() << "\"";
        break;
    case CCONST:
        out << ": '" << tok.GetLexeme() << "'";
        break;
    case ERR:
        out << ": In line " << tok.GetLinenum() << ", Error Message {"
            << tok.GetLexeme() << "}";
        break;
    default:
        break;
    }
    return out;
}

LexItem id_or_kw(const string& lexeme, int linenum) {
    string lower = lexeme;
    transform(lower.begin(), lower.end(), lower.begin(),
              [](unsigned char c) { return tolower(c); });

    auto it = kwmap.find(lower);
    if (it == kwmap.end()) {
        return LexItem(IDENT, lexeme, linenum);
    }

    // TRUE and FALSE are reserved words but behave as boolean constants
    if (it->second == TRUE || it->second == FALSE) {
        return LexItem(BCONST, lower, linenum);
    }
    return LexItem(it->second, lexeme, linenum);
}

// Reads the digits of an integer or float constant. The first digit has
// already been consumed and is in lexeme.
static LexItem readNumber(istream& in, string lexeme, int linenum) {
    bool isFloat = false;

    while (isdigit(in.peek())) {
        lexeme += (char)in.get();
    }

    // A '.' only belongs to the number when a digit follows it, so that
    // ranges like 1..10 lex as ICONST DOT DOT ICONST.
    if (in.peek() == '.') {
        in.get();
        if (isdigit(in.peek())) {
            isFloat = true;
            lexeme += '.';
            while (isdigit(in.peek())) {
                lexeme += (char)in.get();
            }
            if (in.peek() == '.') {
                lexeme += (char)in.get();
                return LexItem(ERR, lexeme, linenum);
            }
        }
        else {
            in.putback('.');
        }
    }

    // optional exponent: E[+|-]digits
    if (in.peek() == 'e' || in.peek() == 'E') {
        lexeme += (char)in.get();
        if (in.peek() == '+' || in.peek() == '-') {
            lexeme += (char)in.get();
        }
        if (!isdigit(in.peek())) {
            return LexItem(ERR, lexeme, linenum);
        }
        while (isdigit(in.peek())) {
            lexeme += (char)in.get();
        }
    }

    return LexItem(isFloat ? FCONST : ICONST, lexeme, linenum);
}

// IDENT ::= Letter { ( Letter | Digit | _Letter | _Digit ) }
static LexItem readIdent(istream& in, string lexeme, int linenum) {
    while (true) {
        int ch = in.peek();
        if (isalnum(ch)) {
            lexeme += (char)in.get();
        }
        else if (ch == '_') {
            lexeme += (char)in.get();
            if (!isalnum(in.peek())) {
                return LexItem(ERR, lexeme, linenum);
            }
        }
        else {
            break;
        }
    }
    return id_or_kw(lexeme, linenum);
}

LexItem getNextToken(istream& in, int& linenum) {
    int ch;

    // skip whitespace and comments
    while (true) {
        ch = in.get();
        if (ch == EOF) {
            return LexItem(DONE, "", linenum);
        }
        if (ch == '\n') {
            linenum++;
            continue;
        }
        if (isspace(ch)) {
            continue;
        }
        if (ch == '-' && in.peek() == '-') {
            while ((ch = in.get()) != EOF && ch != '\n')
                ;
            if (ch == '\n') {
                linenum++;
            }
            continue;
        }
        break;
    }

    string lexeme(1, (char)ch);

    if (isalpha(ch)) {
        return readIdent(in, lexeme, linenum);
    }
    if (isdigit(ch)) {
        return readNumber(in, lexeme, linenum);
    }

    switch (ch) {
    case '"': {
        string str;
        while ((ch = in.get()) != '"') {
            if (ch == EOF || ch == '\n') {
                if (ch == '\n') {
                    in.putback('\n');
                }
                return LexItem(ERR, " Invalid string constant \"" + str, linenum);
            }
            str += (char)ch;
        }
        return LexItem(SCONST, str, linenum);
    }
    case '\'': {
        ch = in.get();
        if (ch == EOF || ch == '\n' || in.peek() != '\'') {
            if (ch == '\n') {
                in.putback('\n');
            }
            return LexItem(ERR, " Invalid character constant '"
                           + (ch == EOF ? string() : string(1, (char)ch)) + "'", linenum);
        }
        in.get();
        return LexItem(CCONST, string(1, (char)ch), linenum);
    }
    case '+': return LexItem(PLUS, lexeme, linenum);
    case '-': return LexItem(MINUS, lexeme, linenum);
    case '&': return LexItem(CONCAT, lexeme, linenum);
    case '=': return LexItem(EQ, lexeme, linenum);
    case '(': return LexItem(LPAREN, lexeme, linenum);
    case ')': return LexItem(RPAREN, lexeme, linenum);
    case ',': return LexItem(COMMA, lexeme, linenum);
    case ';': return LexItem(SEMICOL, lexeme, linenum);
    case '.': return LexItem(DOT, lexeme, linenum);
    case '*':
        if (in.peek() == '*') {
            in.get();
            return LexItem(EXP, "**", linenum);
        }
        return LexItem(MULT, lexeme, linenum);
    case '/':
        if (in.peek() == '=') {
            in.get();
            return LexItem(NEQ, "/=", linenum);
        }
        return LexItem(DIV, lexeme, linenum);
    case ':':
        if (in.peek() == '=') {
            in.get();
            return LexItem(ASSOP, ":=", linenum);
        }
        return LexItem(COLON, lexeme, linenum);
    case '<':
        if (in.peek() == '=') {
            in.get();
            return LexItem(LTE, "<=", linenum);
        }
        return LexItem(LTHAN, lexeme, linenum);
    case '>':
        if (in.peek() == '=') {
            in.get();
            return LexItem(GTE, ">=", linenum);
        }
        return LexItem(GTHAN, lexeme, linenum);
    default:
        return LexItem(ERR, lexeme, linenum);
    }
}

/*
 * val.h
 *
 * Runtime values for the SADAL interpreter. Operators return an error
 * value (VERR) when applied to operands of illegal types.
 */

#ifndef VALUE_H_
#define VALUE_H_

#include <iostream>
#include <string>

using namespace std;

enum ValType { VINT, VREAL, VSTRING, VCHAR, VBOOL, VERR };

class Value {
    ValType T;
    bool    Btemp;
    int     Itemp;
    double  Rtemp;
    string  Stemp;
    char    Ctemp;

public:
    Value()               : T(VERR), Btemp(false), Itemp(0), Rtemp(0.0), Ctemp(0) {}
    Value(bool vb)        : T(VBOOL), Btemp(vb), Itemp(0), Rtemp(0.0), Ctemp(0) {}
    Value(int vi)         : T(VINT), Btemp(false), Itemp(vi), Rtemp(0.0), Ctemp(0) {}
    Value(double vr)      : T(VREAL), Btemp(false), Itemp(0), Rtemp(vr), Ctemp(0) {}
    Value(string vs)      : T(VSTRING), Btemp(false), Itemp(0), Rtemp(0.0), Stemp(vs), Ctemp(0) {}
    Value(char vc)        : T(VCHAR), Btemp(false), Itemp(0), Rtemp(0.0), Ctemp(vc) {}

    ValType GetType() const { return T; }
    bool IsErr() const { return T == VERR; }
    bool IsInt() const { return T == VINT; }
    bool IsReal() const { return T == VREAL; }
    bool IsString() const { return T == VSTRING; }
    bool IsChar() const { return T == VCHAR; }
    bool IsBool() const { return T == VBOOL; }
    bool IsNumeric() const { return T == VINT || T == VREAL; }

    int    GetInt() const { return Itemp; }
    double GetReal() const { return Rtemp; }
    string GetString() const { return Stemp; }
    char   GetChar() const { return Ctemp; }
    bool   GetBool() const { return Btemp; }

    // arithmetic: operands must both be INTEGER or both be FLOAT
    Value operator+(const Value& op) const;
    Value operator-(const Value& op) const;
    Value operator*(const Value& op) const;
    Value operator/(const Value& op) const;
    Value operator%(const Value& op) const;   // MOD, INTEGER only
    Value operator-() const;                  // unary minus

    // relational: operands must have the same type
    Value operator==(const Value& op) const;
    Value operator!=(const Value& op) const;
    Value operator<(const Value& op) const;
    Value operator<=(const Value& op) const;
    Value operator>(const Value& op) const;
    Value operator>=(const Value& op) const;

    // logical: BOOLEAN only
    Value operator&&(const Value& op) const;
    Value operator||(const Value& op) const;
    Value operator!() const;

    // STRING/CHARACTER & STRING/CHARACTER yields STRING
    Value Concat(const Value& op) const;

    // INTEGER ** INTEGER (non-negative exponent) or FLOAT ** INTEGER
    Value Exp(const Value& op) const;

    friend ostream& operator<<(ostream& out, const Value& v);
};

#endif /* VALUE_H_ */

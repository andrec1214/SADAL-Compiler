/*
 * val.cpp
 *
 * Operator implementations for SADAL runtime values.
 */

#include <cmath>
#include <sstream>
#include <iomanip>
#include "val.h"

Value Value::operator+(const Value& op) const {
    if (IsInt() && op.IsInt()) return Value(Itemp + op.Itemp);
    if (IsReal() && op.IsReal()) return Value(Rtemp + op.Rtemp);
    return Value();
}

Value Value::operator-(const Value& op) const {
    if (IsInt() && op.IsInt()) return Value(Itemp - op.Itemp);
    if (IsReal() && op.IsReal()) return Value(Rtemp - op.Rtemp);
    return Value();
}

Value Value::operator*(const Value& op) const {
    if (IsInt() && op.IsInt()) return Value(Itemp * op.Itemp);
    if (IsReal() && op.IsReal()) return Value(Rtemp * op.Rtemp);
    return Value();
}

// The caller is responsible for rejecting division by zero.
Value Value::operator/(const Value& op) const {
    if (IsInt() && op.IsInt()) return Value(Itemp / op.Itemp);
    if (IsReal() && op.IsReal()) return Value(Rtemp / op.Rtemp);
    return Value();
}

// Ada's MOD takes the sign of the divisor, unlike C++'s %.
Value Value::operator%(const Value& op) const {
    if (IsInt() && op.IsInt()) {
        int r = Itemp % op.Itemp;
        if (r != 0 && ((r < 0) != (op.Itemp < 0))) {
            r += op.Itemp;
        }
        return Value(r);
    }
    return Value();
}

Value Value::operator-() const {
    if (IsInt()) return Value(-Itemp);
    if (IsReal()) return Value(-Rtemp);
    return Value();
}

Value Value::operator==(const Value& op) const {
    if (T != op.T) return Value();
    switch (T) {
    case VINT:    return Value(Itemp == op.Itemp);
    case VREAL:   return Value(Rtemp == op.Rtemp);
    case VSTRING: return Value(Stemp == op.Stemp);
    case VCHAR:   return Value(Ctemp == op.Ctemp);
    case VBOOL:   return Value(Btemp == op.Btemp);
    default:      return Value();
    }
}

Value Value::operator!=(const Value& op) const {
    Value eq = *this == op;
    return eq.IsErr() ? eq : Value(!eq.Btemp);
}

Value Value::operator<(const Value& op) const {
    if (T != op.T) return Value();
    switch (T) {
    case VINT:    return Value(Itemp < op.Itemp);
    case VREAL:   return Value(Rtemp < op.Rtemp);
    case VSTRING: return Value(Stemp < op.Stemp);
    case VCHAR:   return Value(Ctemp < op.Ctemp);
    default:      return Value();
    }
}

Value Value::operator<=(const Value& op) const {
    Value gt = *this > op;
    return gt.IsErr() ? gt : Value(!gt.Btemp);
}

Value Value::operator>(const Value& op) const {
    return op < *this;
}

Value Value::operator>=(const Value& op) const {
    Value lt = *this < op;
    return lt.IsErr() ? lt : Value(!lt.Btemp);
}

Value Value::operator&&(const Value& op) const {
    if (IsBool() && op.IsBool()) return Value(Btemp && op.Btemp);
    return Value();
}

Value Value::operator||(const Value& op) const {
    if (IsBool() && op.IsBool()) return Value(Btemp || op.Btemp);
    return Value();
}

Value Value::operator!() const {
    if (IsBool()) return Value(!Btemp);
    return Value();
}

Value Value::Concat(const Value& op) const {
    if ((!IsString() && !IsChar()) || (!op.IsString() && !op.IsChar())) {
        return Value();
    }
    string lhs = IsString() ? Stemp : string(1, Ctemp);
    string rhs = op.IsString() ? op.Stemp : string(1, op.Ctemp);
    return Value(lhs + rhs);
}

Value Value::Exp(const Value& op) const {
    if (!op.IsInt()) return Value();

    if (IsInt()) {
        if (op.Itemp < 0) return Value();
        int result = 1;
        for (int i = 0; i < op.Itemp; i++) {
            result *= Itemp;
        }
        return Value(result);
    }
    if (IsReal()) {
        return Value(pow(Rtemp, op.Itemp));
    }
    return Value();
}

ostream& operator<<(ostream& out, const Value& v) {
    switch (v.T) {
    case VINT:
        out << v.Itemp;
        break;
    case VREAL: {
        // always show a decimal point so FLOAT output is distinguishable
        ostringstream ss;
        ss << setprecision(15) << v.Rtemp;
        string s = ss.str();
        if (s.find_first_of(".einn") == string::npos) {
            s += ".0";
        }
        out << s;
        break;
    }
    case VSTRING:
        out << v.Stemp;
        break;
    case VCHAR:
        out << v.Ctemp;
        break;
    case VBOOL:
        out << (v.Btemp ? "true" : "false");
        break;
    case VERR:
        out << "ERROR";
        break;
    }
    return out;
}

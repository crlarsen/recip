//
//  hp.cpp
//  hp
//
//  Created by Chris Larsen on 8/1/22.
//
// THIS IS NOT PRODUCTION QUALITY CODE!!! IT IS A QUICK AND DIRTY HACK!!!
//
// This code implements a class which supports IEEE 754 binary16 operations, but
// just barely.
//
// There is only enough functionality here to be able to write a binary16
// implementation of finding the reciprocal of a number using Newton's Method.
// Even then, the Newton's Method algorithm isn't supported for the entire range
// of binary16 Normal numbers; Subnormal number support might make this possible.
//
// Little, or no, attempt is made to deal with overflow, underflow, rounding, etc.
//
// The code assumes all inputs are Normal numbers. Some limited support for Subnormal,
// and Infinite results has been added to the addition/subtraction logic.
//
// The addition/subtraction logic also has some minimal support for sums/differences
// which have a result of Zero.
//
// Operations currently supported:
// o Construction of binary16 values from unsigned short, float, and double values.
// o Unary Minus
// o The standard C++ operator <<
// o Arithmetic operators +=, +, -=, -, *=, *
// o Extracting the bits representing the binary16 value as a short.
//
// See the corresponding header file (hp.hpp) for TODO list.
//

#include "hp.hpp"

RoundingAttribute hp::ra = roundTiesToEven;

// Analogous to Java's java.lang.float.floatToRawIntBits but
// for binary16 numbers.
const short binary16ToRawShortBits(const hp f) {
    return f.datum;
}

const hp shortBitsToBinary16(const short bits) {
    hp a(bits);
    return a;
}

std::ostream &operator<<(std::ostream &os, const hp &rhs)
{
    if (rhs.ieee754class() == SNAN) {
        os << "sNaN";
    } else if (rhs.ieee754class() == QNAN) {
        os << "qNaN";
    } else if (rhs.ieee754class() == INFINITY) {
        const unsigned int iSign = rhs.datum & SIGNMASK;
        os << (iSign ? '-' : '+') << "Infinity";
    } else if (rhs.ieee754class() == ZERO) {
        const unsigned int iSign = rhs.datum & SIGNMASK;
        if (iSign) os << '-';
        os << 0.0f;
    } else {
        // Extract sign, exponent, and significand fields from rhs:
        const unsigned int iSign = rhs.datum & SIGNMASK;
        const int iExp = rhs.extractExp();
        const int iSig = rhs.extractSig() & SIGMASK;
        // Reconfigure sign, exponent, and significand fields into IEEE 754
        // binary32 value:
        const unsigned int oSign = iSign << (spNEXP+spNSIG-hpNEXP-hpNSIG);
        const int oExp = (iExp + spBIAS) << spNSIG;
        const int oSig = iSig << (spNSIG-hpNSIG);
        const int i = oSign | oExp | oSig;
        // Reinterpret integer representing binary32 value as float:
        const float& f = reinterpret_cast<const float&>(i);
        
        // Write the value, finally!
        os << f;
    }

    return os;
}

const short logB(const hp& f) {
    return f.logB();
}

const hp scaleB(const hp& f, const short N) {
    return f.scaleB(N);
}

//
//  hp.hpp
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
//
// Operations currently supported:
// o Construction of binary16 values from unsigned short, float, and double values.
// o Unary Minus
// o The standard C++ output operator <<
// o Arithmetic operators +=, +, -=, -, *=, *
// o Extracting the bits representing the binary16 value as a short.
//
// TODO: Add more support for constructing values from integers.
//

#ifndef hp_hpp
#define hp_hpp

#include <iostream>

const auto hpNEXP =  5;
const auto hpNSIG = 10;
const auto hpBIAS = ((1 << (hpNEXP-1)) - 1);
const auto hpEMAX = hpBIAS;
const auto hpEMIN = 1 - hpBIAS;
const auto SIGNMASK = (1 << (hpNEXP+hpNSIG));
const auto EXPMASK = ((1 << hpNEXP) - 1);
const auto SIGMASK = ((1 << hpNSIG) - 1);

const auto spNEXP =  8;
const auto spNSIG = 23;
const auto spBIAS = ((1 << (spNEXP-1)) - 1);
const auto spSIGNMASK = (1 << (spNEXP+spNSIG));
const auto spEXPMASK = ((1 << spNEXP) - 1);
const auto spSIGMASK = ((1 << spNSIG) - 1);

const auto dpNEXP = 11;
const auto dpNSIG = 52;
const auto dpBIAS = ((1 << (dpNEXP-1)) - 1);
const long dpSIGNMASK = (1l << (dpNEXP+dpNSIG));
const auto dpEXPMASK = ((1l << dpNEXP) - 1);
const auto dpSIGMASK = ((1l << dpNSIG) - 1);

//localparam NORMAL    = 0;
//localparam SUBNORMAL = 1;
//localparam ZERO      = 2;
//localparam INFINITY  = 3;
//localparam QNAN      = 4;
//localparam SNAN      = 5;
//localparam NTYPES    = SNAN + 1;

typedef enum : unsigned char {
    NORMAL    = (1 << 0),
    SUBNORMAL = (1 << 1),
    ZERO      = (1 << 2),
    INFINITY  = (1 << 3),
    QNAN      = (1 << 4),
    SNAN      = (1 << 5),
    NTYPES    = (1 << 6)
} IEEE754Class;

typedef enum : unsigned char {
    roundTiesToEven     = (1 << 0),
    roundTowardZero     = (1 << 1),
    roundTowardPositive = (1 << 2),
    roundTowardNegative = (1 << 3),
    roundTiesToAway     = (1 << 4)
} RoundingAttribute;

inline const int floatToRawIntBits(const float f) {
    const int& i = reinterpret_cast<const int&>(f);
    return i;
}

// C++ implementation of Java's java.lang.double.doubleToRawLongBits.
inline const long doubleToRawLongBits(const double d) {
    const long &l = reinterpret_cast<const long&>(d);
    return l;
}

class hp {
private:
    static RoundingAttribute ra;
    unsigned short datum;
    short extractExp() const {
        short fExp;
        int fSig;
        (void)fp_class(fExp, fSig);
        return fExp;
    }
    int extractSig() const {
        short fExp;
        int fSig;
        (void)fp_class(fExp, fSig);
        return fSig;
    }
    void round(const short negIn, const short expIn, const int sigIn,
               short& expOut, int& sigOut, bool& inexact) {
        bool lastKeptBitIsOdd;
        bool decidingBitIsOne;
        bool remainingBitsAreNonzero;
        int tSig;
        int subnormal = 1;
        
        const int INTn = 2*hpNSIG + 6;
        
        if (expIn < hpEMIN-hpNSIG-1) {
                // Is the last bit to be saved a `1', that is, is it odd?
                lastKeptBitIsOdd        =  false; // No bits are being kept.
                   
                // Is the first bit to be truncated a `1'?
                // Then we use the last bit being kept to break the tie
                // in choosing to round, or use the rest of the truncated
                // bits.
                decidingBitIsOne        =  false;
          
                // Are the bits beyond the first bit to be truncated all zero?
                // If not, we don't have a tie situation.
                remainingBitsAreNonzero = sigIn != 0;
                      
                tSig = 0;
        } else if (expIn < hpEMIN-hpNSIG) {
              // Is the last bit to be saved a `1', that is, is it odd?
              lastKeptBitIsOdd        =  false; // No bits are being kept.
                 
              // Is the first bit to be truncated a `1'?
              // Then we use the last bit being kept to break the tie
              // in choosing to round, or use the rest of the truncated
              // bits.
              decidingBitIsOne        =  (sigIn & (1 << (2*hpNSIG+5))) != 0;
        
              // Are the bits beyond the first bit to be truncated all zero?
              // If not, we don't have a tie situation.
              remainingBitsAreNonzero = (sigIn & ((1 << (2*hpNSIG+5)) - 1)) != 0;
                    
              tSig = 0;
        } else if (expIn < hpEMIN) {
            // Is the last bit to be saved a `1', that is, is it odd?
            lastKeptBitIsOdd        =  ((sigIn >> (INTn-hpNSIG+hpEMIN-expIn-1)) & 1) != 0;
                 
            // Is the first bit to be truncated a `1'?
            // Then we use the last bit being kept to break the tie
            // in choosing to round, or use the rest of the truncated
            // bits.
            decidingBitIsOne        =  ((sigIn >> (INTn-hpNSIG+hpEMIN-expIn-2)) & 1) != 0;

            // Are the bits beyond the first bit to be truncated all zero?
            // If not, we don't have a tie situation.
            remainingBitsAreNonzero = (sigIn & ((1 << (INTn-hpNSIG+hpEMIN-expIn-2)) - 1)) != 0;
              
            tSig = sigIn >> (INTn-hpNSIG+hpEMIN-expIn-1);
        } else {
            // Is the last bit to be saved a `1', that is, is it odd?
            lastKeptBitIsOdd = (sigIn >> (INTn-hpNSIG-1)) & 1;

            // Is the first bit to be truncated a `1'?
            // Then we use the last bit being kept to break the tie
            // in choosing to round, or use the rest of the truncated
            // bits.
            decidingBitIsOne = (sigIn >> (INTn-hpNSIG-2)) & 1;

            // Are the bits beyond the first bit to be truncated all zero?
            // If not, we don't have a tie situation.
            remainingBitsAreNonzero = (sigIn & ((1 << (INTn-hpNSIG-2)) - 1)) != 0;

            tSig = sigIn >> (INTn-hpNSIG-1);
            
            subnormal = 0;
        }

        int roundBit =
        ((ra&roundTiesToEven) && // First rounding case
          decidingBitIsOne && (lastKeptBitIsOdd || remainingBitsAreNonzero)) |
        ((ra&roundTowardPositive) && // Second rounding case
         ~negIn && (decidingBitIsOne || remainingBitsAreNonzero)) |
        ((ra&roundTowardNegative) && // Third rounding case
          negIn && (decidingBitIsOne || remainingBitsAreNonzero));
        
        int aSig = tSig + (subnormal&roundBit) + roundBit;
        
        int Cout = (aSig >> (hpNSIG+1)) & 1;
        
        // If there was a carry-out then the carry-out is the new most significant
        // bit set to 1 (one).
        sigOut = aSig >> Cout;
        
        // If when we rounded sigIn there was a carry-out we need to adjust the exponent
        // to re-normalize the result.
        expOut = expIn + Cout; // We're adding either 1 or 0 to expIn.

        inexact = decidingBitIsOne || remainingBitsAreNonzero;
    }
    inline short logB() const {
        short exp;
        int sig;
        IEEE754Class flags = fp_class(exp, sig);
        
        return (flags & (SUBNORMAL|NORMAL)) ? exp : 0x7fff;
    }
    inline hp scaleB(short N) const {
        short exp;
        int sig;
        IEEE754Class flags = fp_class(exp, sig);
        
        if (flags & (SUBNORMAL|NORMAL)) {
            exp += N;
            if ((exp < hpEMIN) || (hpEMAX < exp)) return hp((short)0xffff); // qNaN
            short d = datum & ~(EXPMASK << hpNSIG);
            exp += hpBIAS;
            d |= exp << hpNSIG;
            return hp(d);
        }
        
        return hp((short)0xffff);
    }
public:
    hp() : datum((EXPMASK << hpNSIG) | ('S')) {}
    hp(const short f) : datum(f) {}
    hp(const float f) {
        const int i = floatToRawIntBits(f);
        const int fSign = i & spSIGNMASK;
        const int fExp = ((i >> spNSIG) & spEXPMASK) - spBIAS;
        const int fSig = i & spSIGMASK;
        const short hSign = fSign >> ((spNEXP+spNSIG)-(hpNEXP+hpNSIG));
        const short hExp = (fExp + hpBIAS) << hpNSIG;
        const short hSig = fSig >> (spNSIG-hpNSIG);

        if (fExp >= hpBIAS) {
            // Make binary16 Infinity
            datum = hSign | (EXPMASK<<hpNSIG);
        } else if (fExp < (1-hpBIAS)) {
            // Make binary16 Zero
            datum = hSign;
        } else {
            // Make binary16 Normal number
            datum = hSign | hExp | hSig;
        }
    }
    hp(const double d) {
        const long l = doubleToRawLongBits(d);
        const long dSign = l & dpSIGNMASK;
        const long dExp = ((l >> dpNSIG) & dpEXPMASK) - dpBIAS;
        const long dSig = l & dpSIGMASK;
        const short hSign = dSign >> ((dpNEXP+dpNSIG)-(hpNEXP+hpNSIG));
        const short hExp = (dExp + hpBIAS) << hpNSIG;
        const short hSig = dSig >> (dpNSIG-hpNSIG);

        if (dExp >= hpBIAS) {
            // Make binary16 Infinity
            datum = hSign | (EXPMASK<<hpNSIG);
        } else if (dExp < (1-hpBIAS)) {
            // Make binary16 Zero
            datum = hSign;
        } else {
            // Make binary16 Normal number
            datum = hSign | hExp | hSig;
        }
    }
    inline IEEE754Class fp_class(short& fExp, int& fSig) const {
        fExp = (datum >> hpNSIG) & EXPMASK;
        fSig = datum & SIGMASK;
        
        const bool expOnes   = fExp == EXPMASK;
        const bool expZeroes = fExp == 0;
        const bool sigZeroes = fSig == 0;
        
        IEEE754Class fFlags;
        if (expOnes && !sigZeroes && !((datum >> (hpNSIG-1)) & 1)) {
            fFlags = SNAN;
        } else if (expOnes && ((datum >> (hpNSIG-1)) & 1)) {
            fFlags = QNAN;
        } else if (expOnes && sigZeroes) {
            fFlags = INFINITY;
        } else if (expZeroes && sigZeroes) {
            fFlags = ZERO;
        } else if (expZeroes && !sigZeroes) {
            fFlags = SUBNORMAL;
        } else if (!expOnes && !expZeroes) {
            fFlags = NORMAL;
        } else {
            fFlags = NTYPES;
        }

        int sa = 0;

        if (fFlags == NORMAL) {
            fExp -= hpBIAS;
            fSig |= 1 << hpNSIG;
        } else if (fFlags == SUBNORMAL) {
            // Shift the most significant bit into the position
            // of the Normal's implied 1. Keep track of how many
            // places were needed to shift the most significant
            // set bit so we can adjust the exponent when we're
            // done.
            for (int i = 3; i >= 0; i--) {
                int maskSize = 1 << i;
                int mask = ((1 << maskSize) - 1) << (hpNSIG+1-maskSize);
                if ((mask & fSig) == 0) {
                    fSig <<= maskSize;
                    sa |= maskSize;
                }
            }

            fExp = hpEMIN - sa;
        }
        
        return fFlags; // If this happens it's an error.
    }
    inline IEEE754Class ieee754class() const {
        short fExp;
        int fSig;
        
        return fp_class(fExp, fSig);
    }
    void setRoundingAttribute(RoundingAttribute r) {
        ra = r;
    }
    short binary16BitsToShort() {
        return (short)datum;
    }
    // Unary minus operator:
    inline hp operator-(void) const {
        return hp((short)(this->datum ^ SIGNMASK));
    }
    hp &operator+=(const hp &rhs) {
        const unsigned short aSign = this->datum & SIGNMASK,
                             bSign = rhs.datum & SIGNMASK;
        unsigned short sumSign;
        int shiftAmt;
        short aExp, bExp, adjExp;
        int aSig, bSig;
        
        IEEE754Class aFlags = this->fp_class(aExp, aSig);
        IEEE754Class bFlags = rhs.fp_class(bExp, bSig);
        IEEE754Class pFlags;
        
        if (aFlags == SNAN || bFlags == SNAN) {
            datum = (aFlags == SNAN) ? this->datum : rhs.datum;
            pFlags = SNAN;
        } else if (aFlags == QNAN || bFlags == QNAN) {
            datum = (aFlags == QNAN) ? this->datum : rhs.datum;
            pFlags = QNAN;
        } else if (aFlags == ZERO || bFlags == ZERO)  {
            if (aFlags == ZERO) {
                datum = rhs.datum;
                pFlags = bFlags;
            } else {
                pFlags = aFlags;
            }
        } else if (aFlags == INFINITY && bFlags == INFINITY) {
            datum = aSign | (EXPMASK << hpNSIG);
            pFlags = INFINITY;
        } else if (aFlags == INFINITY || bFlags == INFINITY) {
            pFlags = INFINITY;
            datum = (aFlags == INFINITY) ? this->datum : rhs.datum;
        } else {
            int augendSig = 0;
            int addendSig = 0;
            int na = 0;

            if (aExp < bExp){
                sumSign = bSign;
                shiftAmt = bExp - aExp;
                augendSig = bSig << (hpNSIG+3);
                addendSig = aSig << (hpNSIG+3);
                adjExp = bExp;
            } else {
                sumSign = aSign;
                shiftAmt = aExp - bExp;
                augendSig = aSig << (hpNSIG+3);
                addendSig = bSig << (hpNSIG+3);
                adjExp = aExp;
            }

            addendSig >>= (shiftAmt > hpNSIG+3) ? hpNSIG+3 : shiftAmt;
    
            const int sumSig = (aSign ^ bSign) ? augendSig - addendSig : augendSig + addendSig;
            const unsigned short absSign = (sumSig < 0) ? (sumSign ^ SIGNMASK) : (sumSign);
            const int absSig = abs(sumSig);
    
            const int Cout = absSig >> (2*hpNSIG+4);
            const int bigSig = absSig >> Cout;
            const unsigned short bigExp = adjExp + Cout;
    
            int normSig = bigSig;
            bool zero = true;
            for (int i=3; i >= 0; i--) {
                const int maskSize = 1 << i;
                const int mask = ((1 << maskSize) - 1) << (2*hpNSIG+4-maskSize);
                if (normSig & mask) {
                    zero = false;
                } else {
                    normSig <<= maskSize;
                    na |= maskSize;
                }
            }
    
            short normExp = bigExp - na;
    
            bool inexact;
            round(absSign, normExp, normSig << 2, normExp, normSig, inexact);
            
            if (!normSig) {
                // Construct Zero.
                this->datum = absSign;
            } else if (normExp < hpEMIN) {
                // Construct Subnormal Number.
                this->datum = absSign | normSig;
            } else if (normExp > hpEMAX) {
                // Construct Infinity
                this->datum = absSign | (EXPMASK << hpNSIG);
            } else {
                this->datum = absSign |
                              ((normExp + hpBIAS) << hpNSIG) |
                              (normSig & SIGMASK);
            }
        }
        
        return *this;
    }
    const hp operator+(const hp &rhs) const {
        return hp(*this) += rhs;
    }
    hp &operator-=(const hp &rhs) {
        return *this = *this + -rhs;
    }
    const hp operator-(const hp &rhs) const {
        return hp(*this) -= rhs;
    }
    inline hp &operator*=(const hp &rhs) {
        unsigned short pSign = (this->datum ^ rhs.datum) & SIGNMASK;
        short aExp, bExp;
        int aSig, bSig;
        
        IEEE754Class aFlags = this->fp_class(aExp, aSig);
        IEEE754Class bFlags = rhs.fp_class(bExp, bSig);
        IEEE754Class pFlags;
        
        if (aFlags == SNAN || bFlags == SNAN) {
            datum = (aFlags == SNAN) ? this->datum : rhs.datum;
            pFlags = SNAN;
        } else if (aFlags == QNAN || bFlags == QNAN) {
            datum = (aFlags == QNAN) ? this->datum : rhs.datum;
            pFlags = QNAN;
        } else if (aFlags == INFINITY || bFlags == INFINITY) {
            if (aFlags == ZERO || bFlags == ZERO) {
                datum = pSign | (EXPMASK << hpNSIG) | (1 << (NSIG-1)); // qNaN
                pFlags = QNAN;
            } else {
                datum = pSign | (EXPMASK << hpNSIG);
                pFlags = INFINITY;
            }
        } else if ((aFlags == ZERO || bFlags == ZERO) ||
                   (aFlags == SUBNORMAL && bFlags == SUBNORMAL))  {
            datum = pSign;
            pFlags = ZERO;
        } else {
            short pExp = aExp + bExp;
            int pSig = aSig * bSig;
            
            if (pSig & (1 << (2*hpNSIG+1))) {
                pExp++;
            } else {
                pSig <<= 1;
            }
            
            bool inexact;
            round(pSign, pExp, pSig << 4, pExp, pSig, inexact);
            
            if (pExp < (hpEMIN - hpNSIG)) {
                datum = pSign; // pExp & pSig must both be zero.
                pFlags = ZERO;
            } else if (pExp < hpEMIN) {
                // Remember that we can only store NSIG bits
                datum = pSign | (pSig & SIGMASK);
                pFlags = SUBNORMAL;
            } else if (pExp > hpEMAX) {
                datum = pSign | (EXPMASK << hpNSIG);
                pFlags = INFINITY;
            } else {
                pFlags = NORMAL;
                datum = pSign |
                        ((pExp + hpBIAS) << hpNSIG) |
                        (pSig & SIGMASK);
            }
        }
   
        return *this;
    }
    inline hp operator*(const hp& rhs) const {
        return hp(*this) *= rhs;
    };
    inline bool operator!=(const hp rhs) const {
        return this->datum != rhs.datum;
    }

    friend const short binary16ToRawShortBits(const hp);
    friend const hp shortBitsToBinary16(short bits);
    friend std::ostream &operator<<(std::ostream &, const hp &);
    friend const short logB(const hp&);
    friend const hp scaleB(const hp&, const short);
};

#endif /* hp_hpp */

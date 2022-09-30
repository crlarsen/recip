//
//  main.cpp
//  recip
//
// Use Newton's Method to compute the reciprocal for using class hp,
// the IEEE 754 binary16 data type.
//
// Neither this main routine nor the hp class are production quality
// code.
//
//  Created by Chris Larsen on 5/28/22.
//

#include <iostream>
#include <vector>
#include <array>
#include <ranges>

#include "hp.hpp"

hp recip_lut(const hp x) {
    const short lut[] = {0x0, 0xF, 0xF, 0xF, 0xF, 0xE, 0xE, 0xE,
        0xE, 0xD, 0xD, 0xD, 0xD, 0xD, 0xC, 0xC,
        0xC, 0xC, 0xC, 0xB, 0xB, 0xB, 0xB, 0xB,
        0xA, 0xA, 0xA, 0xA, 0xA, 0xA, 0x9, 0x9,
        0x9, 0x9, 0x9, 0x9, 0x8, 0x8, 0x8, 0x8,
        0x8, 0x8, 0x8, 0x7, 0x7, 0x7, 0x7, 0x7,
        0x7, 0x7, 0x7, 0x6, 0x6, 0x6, 0x6, 0x6,
        0x6, 0x6, 0x6, 0x5, 0x5, 0x5, 0x5, 0x5,
        0x5, 0x5, 0x5, 0x5, 0x4, 0x4, 0x4, 0x4,
        0x4, 0x4, 0x4, 0x4, 0x4, 0x3, 0x3, 0x3,
        0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
        0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
        0x2, 0x2, 0x2, 0x2, 0x1, 0x1, 0x1, 0x1,
        0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
        0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    
    const int &i = binary16ToRawShortBits(x);
    
    const unsigned short sign = i & (1 << (hpNEXP+hpNSIG)); // Preserve sign bit.
    const unsigned short sigIn = (i >> (hpNSIG-7)) & 0x7f; // Mask off 7 MSBs of
                                                           // significand field
    const unsigned short sigOut = lut[sigIn];
    const short expIn = -(((i >> hpNSIG) & EXPMASK) - hpBIAS); // Compute unbiased
                                                                        // exponent.
    int expOut = expIn - ((sigIn == 0) ? 0 : 1) + hpBIAS;
    
    short g = sign | (expOut << hpNSIG) | (sigOut << (hpNSIG-4));
        
    return hp(g);
}

int main(int argc, const char * argv[]) {
    hp a, b, p, s;
    
    hp two(2.0);
    
    int failed = 0;
    // You should be asking yourself why the loop terminates at 0x746f
    // instead of 0x7C00.
    // TODO: For completeness there should be a loop which computes the
    // reciprocal of negative numbers.
    // TODO: Add test to see if the product of D and x4 is 1 within the
    // precision of the binary16 data type. Maybe the product is precise
    // to 9 bits?
    for (short i = 0x0400; i < 0x746f; i++) {
        hp D(i);
        hp x0 = recip_lut(D);
        hp x1 = x0 * (two - D*x0);
        hp x2 = x1 * (two - D*x1);
        hp x3 = x2 * (two - D*x2);
        hp x4 = x3 * (two - D*x3);
        if ((x3 != x4) && (x2 != x4)) {
            failed++;
            std::cout << "D = " << D
                      << " didn't converge in 3 iterations.\n";
            std::cout << x0 << ' ' << x1 << ' ' << x2 << ' '
                      << x3 << ' ' << x4 << std::endl;
            std::cout << D*x0 << ' ' << D*x1 << ' ' << D*x2 << ' '
                      << D*x3 << ' ' << D*x4 << std::endl;
        }
    }
    
    // For information about the following see the video "Building an
    // FPU In Verilog: Floating Point Division, Part 3".
    // https://youtu.be/B4V3AAC06SI
    std::cout << "Better reciprocal: " <<  hp((short)0x724D) << " (" << std::hex << binary16ToRawShortBits(hp((short)0x724D)) << ")" << std::endl;
    std::cout << hp((short)0x724D) * hp((short)1300) << " (" << binary16ToRawShortBits(hp((short)0x724D) * hp((short)1300)) << ")" <<std::endl;
    
    return failed;
}

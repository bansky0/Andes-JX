/*
  ==============================================================================

    NoiseGenerator.h
    Created: 12 Nov 2025 1:55:23pm
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once

#include <cstring> // Para memcpy
//#include <bit>     // Para std::bit_cast (C++20)

class NoiseGenerator
{
public:
    void reset()
    {
        noiseSeed = 22222;
    }

    float nextValue()
    {
        // 1. Lineal Congruential Generator - LCG Step
        noiseSeed = noiseSeed * 196314165 + 907633515;

        // 2. Construct float using bits
        unsigned int r = (noiseSeed & 0x7FFFFF) + 0x40000000;

        // 3. Safe convertion (Choose A o B)

        // OPTION A: C++20 (clean and modern)
        // float noise = std::bit_cast<float>(r);

        // OPTION B: Compatible with old C++ (C++98/11/14/17)
        float noise;
        std::memcpy(&noise, &r, sizeof(float));

        //4. Range [-1, 1]
        return noise - 3.0f;
    }

private:
    unsigned int noiseSeed = 22222;
};

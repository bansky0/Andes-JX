/*
  ==============================================================================

    SVF.h
    Created: 5 Feb 2026 2:21:41pm
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once
#include <cmath>
#include "Constants.h"
#include <algorithm>

class SVF
{
public:
    void prepare(float sr)
    {
        sampleRate = sr;
        reset();
    }
    void updateCoefficients(float cutoff, float Q)
    {
        cutoff = std::clamp(cutoff, 20.0f, 0.45f * sampleRate);
        Q = std::clamp(Q, 0.707f, 10.0f);

        g = std::tan(PI * cutoff / sampleRate);
        k = 1.0f / Q;
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }
    void reset()
    {
        ic1eq = 0.0f;
        ic2eq = 0.0f;
    }
    float render(float x)
    {
        float v3 = x - ic2eq;
        float v1 = a1 * ic1eq + a2 * v3;
        float v2 = ic2eq + a2 * ic1eq + a3 * v3;

        ic1eq = 2.0f * v1 - ic1eq;
        ic2eq = 2.0f * v2 - ic2eq;

        return v2;
    }
private:
    float sampleRate = 48000.0f;
    float g = 0.0f, k = 0.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
    float ic1eq = 0.0f, ic2eq = 0.0f;
};
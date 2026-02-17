/*
  ==============================================================================

    LadderFilter.h
    Created: 11 Feb 2026 2:46:22pm
    Author:  Jhonatan

    Moog Ladder Filter based on Antti Huovilainen's DAFx 2004 paper:
    "Non-Linear Digital Implementation of the Moog Ladder Filter"
    https://dafx.de/paper-archive/2004/P_061.PDF

    Features:
    - 4-pole (24 dB/oct) lowpass with self-oscillation
    - Frequency-dependent resonance compensation
    - 2x oversampling with linear interpolation
    - Per-stage soft saturation (tanh)

  ==============================================================================
*/

#pragma once

#include <cmath>
#include <algorithm>

class LadderFilter
{
public:
    LadderFilter() = default;

    void reset()
    {
        std::fill(std::begin(y), std::end(y), 0.0f);
        std::fill(std::begin(tanhY), std::end(tanhY), 0.0f);
        x1 = 0.0f;
    }

    void setSampleRate(float newSampleRate)
    {
        sampleRate = std::max(1.0f, newSampleRate);
        updateCoefficients(cutoffHz, resonance);
    }

    void updateCoefficients(float cutoffFreq, float reso01)
    {
        cutoffHz = std::clamp(cutoffFreq, 20.0f, sampleRate * 0.45f);
        resonance = std::clamp(reso01, 0.0f, 1.0f);  // 0.0 = no resonance, 1.0 = self-oscillation

        // Oversampled frequency
        const float fs_os = sampleRate * 2.0f;
        float f = cutoffHz / fs_os;
        f = std::clamp(f, 0.0f, 0.49f);

        // Pre-warp frequency (Huovilainen eq. 13)
        p = f * (1.8f - 0.8f * f);
        k = 2.0f * p - 1.0f;

        // Frequency-dependent resonance compensation (eq. 15)
        float t = (1.0f - p) * 1.386249f;
        float t2 = 12.0f + t * t;
        r = resonance * (t2 + 6.0f * t) / (t2 - 6.0f * t);

        g = std::clamp(p, 0.0f, 0.9995f);
    }

    float render(float x)
    {
        // 2x oversampling with linear interpolation
        float x_os0 = 0.5f * (x1 + x);  // Midpoint
        float x_os1 = x;                // Endpoint

        float y_os0 = processOneSubstep(x_os0);
        float y_os1 = processOneSubstep(x_os1);

        x1 = x;

        return 0.5f * (y_os0 + y_os1);
    }

    const char* getFilterType() const
    {
        return "Moog Ladder (Huovilainen)";
    }

private:
    // Fast tanh approximation (rational function)
    static inline float tanh_approx(float x)
    {
        x = std::clamp(x, -5.0f, 5.0f);
        const float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    float processOneSubstep(float x)
    {
        // Feedback from output with saturation
        const float fb = r * tanhY[3];
        float u = x - fb;
        float in = tanh_approx(u);

        // Process through 4 one-pole stages
        for (int i = 0; i < 4; ++i)
        {
            y[i] += g * (in - y[i]);      // One-pole integration
            tanhY[i] = tanh_approx(y[i]); // Soft saturation
            in = tanhY[i];                // Output becomes next input
        }

        return y[3];
    }

    // State
    float sampleRate = 44100.0f;
    float cutoffHz = 1000.0f;
    float resonance = 0.0f;

    float y[4] = { 0.0f, 0.0f, 0.0f, 0.0f };      // Stage outputs
    float tanhY[4] = { 0.0f, 0.0f, 0.0f, 0.0f };  // Saturated outputs
    float x1 = 0.0f;                            // Previous input (for interpolation)

    // Coefficients (Huovilainen model)
    float g = 0.0f;  // Integration coefficient
    float p = 0.0f;  // Pre-warped frequency
    float k = 0.0f;  // Correction factor
    float r = 0.0f;  // Resonance feedback gain
};
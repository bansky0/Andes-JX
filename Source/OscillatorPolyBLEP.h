/*
  ==============================================================================

    OscillatorPolyBLEEP.h
    Created: 21 Nov 2025 11:12:13am
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once
#include <cmath>
#include "Constants.h"
#include <algorithm>

class OscillatorPolyBLEP
{
public:
    void prepare(double sampleRate);
    void setFrequency(float freq);
    void setPulseWidth(float width)
    {
        // Limitar entre 0.05 y 0.95 para evitar problemas num	ricos
        pulseWidth = std::clamp(width, 0.05f, 0.95f);
    }

    void reset() { 
        phase = 0.f; 
        integrator = 0.f;
    }

    void syncPhase(const OscillatorPolyBLEP& other);

    float sine();
    float saw();
    float square();
    float squarePWM();
    float triangle();
    float nextSample();

private:
    float sampleRate = 48000.f;
    float phase = 0.0f;
    float phaseInc = 0.0f;
    float frequency = 440.f;
    float integrator = 0.0f;
    float pulseWidth = 0.5f;

    float polyBLEP(float t) const;
    float advance();
};
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

class OscillatorPolyBLEP
{
public:
    void prepare(double sampleRate);
    void setFrequency(float freq);
    
    float nextSample();
    void reset() { 
        phase = 0.f; 
        integrator = 0.f;
    }
    
    float saw();
    float square();
    float triangle();

private:
    float sampleRate = 48000.f;
    float phase = 0.0f;
    float frequency = 440.f;
    float integrator = 0.0f;

    float polyBLEP(float t) const;
    float advance();
};
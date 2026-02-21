/*
  ==============================================================================

    Oscillator.h
    Created: 15 Nov 2025 4:28:29pm
    Author:  Valeria

  ==============================================================================
*/

#pragma once
#include "OscillatorPolyBLEP.h"

enum class WaveType
{
    Sine,
    Saw,
    Square,
    SquarePWM,
    Triangle
};

class Oscillator
{
public:
    float amplitude = 1.0f;

    void prepare(double sampleRate)
    {
        polyblep.prepare(sampleRate);
    }

    void setFrequency(float freq)
    {
        polyblep.setFrequency(freq);
    }

    void setWaveType(WaveType type)
    {
        waveType = type;
    }

    void setPulseWidth(float width)
    {
        polyblep.setPulseWidth(width);
    }

    void syncPhase(const Oscillator& other)
    {
        polyblep.syncPhase(other.polyblep);
    }

    float nextSample()
    {
        switch (waveType)
        {
            case WaveType::Sine:        return polyblep.sine();
            case WaveType::Saw:         return polyblep.saw();
            case WaveType::Square:      return polyblep.square();
            case WaveType::SquarePWM:   return polyblep.squarePWM();
            case WaveType::Triangle:    return polyblep.triangle();
        }
        return 0.0f;
    }

    void reset()
    {
        polyblep.reset();
    }


private:
    WaveType waveType = WaveType::Saw;
    OscillatorPolyBLEP polyblep;
};
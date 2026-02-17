/*
  ==============================================================================

    OscillatorPolyBLEP.cpp
    Created: 21 Nov 2025 11:39:49am
    Author:  Jhonatan

  ==============================================================================
*/

#include "OscillatorPolyBLEP.h"
#include <cmath>

//==============================================================================
// Helpers
//==============================================================================

float OscillatorPolyBLEP::advance()
{
    phase += frequency / sampleRate;

    if (phase >= 1.0f) {
        phase -= 1.0f;
    }
    return phase;
}

float OscillatorPolyBLEP::polyBLEP(float t) const
{   
    float dt = frequency / sampleRate;
    dt = std::min(dt, 0.5f);

    // rising edge
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    // falling edge
    else if (t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }

    return 0.0f;
    
}

//==============================================================================
// Public API
//==============================================================================

void OscillatorPolyBLEP::prepare(double newSampleRate)
{
    sampleRate = (float)newSampleRate;
    phase = 0.0f;
    integrator = 0.0f;
}

void OscillatorPolyBLEP::setFrequency(float newFreq)
{
    frequency = newFreq;
}

//==============================================================================
// Waveforms
//==============================================================================

float OscillatorPolyBLEP::sine()
{
    float out = std::sin(TWO_PI * phase);
    advance();
    return out;
}

float OscillatorPolyBLEP::saw()
{
    float t = phase;

    float value = 2.0f * t - 1.0f;
    value += polyBLEP(t);

    advance();
    return value;
}

float OscillatorPolyBLEP::square()
{
    float t = phase;
    float value = (t < 0.5f) ? 1.0f : -1.0f;
    value += polyBLEP(t);

    float t2 = t + 0.5f;
    if (t2 >= 1.0f) t2 -= 1.0f;
    value -= polyBLEP(t2);

    advance();
    return value;
}

float OscillatorPolyBLEP::triangle()
{
    float t = phase;
    float value = (t < 0.5f) ? 1.0f : -1.0f;
    value += polyBLEP(t);
    value -= polyBLEP(fmod(t + 0.5f, 1.0f));

    // Integrar con leaky integrator para evitar DC drift
    float k = 4.0f * frequency / sampleRate;
    integrator = integrator * 0.9999f + value * k;

    advance();
    return integrator;
}

float OscillatorPolyBLEP::squarePWM()
{
    float t = phase;

    // Onda cuadrada con ancho de pulso variable
    float value = (t < pulseWidth) ? 1.0f : -1.0f;

    // PolyBLEP en el flanco ascendente (t = 0)
    value += polyBLEP(t);

    // PolyBLEP en el flanco descendente (t = width)
    float t2 = t + (1.0f - pulseWidth);
    if (t2 >= 1.0f) t2 -= 1.0f;
    value -= polyBLEP(t2);

    value -= (2.0f * pulseWidth - 1.0f);

    advance();
    return value;
}

void OscillatorPolyBLEP::syncPhase(const OscillatorPolyBLEP& other)
{
    // Copiar la fase del otro oscilador para sincronizar
    phase = other.phase;
}

float OscillatorPolyBLEP::nextSample()
{
    return saw();  // default waveform
}
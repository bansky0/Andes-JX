/*
  ==============================================================================
    OscillatorPolyBLEP.cpp

    ESP:
    Implementación del oscilador PolyBLEP. La idea central:
    - Generar una forma “naive” (saw/square/PWM) con discontinuidades
    - Aplicar una corrección polinómica local (PolyBLEP) cerca del borde
      para reducir aliasing.

    ENG:
    PolyBLEP oscillator implementation. Core idea:
    - Generate a naive waveform (saw/square/PWM) with discontinuities
    - Apply a local polynomial correction (PolyBLEP) near the edge
      to reduce aliasing.
  ==============================================================================
*/

#include "OscillatorPolyBLEP.h"
#include <cmath>

//==============================================================================
// Helpers
//==============================================================================

//------------------------------------------------------------------------------
// ESP: Avanza fase normalizada [0..1) con incremento frequency/sampleRate.
//      Envolver (wrap) evita crecimiento indefinido y mantiene estabilidad numérica.
// ENG: Advances normalized phase [0..1) with increment frequency/sampleRate.
//      Wrapping prevents unbounded growth and keeps numerical stability.
float OscillatorPolyBLEP::advance()
{
    phase += frequency / sampleRate;                 //

    if (phase >= 1.0f) {
        phase -= 1.0f;
    }
    return phase;
}

//------------------------------------------------------------------------------
// ESP: PolyBLEP(t) corrige el salto (step) cerca de una discontinuidad.
//      dt = incremento de fase por muestra. Se limita a 0.5 por seguridad.
//      Retorna 0 fuera de la vecindad del borde.
// ENG: PolyBLEP(t) corrects a step discontinuity near an edge.
//      dt = phase increment per sample. Clamped to 0.5 for safety.
//      Returns 0 outside the edge neighborhood.
float OscillatorPolyBLEP::polyBLEP(float t) const
{
    float dt = frequency / sampleRate;
    dt = std::min(dt, 0.5f);                         //

    // ESP/ENG: rising edge (t cercano a 0)
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0f;                 //
    }
    // ESP/ENG: falling edge (t cercano a 1)
    else if (t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;                 //
    }

    return 0.0f;
}

//==============================================================================
// Public API
//==============================================================================

//------------------------------------------------------------------------------
// ESP: Inicializa sampleRate y reinicia fase/integrador.
// ENG: Initializes sampleRate and resets phase/integrator.
void OscillatorPolyBLEP::prepare(double newSampleRate)
{
    sampleRate = (float)newSampleRate;
    phase = 0.0f;
    integrator = 0.0f;                               //
}

//------------------------------------------------------------------------------
// ESP: Frecuencia en Hz.
// ENG: Frequency in Hz.
void OscillatorPolyBLEP::setFrequency(float newFreq)
{
    frequency = newFreq;                             //
}

//==============================================================================
// Waveforms
//==============================================================================

//------------------------------------------------------------------------------
// ESP: Seno no requiere corrección PolyBLEP (no hay discontinuidades).
// ENG: Sine does not need PolyBLEP correction (no discontinuities).
float OscillatorPolyBLEP::sine()
{
    float out = std::sin(TWO_PI * phase);
    advance();
    return out;                                      //
}

//------------------------------------------------------------------------------
// ESP: Saw naive (2t-1) + corrección PolyBLEP en el salto del ciclo.
// ENG: Naive saw (2t-1) + PolyBLEP correction at the cycle discontinuity.
float OscillatorPolyBLEP::saw()
{
    float t = phase;

    float value = 2.0f * t - 1.0f;
    value += polyBLEP(t);

    advance();
    return value;                                    //
}

//------------------------------------------------------------------------------
// ESP: Square con dos bordes por ciclo (t=0 y t=0.5).
//      Se suma PolyBLEP en el flanco ascendente y se resta en el descendente.
// ENG: Square with two edges per cycle (t=0 and t=0.5).
//      Add PolyBLEP at rising edge and subtract at falling edge.
float OscillatorPolyBLEP::square()
{
    float t = phase;
    float value = (t < 0.5f) ? 1.0f : -1.0f;
    value += polyBLEP(t);

    float t2 = t + 0.5f;
    if (t2 >= 1.0f) t2 -= 1.0f;
    value -= polyBLEP(t2);

    advance();
    return value;                                    //
}

//------------------------------------------------------------------------------
// ESP: Triángulo por integración de una square band-limited.
//      Leaky integrator (0.9999) evita acumulación DC/drift con modulación.
// ENG: Triangle via integration of a band-limited square.
//      Leaky integrator (0.9999) prevents DC build-up/drift under modulation.
float OscillatorPolyBLEP::triangle()
{
    float t = phase;
    float value = (t < 0.5f) ? 1.0f : -1.0f;
    value += polyBLEP(t);
    value -= polyBLEP(fmod(t + 0.5f, 1.0f));

    float k = 4.0f * frequency / sampleRate;
    integrator = integrator * 0.9999f + value * k;

    advance();
    return integrator;                               //
}

//------------------------------------------------------------------------------
// ESP: Square PWM. Dos discontinuidades: t=0 y t=pulseWidth.
//      Se corrigen ambos bordes con PolyBLEP y se compensa DC para centrar la onda.
// ENG: PWM square. Two discontinuities: t=0 and t=pulseWidth.
//      Correct both edges with PolyBLEP and apply DC compensation to re-center.
float OscillatorPolyBLEP::squarePWM()
{
    float t = phase;

    float value = (t < pulseWidth) ? 1.0f : -1.0f;

    // PolyBLEP en el flanco ascendente (t = 0)
    value += polyBLEP(t);

    // PolyBLEP en el flanco descendente (t = pulseWidth)
    float t2 = t + (1.0f - pulseWidth);
    if (t2 >= 1.0f) t2 -= 1.0f;
    value -= polyBLEP(t2);

    // ESP: Compensación DC aproximada (la PWM pura tiene media ≠ 0 si duty ≠ 0.5).
    // ENG: Approx DC compensation (raw PWM has non-zero mean when duty ≠ 0.5).
    value -= (2.0f * pulseWidth - 1.0f);

    advance();
    return value;                                    //
}

//------------------------------------------------------------------------------
// ESP: Sincronización simple: copia fase del otro oscilador.
// ENG: Simple sync: copy phase from the other oscillator.
void OscillatorPolyBLEP::syncPhase(const OscillatorPolyBLEP& other)
{
    phase = other.phase;                             //
}

//------------------------------------------------------------------------------
// ESP: Forma por defecto del motor.
// ENG: Default waveform for the engine.
float OscillatorPolyBLEP::nextSample()
{
    return saw();                                    //
}
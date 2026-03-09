/*
  ==============================================================================
    SVF.h

    ESP:
    Filtro de Estado Variable (SVF) usado como una de las implementaciones de filtro del sintetizador.
Este módulo se centra en el procesamiento DSP (coeficientes + estado interno) y expone
funciones para configurar cutoff/resonance y procesar muestras.

    ENG:
    State Variable Filter (SVF) used as one of the synth's filter implementations.
This module focuses on DSP processing (coefficients + internal state) and exposes
functions to set cutoff/resonance and process samples.
  ==============================================================================
*/

#pragma once
#include <cmath>
#include "Constants.h"
#include <algorithm>
//------------------------------------------------------------------------------
// ESP: Implementación SVF (State Variable Filter). Mantiene estado interno por voz
//      y procesa audio sample-by-sample. Ideal para barridos suaves de cutoff.
// ENG: SVF (State Variable Filter) implementation. Keeps per-voice internal state
//      and processes audio sample-by-sample. Well suited for smooth cutoff sweeps.
//------------------------------------------------------------------------------


class SVF
{
public:
//------------------------------------------------------------------------------
// ESP: Inicializa coeficientes/estado dependiente de sampleRate.
// ENG: Initializes sample-rate dependent coefficients/state.
//------------------------------------------------------------------------------

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
//------------------------------------------------------------------------------
// ESP: Limpia memorias internas del filtro (integradores/delays).
// ENG: Clears internal filter memories (integrators/delays).
//------------------------------------------------------------------------------

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
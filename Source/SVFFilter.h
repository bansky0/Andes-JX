/*
  ==============================================================================
    SVFFilter.h

    ESP:
    Wrapper/adapter del filtro SVF para cumplir con la interfaz común de filtros (IFilter)
utilizada por Voice. Encapsula la instancia SVF y traduce llamadas prepare/reset/setParams
al formato esperado por el motor.

    ENG:
    SVF wrapper/adapter that satisfies the common filter interface (IFilter) used by Voice.
It encapsulates an SVF instance and routes prepare/reset/setParams calls in the format
expected by the engine.
  ==============================================================================
*/

#pragma once

#include "IFilter.h"
#include "SVF.h"
#include <algorithm>
//------------------------------------------------------------------------------
// ESP: Implementación SVF (State Variable Filter). Mantiene estado interno por voz
//      y procesa audio sample-by-sample. Ideal para barridos suaves de cutoff.
// ENG: SVF (State Variable Filter) implementation. Keeps per-voice internal state
//      and processes audio sample-by-sample. Well suited for smooth cutoff sweeps.
//------------------------------------------------------------------------------


class SVFFilter : public IFilter
{
public:
    SVFFilter() = default;
//------------------------------------------------------------------------------
// ESP: Inicializa coeficientes/estado dependiente de sampleRate.
// ENG: Initializes sample-rate dependent coefficients/state.
//------------------------------------------------------------------------------


    void prepare(double newSampleRate) override
    {
        svf1.prepare(static_cast<float>(newSampleRate));
        svf2.prepare(static_cast<float>(newSampleRate));
        reset();
    }
//------------------------------------------------------------------------------
// ESP: Limpia memorias internas del filtro (integradores/delays).
// ENG: Clears internal filter memories (integrators/delays).
//------------------------------------------------------------------------------


    void reset() override
    {
        svf1.reset();
        svf2.reset();
    }

    void setSampleRate(float newSampleRate) override
    {
        this->sampleRate = newSampleRate;
        svf1.prepare(newSampleRate);
        svf2.prepare(newSampleRate);
    }

    void updateCoefficients(float cutoffHz, float resonance) override
    {
        resonance = std::clamp(resonance, 0.0f, 1.0f);
        lastRes = resonance;
        // Convertir resonance normalizado (0-1) a Q (estilo SVF 0.5-10)
        // resonance=0 → Q=0.5 (sin resonancia)
        // resonance=1 → Q=10 (alta resonancia)
        float Q = 0.5f + resonance * 9.5f;
        Q = std::clamp(Q, 0.5f, 10.0f);

        // Para cascada 24dB conviene bajar un poco Q en la 2da etapa
        // si no, el SVF puede sonar más agresivo que el ladder.
        float Q2 = 0.7f * Q;

        svf1.updateCoefficients(cutoffHz, Q);
        svf2.updateCoefficients(cutoffHz, Q2);
    }

    float render(float input) override
    {

        float y = svf1.render(input);
        y = svf2.render(y);

        // Compensación simple para no “ganar” por loudness/clipping con resonancia alta
        y *= 1.0f / (1.0f + 0.8f * lastRes);

        return y;
    }

    const char* getFilterType() const override
    {
        return "SVF (State Variable)";
    }

private:
    SVF svf1, svf2;
    float lastRes = 0.0f;
    float sampleRate = 48000.0f;
};
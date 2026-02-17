/*
  ==============================================================================

    SVFFilter.h
    Created: 11 Feb 2026 4:49:15pm
    Author:  Jhonatan

    Wrapper para SVF que implementa IFilter

    Adapta SVF existente para que sea compatible con
    el sistema de filtros intercambiables.

  ==============================================================================
*/

#pragma once

#include "IFilter.h"
#include "SVF.h"
#include <algorithm>

class SVFFilter : public IFilter
{
public:
    SVFFilter() = default;

    void prepare(double newSampleRate) override
    {
        //setSampleRate(static_cast<float>(newSampleRate));
        svf1.prepare(static_cast<float>(newSampleRate));
        svf2.prepare(static_cast<float>(newSampleRate));
        reset();
    }

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
        //return svf1.render(input);
        //return svf2.render(input);
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
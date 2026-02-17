/*
  ==============================================================================

    MoogFilter.h
    Created: 12 Feb 2026 10:43:48am
    Author:  Jhonatan

    Wrapper para LadderFilter que implementa IFilter

    Adapta el filtro Moog Ladder (Huovilainen) para que sea compatible con
    el sistema de filtros intercambiables.

  ==============================================================================
*/

#pragma once

#include "IFilter.h"
#include "LadderFilter.h"

class MoogFilter : public IFilter
{
public:
    MoogFilter() = default;

    void prepare(double sampleRate) override
    {
        moog.setSampleRate(static_cast<float>(sampleRate));
        moog.reset();
    }

    void reset() override
    {
        moog.reset();
    }

    void setSampleRate(float sampleRate) override
    {
        moog.setSampleRate(sampleRate);
    }

    void updateCoefficients(float cutoffHz, float resonance) override
    {
        // La resonance ya viene normalizada (0-1) desde Synth.cpp
        // El LadderFilter también usa 0-1, así que pasa directo
        moog.updateCoefficients(cutoffHz, resonance);
    }

    float render(float input) override
    {
        return moog.render(input);
    }

    const char* getFilterType() const override
    {
        return "Moog Ladder (Huovilainen)";
    }

private:
    LadderFilter moog;
};
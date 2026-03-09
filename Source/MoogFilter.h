/*
  ==============================================================================
    MoogFilter.h

    ESP:
    Implementación de filtro tipo "Moog Ladder" (ladder filter) para el sintetizador.
    Este tipo de filtro emula el carácter de un filtro analógico en escalera:
      - Respuesta suave y musical al barrer cutoff
      - Resonancia característica (auto-oscilación dependiendo del mapeo)
      - Procesamiento no lineal / saturación (según implementación)

    Se usa como una alternativa al SVF dentro de Voice/Synth, seleccionable por tipo.

    ENG:
    Moog Ladder filter implementation for the synthesizer.
    This filter model aims to reproduce the character of an analog ladder filter:
      - Smooth, musical cutoff sweeps
      - Characteristic resonance (possible self-oscillation depending on mapping)
      - Non-linear processing / saturation (implementation-dependent)

    Used as an alternative to SVF inside Voice/Synth, selectable by filter type.
  ==============================================================================
*/

#pragma once

#include "IFilter.h"
#include "LadderFilter.h"
//------------------------------------------------------------------------------
// ESP: Clase de filtro Moog/Ladder. Mantiene estado interno por voz y procesa
//      audio muestra a muestra.
// ENG: Moog/Ladder filter class. Keeps per-voice internal state and processes
//      audio sample-by-sample.
//------------------------------------------------------------------------------


class MoogFilter : public IFilter
{
public:
    MoogFilter() = default;
//------------------------------------------------------------------------------
// ESP: Inicialización dependiente de sampleRate (coeficientes, buffers internos).
// ENG: Sample-rate dependent initialization (coefficients, internal buffers).
//------------------------------------------------------------------------------


    void prepare(double sampleRate) override
    {
        moog.setSampleRate(static_cast<float>(sampleRate));
        moog.reset();
    }
//------------------------------------------------------------------------------
// ESP: Reinicia memorias internas del filtro (delays/estados).
// ENG: Resets internal filter memories (delays/states).
//------------------------------------------------------------------------------


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
        // El LadderFilter tambin usa 0-1, así que pasa directo
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
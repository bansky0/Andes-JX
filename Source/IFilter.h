/*
  ==============================================================================

    IFilter.h
    Created: 11 Feb 2026 4:36:24pm
    Author:  Jhonatan

    Interface base para filtros intercambiables

    Permite cambiar entre diferentes implementaciones de filtro (SVF, Moog, etc.)
    sin modificar el código de Voice o Synth.

  ==============================================================================
*/

#pragma once

class IFilter
{
public:
    virtual ~IFilter() = default;

    // Métodos virtuales puros que todos los filtros deben implementar
    virtual void prepare(double sampleRate) = 0;
    virtual void reset() = 0;
    virtual void setSampleRate(float sampleRate) = 0;
    virtual void updateCoefficients(float cutoffHz, float resonance) = 0;
    virtual float render(float input) = 0;

    // Método opcional para obtener el tipo de filtro (útil para debugging)
    virtual const char* getFilterType() const = 0;
};
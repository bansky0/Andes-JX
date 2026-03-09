/*
  ==============================================================================
    IFilter.h

    ESP:
    Interfaz (contrato) común para filtros por voz.
    Define las operaciones mínimas que el motor espera de cualquier filtro:
      - prepare(sampleRate): inicialización dependiente de SR
      - reset(): reinicio de estado interno
      - setParams(cutoffHz, resonance): configuración de parámetros
      - process(sample): procesa una muestra mono y devuelve la salida filtrada

    Esta interfaz permite intercambiar implementaciones (SVF, Moog/Ladder, etc.)
    sin cambiar el código de alto nivel (Voice/Synth).

    ENG:
    Common filter interface (contract) for per-voice filters.
    Defines the minimum operations the engine expects from any filter:
      - prepare(sampleRate): sample-rate dependent initialization
      - reset(): reset internal state
      - setParams(cutoffHz, resonance): parameter configuration
      - process(sample): process one mono sample and return filtered output

    This interface allows swapping implementations (SVF, Moog/Ladder, etc.)
    without changing high-level code (Voice/Synth).
  ==============================================================================
*/

#pragma once
//------------------------------------------------------------------------------
// ESP: Interfaz de filtro. Las clases derivadas implementan el procesamiento y
//      mantienen su propio estado (delays, integradores, etc.).
// ENG: Filter interface. Derived classes implement processing and keep their own
//      internal state (delays, integrators, etc.).
//------------------------------------------------------------------------------


class IFilter
{
public:
    virtual ~IFilter() = default;

    // Métodos virtuales puros que todos los filtros deben implementar
//------------------------------------------------------------------------------
// ESP: Inicializa el filtro para un sampleRate dado. Debe llamarse antes de process().
// ENG: Initializes the filter for a given sampleRate. Must be called before process().
//------------------------------------------------------------------------------

    virtual void prepare(double sampleRate) = 0;
//------------------------------------------------------------------------------
// ESP: Reinicia el estado interno del filtro (memorias, integradores, etc.).
// ENG: Resets the filter internal state (memories, integrators, etc.).
//------------------------------------------------------------------------------

    virtual void reset() = 0;
    virtual void setSampleRate(float sampleRate) = 0;
    virtual void updateCoefficients(float cutoffHz, float resonance) = 0;
    virtual float render(float input) = 0;

    // Método opcional para obtener el tipo de filtro (útil para debugging)
    virtual const char* getFilterType() const = 0;
};
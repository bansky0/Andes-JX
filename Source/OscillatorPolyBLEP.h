/*
  ==============================================================================

    OscillatorPolyBLEP.h
    Created: 21 Nov 2025 11:12:13am
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: OscillatorPolyBLEP
    Purpose:
        EN: Band-limited oscillator engine that powers the Oscillator facade.
            Implements classic subtractive waveforms (sine, saw, square,
            squarePWM, triangle) with anti-aliasing at audio rate.
        ES: Motor del oscilador band-limited que alimenta la fachada
            Oscillator. Implementa las formas de onda clásicas (sine, saw,
            square, squarePWM, triangle) con anti-aliasing a frecuencia
            de audio.

    Main responsibilities:
        EN:
          - Advance and wrap the internal phase at the set frequency
          - Generate aliasing-free saw and square via PolyBLEP correction
          - Derive triangle by leaky-integrating a band-limited square
          - Support hard phase-sync between two oscillators
        ES:
          - Avanzar y envolver la fase interna a la frecuencia fijada
          - Generar saw y square sin aliasing mediante corrección PolyBLEP
          - Derivar triangle integrando (leaky) una square band-limited
          - Permitir sync duro de fase entre dos osciladores

    Architectural role:
        EN: Owned privately by Oscillator (see Oscillator.h). This header
            only declares the interface; the DSP implementation lives in
            OscillatorPolyBLEP.cpp, where the PolyBLEP algorithm and its
            references are documented in detail.
        ES: Propiedad privada de Oscillator (ver Oscillator.h). Este header
            solo declara la interfaz; la implementación DSP está en
            OscillatorPolyBLEP.cpp, donde el algoritmo PolyBLEP y sus
            referencias se documentan en detalle.

    Notes:
        EN:
          - PolyBLEP (Polynomial Band-Limited Step) corrects the aliasing
            produced by waveforms with discontinuities (saw, square) by
            adding a small polynomial residual around each discontinuity.
          - Pulse width is clamped to [0.05, 0.95] to avoid degenerate
            square waves that approach silence or DC.
          - All state is plain floats; no dynamic allocation, safe for
            the real-time audio callback.
        ES:
          - PolyBLEP (Polynomial Band-Limited Step) corrige el aliasing
            producido por formas de onda con discontinuidades (saw, square)
            sumando un residuo polinómico alrededor de cada discontinuidad.
          - El ancho de pulso se limita a [0.05, 0.95] para evitar ondas
            cuadradas degeneradas que tiendan al silencio o al DC.
          - Todo el estado son floats simples; sin reservas dinámicas,
            seguro para el callback de audio en tiempo real.
*/

#pragma once
#include <cmath>
#include "Constants.h"
#include <algorithm>


class OscillatorPolyBLEP
{
public:
    // EN: Stores the host sample rate and recalculates internal increments.
    //     Must be called before the first call to any sample-generating method.
    // ES: Guarda la sample rate del host y recalcula los incrementos internos.
    //     Debe llamarse antes del primer método que genere muestras.
    void prepare(double sampleRate);

    // EN: Sets the oscillator frequency in Hz and refreshes phaseInc.
    // ES: Fija la frecuencia del oscilador en Hz y actualiza phaseInc.
    void setFrequency(float freq);

    // EN: Sets the pulse width used by squarePWM. Clamped to [0.05, 0.95]
    //     to avoid degenerate waveforms near silence or DC.
    // ES: Fija el ancho de pulso usado por squarePWM. Limitado a [0.05, 0.95]
    //     para evitar formas de onda degeneradas cerca del silencio o del DC.
    void setPulseWidth(float width)
    {
        pulseWidth = std::clamp(width, 0.05f, 0.95f);
    }

    // EN: Resets phase and integrator to zero. Call on note-on when every
    //     note should start from the same phase state.
    // ES: Reinicia la fase y el integrador a cero. Llamar en note-on cuando
    //     cada nota deba arrancar desde el mismo estado de fase.
    void reset()
    {
        phase = 0.f;
        integrator = 0.f;
    }

    // EN: Copies another oscillator's phase into this one. Used by
    //     Oscillator::syncPhase to implement hard-sync effects.
    // ES: Copia la fase de otro oscilador en este. Lo usa
    //     Oscillator::syncPhase para implementar efectos de hard-sync.
    void syncPhase(const OscillatorPolyBLEP& other);


    // --- Waveform generators / Generadores de forma de onda ----------------
    // EN: Each method returns one sample of the corresponding waveform and
    //     internally advances the phase. PolyBLEP correction is applied to
    //     saw, square and squarePWM; triangle derives from an integrated
    //     square; sine is a direct sin() call (already band-limited).
    // ES: Cada método devuelve una muestra de la forma de onda correspondiente
    //     y avanza la fase internamente. La corrección PolyBLEP se aplica a
    //     saw, square y squarePWM; triangle se deriva de una square integrada;
    //     sine es un sin() directo (ya es band-limited).
    float sine();
    float saw();
    float square();
    float squarePWM();
    float triangle();

    // EN: Advances phase by one sample without producing any specific
    //     waveform. Useful when the engine needs to keep running without
    //     audible output (e.g. for sync reference).
    // ES: Avanza la fase una muestra sin producir una forma de onda
    //     específica. Útil cuando el motor debe seguir corriendo sin salida
    //     audible (p. ej. como referencia para sync).
    float nextSample();


private:
    // --- Runtime state / Estado en ejecución -------------------------------

    // EN: Host sample rate. Default 48 kHz is replaced by prepare().
    // ES: Sample rate del host. El valor por defecto 48 kHz lo reemplaza prepare().
    float sampleRate = 48000.f;

    // EN: Normalized phase in [0, 1). Wraps around on each cycle.
    // ES: Fase normalizada en [0, 1). Se envuelve en cada ciclo.
    float phase = 0.0f;

    // EN: Phase increment per sample = frequency / sampleRate.
    // ES: Incremento de fase por muestra = frequency / sampleRate.
    float phaseInc = 0.0f;

    // EN: Current oscillator frequency in Hz.
    // ES: Frecuencia actual del oscilador en Hz.
    float frequency = 440.f;

    // EN: Leaky integrator state used to derive the triangle waveform from
    //     a band-limited square. "Leaky" means it decays slightly each
    //     sample to prevent DC drift.
    // ES: Estado del integrador leaky usado para derivar la onda triangle
    //     desde una square band-limited. "Leaky" significa que decae un
    //     poco por muestra para evitar deriva de DC.
    float integrator = 0.0f;

    // EN: Pulse width for squarePWM in (0, 1). Clamped by setPulseWidth.
    // ES: Ancho de pulso para squarePWM en (0, 1). Limitado por setPulseWidth.
    float pulseWidth = 0.5f;


    // --- Internal DSP / DSP interno ----------------------------------------

    // EN: PolyBLEP polynomial residual. Given a normalized phase offset `t`
    //     relative to a discontinuity, returns a small correction value
    //     that cancels the first-order alias components. Implementation
    //     and references live in OscillatorPolyBLEP.cpp.
    // ES: Residuo polinómico de PolyBLEP. Dada una fase normalizada `t`
    //     relativa a una discontinuidad, devuelve un valor de corrección
    //     que cancela los componentes de aliasing de primer orden.
    //     Implementación y referencias en OscillatorPolyBLEP.cpp.
    float polyBLEP(float t) const;

    // EN: Advances phase by phaseInc and wraps it into [0, 1). Returns the
    //     previous phase (useful for PolyBLEP discontinuity detection).
    // ES: Avanza la fase en phaseInc y la envuelve a [0, 1). Devuelve la
    //     fase anterior (útil para detectar discontinuidades en PolyBLEP).
    float advance();
};
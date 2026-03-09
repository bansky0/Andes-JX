/*
  ==============================================================================
    Oscillator.h

    ESP:
    Interfaz de oscilador por voz (wrapper) sobre OscillatorPolyBLEP.
    Encapsula la selección de forma de onda y el ruteo hacia el generador
    band-limited (PolyBLEP) para minimizar aliasing en ondas con discontinuidades
    (saw/square/PWM). Se usa típicamente dentro de Voice para generar muestras
    por oscilador (osc1/osc2).

    ENG:
    Per-voice oscillator interface (wrapper) around OscillatorPolyBLEP.
    Encapsulates waveform selection and routing into a band-limited (PolyBLEP)
    generator to minimize aliasing for discontinuous waveforms (saw/square/PWM).
    Typically used inside Voice to generate samples per oscillator (osc1/osc2).
  ==============================================================================
*/

#pragma once

#include "OscillatorPolyBLEP.h"

//==============================================================================
// ESP: Tipos de forma de onda soportados por el oscilador.
//      SquarePWM usa ancho de pulso variable (pulse width) para timbre dinámico.
// ENG: Supported oscillator waveforms.
//      SquarePWM uses variable pulse width for dynamic timbre.
enum class WaveType
{
    Sine,
    Saw,
    Square,
    SquarePWM,
    Triangle
};

class Oscillator
{
public:
    //==========================================================================
    // ESP: Ganancia base del oscilador (salida típica en [-1..1] * amplitude).
    // ENG: Base oscillator gain (typical output in [-1..1] * amplitude).
    float amplitude = 1.0f;

    //==========================================================================
    // ESP: Inicializa dependencias con sampleRate (debe llamarse antes de renderizar).
    // ENG: Initializes sample-rate dependent internals (must be called before rendering).
    void prepare(double sampleRate)
    {
        polyblep.prepare(sampleRate);
    }

    //==========================================================================
    // ESP: Configura la frecuencia fundamental (Hz).
    // ENG: Sets fundamental frequency (Hz).
    void setFrequency(float freq)
    {
        polyblep.setFrequency(freq);
    }

    //==========================================================================
    // ESP: Selecciona la forma de onda a generar.
    // ENG: Selects which waveform to generate.
    void setWaveType(WaveType type)
    {
        waveType = type;
    }

    //==========================================================================
    // ESP: Configura el ancho de pulso para SquarePWM (rango típico: 0..1).
    //      Nota: el rango exacto/seguro depende de la implementación interna (evitar 0 o 1).
    // ENG: Sets pulse width for SquarePWM (typical range: 0..1).
    //      Note: exact safe range depends on the internal implementation (avoid 0 or 1).
    void setPulseWidth(float width)
    {
        polyblep.setPulseWidth(width);
    }

    //==========================================================================
    // ESP: Sincroniza la fase con otro oscilador (útil para hard-sync / reinicios coherentes).
    // ENG: Phase-sync with another oscillator (useful for hard sync / coherent restarts).
    void syncPhase(const Oscillator& other)
    {
        polyblep.syncPhase(other.polyblep);
    }

    //==========================================================================
    // ESP: Devuelve la siguiente muestra del oscilador, según waveType.
    //      El motor PolyBLEP genera versiones band-limited de ondas con bordes
    //      abruptos para reducir aliasing.
    // ENG: Returns the next oscillator sample, based on waveType.
    //      The PolyBLEP engine generates band-limited versions of discontinuous
    //      waveforms to reduce aliasing.
    float nextSample()
    {
        switch (waveType)
        {
            case WaveType::Sine:        return polyblep.sine();
            case WaveType::Saw:         return polyblep.saw();
            case WaveType::Square:      return polyblep.square();
            case WaveType::SquarePWM:   return polyblep.squarePWM();
            case WaveType::Triangle:    return polyblep.triangle();
        }

        // ESP/ENG: Fallback defensivo (no debería ocurrir).
        return 0.0f;
    }

    //==========================================================================
    // ESP: Reinicia el estado interno (fase y acumuladores del motor).
    // ENG: Resets internal state (phase and internal accumulators).
    void reset()
    {
        polyblep.reset();
    }

private:
    //==========================================================================
    // ESP: Forma de onda actual (por defecto: Saw).
    // ENG: Current waveform (default: Saw).
    WaveType waveType = WaveType::Saw;

    //==========================================================================
    // ESP: Motor DSP band-limited (PolyBLEP).
    // ENG: Band-limited DSP engine (PolyBLEP).
    OscillatorPolyBLEP polyblep;
};
/*
  ==============================================================================

    Oscillator.h
    Created: 15 Nov 2025 4:28:29pm
    Author:  Valeria

  ==============================================================================
*/

/*
    Module: Oscillator
    Purpose:
        EN: High-level oscillator facade. Exposes a waveform-oriented API
            (Sine, Saw, Square, SquarePWM, Triangle) while delegating the
            band-limited synthesis to OscillatorPolyBLEP.
        ES: Fachada de alto nivel del oscilador. Expone una API orientada a
            forma de onda (Sine, Saw, Square, SquarePWM, Triangle) delegando
            la síntesis band-limited a OscillatorPolyBLEP.

    Main responsibilities:
        EN:
          - Present a simple setWaveType / nextSample interface to Voice
          - Forward frequency, pulse width and phase-sync to the PolyBLEP core
          - Keep the rest of the synth decoupled from the underlying algorithm
        ES:
          - Presentar una interfaz simple setWaveType / nextSample a Voice
          - Reenviar frecuencia, ancho de pulso y sync de fase al núcleo PolyBLEP
          - Mantener al resto del synth desacoplado del algoritmo subyacente

    Architectural role:
        EN: Used by Voice, which owns two oscillators per voice (osc1, osc2)
            and combines them via the oscMix parameter. The Oscillator class
            itself contains no DSP: all sample generation happens inside
            OscillatorPolyBLEP.
        ES: Usado por Voice, que posee dos osciladores por voz (osc1, osc2)
            y los combina mediante el parámetro oscMix. La clase Oscillator
            no contiene DSP: toda la generación ocurre dentro de
            OscillatorPolyBLEP.

    Notes:
        EN:
          - Facade pattern: this thin layer lets the DSP engine be swapped
            (as it already was, see historical reference at the bottom)
            without touching Voice or the rest of the synth.
          - The default wave is Saw, chosen because it is the most versatile
            starting point for subtractive synthesis (rich in harmonics).
        ES:
          - Patrón fachada: esta capa fina permite intercambiar el motor DSP
            (como ya ocurrió una vez, ver referencia histórica al final)
            sin tocar Voice ni el resto del sintetizador.
          - La onda por defecto es Saw, por ser el punto de partida más
            versátil para la síntesis sustractiva (rica en armónicos).
*/

#pragma once
#include "OscillatorPolyBLEP.h"


// EN: Available waveforms. The enum class avoids implicit conversions and
//     makes switch statements exhaustive-checkable by the compiler.
// ES: Formas de onda disponibles. El enum class evita conversiones implícitas
//     y permite al compilador verificar que los switch sean exhaustivos.
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
    // EN: Output gain applied by downstream consumers (Voice mixes osc1 and
    //     osc2 using this). Kept public for audio-callback performance.
    // ES: Ganancia de salida aplicada por los consumidores (Voice mezcla
    //     osc1 y osc2 usando esto). Público por rendimiento en el callback.
    float amplitude = 1.0f;


    // EN: Initializes the underlying PolyBLEP core with the host sample rate.
    //     Must be called before the first nextSample() (usually from
    //     AudioProcessor::prepareToPlay).
    // ES: Inicializa el núcleo PolyBLEP con la sample rate del host.
    //     Debe llamarse antes del primer nextSample() (normalmente desde
    //     AudioProcessor::prepareToPlay).
    void prepare(double sampleRate)
    {
        polyblep.prepare(sampleRate);
    }

    // EN: Sets the oscillator frequency in Hz.
    // ES: Fija la frecuencia del oscilador en Hz.
    void setFrequency(float freq)
    {
        polyblep.setFrequency(freq);
    }

    // EN: Selects which waveform nextSample() will produce on subsequent calls.
    // ES: Selecciona qué forma de onda producirá nextSample() en lo sucesivo.
    void setWaveType(WaveType type)
    {
        waveType = type;
    }

    // EN: Sets the pulse width for SquarePWM (typically in [0, 1]).
    //     Ignored by the other waveforms.
    // ES: Fija el ancho de pulso para SquarePWM (típicamente en [0, 1]).
    //     Las demás formas de onda lo ignoran.
    void setPulseWidth(float width)
    {
        polyblep.setPulseWidth(width);
    }

    // EN: Hard-syncs this oscillator's phase to another one. Used to
    //     implement oscillator sync effects (osc1 resets osc2 on each cycle).
    // ES: Sincroniza duro la fase de este oscilador con otro. Se usa para
    //     implementar sync de osciladores (osc1 resetea a osc2 en cada ciclo).
    void syncPhase(const Oscillator& other)
    {
        polyblep.syncPhase(other.polyblep);
    }

    // EN: Produces one audio sample of the currently selected waveform.
    //     Band-limiting is handled by the PolyBLEP core.
    // ES: Produce una muestra de audio de la forma de onda seleccionada.
    //     El band-limiting lo maneja el núcleo PolyBLEP.
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
        return 0.0f; // EN: unreachable, silences compiler warnings
        // ES: inalcanzable, silencia avisos del compilador
    }

    // EN: Resets the internal phase to a known state. Call on note-on
    //     when you want every note to start from the same phase.
    // ES: Reinicia la fase interna a un estado conocido. Llamar en note-on
    //     cuando se quiere que cada nota arranque desde la misma fase.
    void reset()
    {
        polyblep.reset();
    }


private:
    WaveType waveType = WaveType::Saw;
    OscillatorPolyBLEP polyblep;
};


/*
    ============================================================================
    HISTORICAL REFERENCE / REFERENCIA HISTÓRICA
    ============================================================================

    EN: Earlier oscillator implementation, preserved here as a teaching
        artifact. It uses the BLIT-DSF algorithm (Band-Limited Impulse Train
        via Discrete Summation Formula), which generates aliasing-free
        waveforms using the Dirichlet kernel  sin(Nx)/sin(x)  together with
        a DC compensation term.

        Why it was replaced by PolyBLEP:
          - Numerical instability near phase = 0 (the  phase*phase > 1e-9
            guard is a symptom, not a fix).
          - DC offset that must be subtracted every sample.
          - Less stable behavior at low frequencies compared to PolyBLEP.
          - Less modular: switching waveforms requires rederiving the
            whole summation, whereas PolyBLEP cleanly isolates the
            discontinuity correction per waveform type.

        Reference:
          Stilson, T. & Smith, J. O. (1996). "Alias-Free Digital Synthesis
          of Classic Analog Waveforms". Proc. ICMC.

    ES: Implementación anterior del oscilador, conservada como artefacto
        didáctico. Usa el algoritmo BLIT-DSF (Band-Limited Impulse Train
        via Discrete Summation Formula), que genera formas de onda sin
        aliasing usando el núcleo de Dirichlet  sin(Nx)/sin(x)  junto con
        un término de compensación de DC.

        Por qué se reemplazó por PolyBLEP:
          - Inestabilidad numérica cerca de fase = 0 (la guarda
            phase*phase > 1e-9 es un síntoma, no una solución).
          - Offset de DC que hay que restar cada muestra.
          - Comportamiento menos estable en frecuencias bajas que PolyBLEP.
          - Menos modular: cambiar de forma de onda exige rederivar toda
            la sumatoria, mientras que PolyBLEP aísla limpiamente la
            corrección de discontinuidad por tipo de onda.

        Referencia:
          Stilson, T. & Smith, J. O. (1996). "Alias-Free Digital Synthesis
          of Classic Analog Waveforms". Proc. ICMC.

    ----------------------------------------------------------------------------

#pragma once
#include <cmath>

const float PI_OVER_4 = 0.7853981633974483f;
const float PI = 3.1415926535897932f;
const float TWO_PI = 6.2831853071795864f;

class Oscillator
{
    public:
        float period = 0.0f;
        float amplitude = 1.0f;

    void reset()
    {
        inc = 0.0f;
        phase = 0.0f;
        sin0 = 0.0f;
        sin1 = 0.0f;
        dsin = 0.0f;
        dc = 0.0f;
    }

    float nextSample()
    {
        float output = 0.0f;
        phase += inc;
        if (phase <= PI_OVER_4) {

            float halfPeriod = period / 2.0f;
            phaseMax = std::floor(0.5f + halfPeriod) - 0.5f;
            dc = 0.5f * amplitude / phaseMax;
            phaseMax *= PI;
            inc = phaseMax / halfPeriod;
            phase = -phase;
            sin0 = amplitude * std::sin(phase);
            sin1 = amplitude * std::sin(phase - inc);
            dsin = 2.0f * std::cos(inc);
            if (phase * phase > 1e-9) {
                output = sin0 / phase;
            }
            else {
                output = amplitude;
            }
        }
        else {
            if (phase > phaseMax) {
                phase = phaseMax + phaseMax - phase;
                inc = -inc;
            }
            float sinp = dsin * sin0 - sin1;
            sin1 = sin0;
            sin0 = sinp;
            output = sinp / phase;

        }
        return output - dc;

    }
    private:
        float phase;
        float phaseMax;
        float inc;
        float sin0;
        float sin1;
        float dsin;
        float dc;
};

*/
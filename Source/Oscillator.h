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
            band-limited synthesis to a selectable anti-aliasing engine.
            The ADOPTED engine is DPW (OscillatorDPW); OscillatorPolyBLEP
            is retained as a swappable comparative engine.
        ES: Fachada de alto nivel del oscilador. Expone una API orientada a
            forma de onda (Sine, Saw, Square, SquarePWM, Triangle) delegando
            la síntesis band-limited a un motor de anti-aliasing seleccionable.
            El motor ADOPTADO es DPW (OscillatorDPW); OscillatorPolyBLEP se
            conserva como motor comparativo conmutable.

    Main responsibilities:
        EN:
          - Present a simple setWaveType / setEngine / nextSample interface
          - Forward frequency, pulse width and phase-sync to BOTH cores so
            the active engine always has up-to-date state
          - Keep the rest of the synth decoupled from the algorithm in use
        ES:
          - Presentar una interfaz simple setWaveType / setEngine / nextSample
          - Reenviar frecuencia, ancho de pulso y sync de fase a AMBOS núcleos
            para que el motor activo tenga siempre el estado actualizado
          - Mantener al resto del synth desacoplado del algoritmo en uso

    Architectural role:
        EN: Used by Voice, which owns two oscillators per voice (osc1, osc2)
            and combines them via the oscMix parameter. The Oscillator class
            itself contains no DSP: all sample generation happens inside
            OscillatorDPW or OscillatorPolyBLEP depending on OscEngine. The
            default engine is DPW (the technique adopted for the synth) so
            no caller change is needed for the reviewer-aligned behaviour.
        ES: Usado por Voice, que posee dos osciladores por voz (osc1, osc2)
            y los combina mediante el parámetro oscMix. La clase Oscillator
            no contiene DSP: toda la generación ocurre dentro de
            OscillatorDPW u OscillatorPolyBLEP según OscEngine. El motor por
            defecto es DPW (la técnica adoptada para el sintetizador), así
            que ningún llamador necesita cambios para el comportamiento
            alineado con el revisor.

    Notes:
        EN:
          - Facade pattern: this thin layer lets the DSP engine be swapped
            without touching Voice or the rest of the synth. The engine has
            in fact been swapped twice (see historical reference below).
          - The default wave is Saw, chosen because it is the most versatile
            starting point for subtractive synthesis (rich in harmonics).
          - The default engine is DPW; PolyBLEP / Hybrid are exposed only
            for the comparative teaching module.
        ES:
          - Patrón fachada: esta capa fina permite intercambiar el motor DSP
            sin tocar Voice ni el resto del sintetizador. El motor ya se ha
            intercambiado dos veces (ver referencia histórica al final).
          - La onda por defecto es Saw, por ser el punto de partida más
            versátil para la síntesis sustractiva (rica en armónicos).
          - El motor por defecto es DPW; PolyBLEP / Hybrid se exponen solo
            para el módulo comparativo de enseñanza.
*/

#pragma once
#include "OscillatorPolyBLEP.h"
#include "OscillatorDPW.h"


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


// EN: Anti-aliasing engine selector.
//       DPW      - adopted technique (pedagogically accessible)
//       PolyBLEP - historical / professional-standard engine, comparative
//       Hybrid   - DPW for saw & triangle, PolyBLEP for square & PWM
//     Default is DPW so existing callers (Voice) need no change.
// ES: Selector del motor de anti-aliasing.
//       DPW      - técnica adoptada (accesible pedagógicamente)
//       PolyBLEP - motor histórico / estándar profesional, comparativo
//       Hybrid   - DPW para saw y triangle, PolyBLEP para square y PWM
//     Por defecto DPW para que los llamadores actuales (Voice) no cambien.
enum class OscEngine
{
    DPW,
    PolyBLEP,
    Hybrid
};


class Oscillator
{
public:
    // EN: Output gain applied by downstream consumers (Voice mixes osc1 and
    //     osc2 using this). Kept public for audio-callback performance.
    // ES: Ganancia de salida aplicada por los consumidores (Voice mezcla
    //     osc1 y osc2 usando esto). Público por rendimiento en el callback.
    float amplitude = 1.0f;


    // EN: Initializes BOTH engines with the host sample rate. Must be called
    //     before the first nextSample() (usually from prepareToPlay).
    // ES: Inicializa AMBOS motores con la sample rate del host. Debe llamarse
    //     antes del primer nextSample() (normalmente desde prepareToPlay).
    void prepare(double sampleRate)
    {
        polyblep.prepare(sampleRate);
        dpw.prepare(sampleRate);
    }

    // EN: Sets the oscillator frequency in Hz on both engines.
    // ES: Fija la frecuencia del oscilador en Hz en ambos motores.
    void setFrequency(float freq)
    {
        polyblep.setFrequency(freq);
        dpw.setFrequency(freq);
    }

    // EN: Selects which waveform nextSample() will produce on subsequent calls.
    // ES: Selecciona qué forma de onda producirá nextSample() en lo sucesivo.
    void setWaveType(WaveType type)
    {
        waveType = type;
    }

    // EN: Selects the anti-aliasing engine. Default DPW = adopted technique;
    //     PolyBLEP / Hybrid are for the comparative teaching module.
    // ES: Selecciona el motor de anti-aliasing. Por defecto DPW = técnica
    //     adoptada; PolyBLEP / Hybrid para el módulo comparativo.
    void setEngine(OscEngine e)
    {
        engine = e;
    }

    // EN: Sets the pulse width for SquarePWM (clamped internally to a safe
    //     range). Forwarded to BOTH engines so the active one is always in
    //     sync. Ignored by the other waveforms.
    // ES: Fija el ancho de pulso para SquarePWM (limitado internamente a un
    //     rango seguro). Se reenvía a AMBOS motores para que el activo esté
    //     siempre sincronizado. Las demás ondas lo ignoran.
    void setPulseWidth(float width)
    {
        polyblep.setPulseWidth(width);
        dpw.setPulseWidth(width);
    }

    // EN: Hard-syncs this oscillator's phase to another one (osc1 resets
    //     osc2 each cycle). Forwarded to BOTH engines so sync works
    //     regardless of the selected OscEngine.
    // ES: Sincroniza duro la fase de este oscilador con otro (osc1 resetea
    //     a osc2 en cada ciclo). Se reenvía a AMBOS motores para que el sync
    //     funcione con cualquier OscEngine seleccionado.
    void syncPhase(const Oscillator& other)
    {
        polyblep.syncPhase(other.polyblep);
        dpw.syncPhase(other.dpw);
    }

    // EN: Produces one audio sample of the currently selected waveform,
    //     using the currently selected engine. Sine needs no anti-aliasing,
    //     so it does not branch on the engine.
    // ES: Produce una muestra de audio de la forma de onda seleccionada,
    //     usando el motor seleccionado. La sinusoide no necesita
    //     anti-aliasing, por eso no se ramifica según el motor.
    float nextSample()
    {
        const bool useDPW =
            (engine == OscEngine::DPW) ||
            (engine == OscEngine::Hybrid &&
                (waveType == WaveType::Saw || waveType == WaveType::Triangle));

        switch (waveType)
        {
        case WaveType::Sine:        return dpw.sine();
        case WaveType::Saw:         return useDPW ? dpw.saw() : polyblep.saw();
        case WaveType::Square:      return useDPW ? dpw.square() : polyblep.square();
        case WaveType::SquarePWM:   return useDPW ? dpw.squarePWM() : polyblep.squarePWM();
        case WaveType::Triangle:    return useDPW ? dpw.triangle() : polyblep.triangle();
        }
        return 0.0f; // EN: unreachable, silences compiler warnings
        // ES: inalcanzable, silencia avisos del compilador
    }

    // EN: Resets the internal phase of both engines to a known state. Call
    //     on note-on when you want every note to start from the same phase.
    // ES: Reinicia la fase interna de ambos motores a un estado conocido.
    //     Llamar en note-on para que cada nota arranque desde la misma fase.
    void reset()
    {
        polyblep.reset();
        dpw.reset();
    }


private:
    WaveType  waveType = WaveType::Saw;
    OscEngine engine = OscEngine::DPW;   // EN/ES: adopted default / por defecto adoptado

    OscillatorPolyBLEP polyblep;
    OscillatorDPW      dpw;
};


/*
    ============================================================================
    HISTORICAL REFERENCE / REFERENCIA HISTÓRICA
    ============================================================================

    EN: This synth's anti-aliasing engine has gone through three stages.
        Keeping the lineage documented (and the original code preserved)
        is itself a teaching goal of the project.

          (1) BLIT-DSF  -- original engine (preserved as code below)
                Band-Limited Impulse Train via Discrete Summation Formula.
                Generates aliasing-free waveforms with the Dirichlet kernel
                sin(Nx)/sin(x) plus a DC compensation term.
                Replaced because:
                  - Numerical instability near phase = 0 (the
                    phase*phase > 1e-9 guard is a symptom, not a fix).
                  - DC offset that must be subtracted every sample.
                  - Less stable at low frequencies.
                  - Not modular: changing waveform means rederiving the
                    whole summation.

          (2) PolyBLEP  -- replaced BLIT-DSF; now a COMPARATIVE engine
                Adds a small polynomial residual near each discontinuity
                to cancel the first alias components. Clean per-waveform,
                stable at low frequencies, and the professional standard.
                NOT removed: it is the best choice very close to Nyquist
                and serves as the comparative reference for teaching.

          (3) DPW  -- ADOPTED technique (current default engine)
                Differentiated Polynomial Waveform. A single, easily
                derivable idea -- "build a smooth polynomial, then take
                finite differences" -- recovers band-limited saw/triangle,
                and square/PWM fall out of the same engine as "a square is
                two saws subtracted". Chosen for the synth because teaching
                PolyBLEP properly would first require BLIT, BLEP, MinBLEP
                and fractional-delay filters, whereas DPW reaches the same
                pedagogical goal (anti-aliasing in digital oscillators)
                with far less prerequisite machinery.

        The facade pattern is what made the (1) -> (2) -> (3) swap possible
        without touching Voice or the rest of the synth.

        References:
          Stilson, T. & Smith, J. O. (1996). "Alias-Free Digital Synthesis
          of Classic Analog Waveforms". Proc. ICMC.                      [1]
          Välimäki, V. & Huovilainen, A. (2007). "Antialiasing Oscillators
          in Subtractive Synthesis". IEEE Sig. Proc. Mag. 24(2), 116-125. [2]
          Välimäki, V., Nam, J., Smith, J. O. & Abel, J. S. (2010).
          "Alias-Suppressed Oscillators Based on Differentiated Polynomial
          Waveforms". IEEE Trans. ASLP 18(4), 786-798.                   [3]

    ES: El motor de anti-aliasing de este sintetizador pasó por tres etapas.
        Documentar el linaje (y conservar el código original) es en sí mismo
        un objetivo didáctico del proyecto.

          (1) BLIT-DSF  -- motor original (conservado como código abajo)
                Band-Limited Impulse Train vía Discrete Summation Formula.
                Genera formas de onda sin aliasing con el núcleo de
                Dirichlet sin(Nx)/sin(x) más un término de compensación
                de DC. Reemplazado porque:
                  - Inestabilidad numérica cerca de fase = 0 (la guarda
                    phase*phase > 1e-9 es un síntoma, no una solución).
                  - Offset de DC que hay que restar cada muestra.
                  - Menos estable en frecuencias bajas.
                  - No modular: cambiar de onda exige rederivar toda la
                    sumatoria.

          (2) PolyBLEP  -- reemplazó a BLIT-DSF; ahora motor COMPARATIVO
                Suma un residuo polinómico cerca de cada discontinuidad
                para cancelar los primeros componentes de alias. Limpio por
                forma de onda, estable en bajas frecuencias y estándar
                profesional. NO se elimina: es la mejor opción muy cerca de
                Nyquist y sirve como referencia comparativa para enseñar.

          (3) DPW  -- técnica ADOPTADA (motor por defecto actual)
                Differentiated Polynomial Waveform. Una sola idea
                fácilmente derivable -- "construir un polinomio suave y
                tomar diferencias finitas" -- recupera saw/triangle
                band-limited, y square/PWM salen del mismo motor como "un
                square son dos saws restados". Elegida para el sintetizador
                porque enseñar PolyBLEP correctamente exigiría primero
                BLIT, BLEP, MinBLEP y filtros de retardo fraccional,
                mientras que DPW alcanza el mismo objetivo pedagógico
                (anti-aliasing en osciladores digitales) con mucha menos
                maquinaria previa.

        El patrón fachada es lo que hizo posible el intercambio
        (1) -> (2) -> (3) sin tocar Voice ni el resto del sintetizador.

        Referencias:
          Stilson, T. & Smith, J. O. (1996). "Alias-Free Digital Synthesis
          of Classic Analog Waveforms". Proc. ICMC.                      [1]
          Välimäki, V. & Huovilainen, A. (2007). "Antialiasing Oscillators
          in Subtractive Synthesis". IEEE Sig. Proc. Mag. 24(2), 116-125. [2]
          Välimäki, V., Nam, J., Smith, J. O. & Abel, J. S. (2010).
          "Alias-Suppressed Oscillators Based on Differentiated Polynomial
          Waveforms". IEEE Trans. ASLP 18(4), 786-798.                   [3]

    ----------------------------------------------------------------------------
    EN: Preserved original BLIT-DSF implementation (teaching artifact).
    ES: Implementación BLIT-DSF original conservada (artefacto didáctico).
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
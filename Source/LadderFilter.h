/*
  ==============================================================================

    LadderFilter.h
    Created: 11 Feb 2026 2:46:22pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: LadderFilter
    Purpose:
        EN: Digital model of the four-pole Moog ladder lowpass filter,
            based on Huovilainen's non-linear formulation. Provides the
            warm, slightly saturated, self-oscillating character that
            defined the Minimoog and similar instruments.
        ES: Modelo digital del filtro Moog ladder lowpass de cuatro polos,
            basado en la formulación no lineal de Huovilainen. Aporta el
            carácter cálido, ligeramente saturado y auto-oscilante que
            definió al Minimoog y otros instrumentos similares.

    Main responsibilities:
        EN:
          - Run the four cascaded one-pole stages with soft tanh saturation
          - Apply 2x oversampling with linear interpolation to attenuate
            non-linear aliasing introduced by the saturator
          - Compensate the resonance loss that appears at high cutoffs
          - Expose a minimal API (setSampleRate, updateCoefficients, render,
            reset) that an Adapter can wrap into the IFilter contract
        ES:
          - Ejecutar las cuatro etapas one-pole en cascada con saturación
            tanh suave
          - Aplicar oversampling 2x con interpolación lineal para atenuar
            el aliasing no lineal que introduce el saturador
          - Compensar la pérdida de resonancia en cutoffs altos
          - Exponer una API mínima (setSampleRate, updateCoefficients,
            render, reset) que un Adapter pueda envolver en el contrato IFilter

    Algorithm overview / Visión general del algoritmo:
        EN:
          Four one-pole lowpass stages are connected in series. The output
          of the last stage is fed back to the input through a non-linear
          element (tanh), producing the resonance:

              x ──►(+)──► [tanh] ──► [LP1] ──► [LP2] ──► [LP3] ──► [LP4] ──┬──► y
                    ▲                                                       │
                    └──────────────── -r * tanh(y[3]) ◄────────────────────┘

          Each integrator is the simplest possible one-pole IIR:
              y[i] = y[i] + g * (in - y[i])
          The product g·(in - y[i]) is the band-limited approximation of
          the analog d/dt term in the original Moog circuit.

        ES:
          Se conectan en serie cuatro etapas lowpass one-pole. La salida
          de la última se realimenta a la entrada por un elemento no lineal
          (tanh), generando la resonancia:

              x ──►(+)──► [tanh] ──► [LP1] ──► [LP2] ──► [LP3] ──► [LP4] ──┬──► y
                    ▲                                                       │
                    └──────────────── -r * tanh(y[3]) ◄────────────────────┘

          Cada integrador es el IIR de un polo más simple posible:
              y[i] = y[i] + g * (in - y[i])
          El producto g·(in - y[i]) es la aproximación discreta del término
          d/dt analógico del circuito Moog original.

    Reference:
        Huovilainen, A. (2004). "Non-Linear Digital Implementation of the
        Moog Ladder Filter". Proc. DAFx-04, Naples.
        https://dafx.de/paper-archive/2004/P_061.PDF

    Notes:
        EN:
          - The 2x oversampling is essential, not optional: the per-stage
            tanh creates harmonics that would alias back into the audible
            range at the host sample rate.
          - The fast tanh used here is a Padé rational approximation of
            order [3/2]. Indistinguishable from std::tanh in audio contexts
            and ~5x faster.
          - This class deliberately does NOT inherit from IFilter. The
            adapter MoogFilter (see MoogFilter.h) wraps it and exposes
            the IFilter contract. This keeps the DSP class portable to
            projects that do not use the IFilter abstraction.
        ES:
          - El oversampling 2x es esencial, no opcional: el tanh de cada
            etapa crea armónicos que harían aliasing en el rango audible
            a la sample rate del host.
          - El tanh rápido es una aproximación racional de Padé de orden
            [3/2]. Indistinguible de std::tanh en contextos de audio y
            ~5x más rápido.
          - Esta clase NO hereda de IFilter a propósito. El adaptador
            MoogFilter (ver MoogFilter.h) la envuelve y expone el contrato
            IFilter. Esto mantiene a la clase DSP portable a proyectos
            que no usen la abstracción IFilter.
*/

#pragma once

#include <cmath>
#include <algorithm>


class LadderFilter
{
public:
    LadderFilter() = default;


    // EN: Clears all filter state. Call on note-on or preset change to
    //     prevent clicks from leftover energy in the integrators.
    // ES: Limpia todo el estado del filtro. Llamar en note-on o al cambiar
    //     de preset para evitar clicks por energía residual en los integradores.
    void reset()
    {
        std::fill(std::begin(y), std::end(y), 0.0f);
        std::fill(std::begin(tanhY), std::end(tanhY), 0.0f);
        x1 = 0.0f;
    }

    // EN: Updates the sample rate and recalculates coefficients to keep
    //     the cutoff at the same Hz value.
    // ES: Actualiza la sample rate y recalcula coeficientes para mantener
    //     el cutoff en el mismo valor en Hz.
    void setSampleRate(float newSampleRate)
    {
        sampleRate = std::max(1.0f, newSampleRate);
        updateCoefficients(cutoffHz, resonance);
    }

    // EN: Computes all internal coefficients from the user-facing cutoff
    //     and resonance. Should be called per audio block, not per sample.
    //     The math here is the Huovilainen model (DAFx 2004, eqs. 13 & 15).
    // ES: Calcula todos los coeficientes internos a partir del cutoff y
    //     la resonancia que ve el usuario. Debe llamarse por bloque de
    //     audio, no por muestra. La matemática es el modelo de Huovilainen
    //     (DAFx 2004, ecs. 13 y 15).
    void updateCoefficients(float cutoffFreq, float reso01)
    {
        cutoffHz = std::clamp(cutoffFreq, 20.0f, sampleRate * 0.45f);
        resonance = std::clamp(reso01, 0.0f, 1.0f);  // EN: 0 = none, 1 = self-oscillation
        // ES: 0 = nada, 1 = auto-oscilación

// EN: Frequency normalized to the oversampled rate (2x).
// ES: Frecuencia normalizada a la sample rate sobremuestreada (2x).
        const float fs_os = sampleRate * 2.0f;
        float f = cutoffHz / fs_os;
        f = std::clamp(f, 0.0f, 0.49f);

        // EN: Pre-warped cutoff frequency (Huovilainen eq. 13). The bilinear
        //     transform compresses high frequencies; this polynomial
        //     compensates so that the perceived cutoff matches the request.
        // ES: Frecuencia de cutoff pre-warped (Huovilainen ec. 13). La
        //     transformada bilineal comprime las altas frecuencias; este
        //     polinomio compensa para que el cutoff percibido coincida.
        p = f * (1.8f - 0.8f * f);

        // EN: Auxiliary coefficient declared in the original paper but not
        //     used by this simplified one-substep implementation. Kept here
        //     for fidelity with the reference and possible future use.
        // ES: Coeficiente auxiliar declarado en el paper original pero no
        //     usado por esta implementación simplificada de un substep.
        //     Se conserva por fidelidad con la referencia y uso futuro.
        k = 2.0f * p - 1.0f;

        // EN: Frequency-dependent resonance compensation (Huovilainen
        //     eq. 15). At high cutoffs the simple feedback gain "r"
        //     produces less perceived resonance; this rational expression
        //     restores the missing energy.
        // ES: Compensación de resonancia dependiente de la frecuencia
        //     (Huovilainen ec. 15). En cutoffs altos la ganancia de
        //     realimentación "r" simple produce menos resonancia
        //     percibida; esta expresión racional recupera la energía.
        float t = (1.0f - p) * 1.386249f;
        float t2 = 12.0f + t * t;
        r = resonance * (t2 + 6.0f * t) / (t2 - 6.0f * t);

        // EN: Integration coefficient. Capped just below 1.0 to avoid
        //     marginal stability when the filter is asked to track Nyquist.
        // ES: Coeficiente de integración. Limitado justo bajo 1.0 para
        //     evitar inestabilidad marginal cuando se pide rastrear Nyquist.
        g = std::clamp(p, 0.0f, 0.9995f);
    }

    // EN: Processes one input sample. Internally runs the filter twice at
    //     2x rate (midpoint + endpoint) and averages the results, then
    //     stores the input for next-sample interpolation.
    // ES: Procesa una muestra de entrada. Internamente ejecuta el filtro
    //     dos veces a 2x (punto medio + extremo) y promedia, luego guarda
    //     la entrada para la interpolación de la siguiente muestra.
    float render(float x)
    {
        // EN: Linear interpolation between previous and current input
        //     gives the "in-between" sample needed for 2x oversampling.
        // ES: La interpolación lineal entre la entrada anterior y la
        //     actual da la muestra intermedia que necesita el 2x oversampling.
        float x_os0 = 0.5f * (x1 + x);  // EN: midpoint  |  ES: punto medio
        float x_os1 = x;                // EN: endpoint  |  ES: extremo

        float y_os0 = processOneSubstep(x_os0);
        float y_os1 = processOneSubstep(x_os1);

        x1 = x;

        // EN: Decimation by averaging acts as a simple lowpass that drops
        //     the upper half of the oversampled spectrum (anti-aliasing).
        // ES: La diezmación por promedio actúa como un lowpass simple que
        //     descarta la mitad superior del espectro sobremuestreado (anti-alias).
        return 0.5f * (y_os0 + y_os1);
    }

    // EN: Identifies this filter for debugging and GUI display.
    // ES: Identifica este filtro para depuración y para la GUI.
    const char* getFilterType() const
    {
        return "Moog Ladder (Huovilainen)";
    }


private:
    // EN: Fast tanh via Padé rational approximation of order [3/2].
    //     Audio-indistinguishable from std::tanh and roughly 5x faster.
    //     Input is clamped to ±5 because beyond that the rational form
    //     would overshoot and the real tanh is already saturated.
    // ES: tanh rápido vía aproximación racional de Padé orden [3/2].
    //     Indistinguible de std::tanh para audio y ~5x más rápido.
    //     La entrada se limita a ±5 porque más allá la forma racional
    //     se desborda y el tanh real ya está saturado.
    static inline float tanh_approx(float x)
    {
        x = std::clamp(x, -5.0f, 5.0f);
        const float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    // EN: Runs one full pass of the ladder at the oversampled rate.
    //     Called twice per output sample by render().
    // ES: Ejecuta una pasada completa del ladder a la sample rate
    //     sobremuestreada. Lo llama render() dos veces por muestra de salida.
    float processOneSubstep(float x)
    {
        // EN: Feedback from the last stage's saturated output. The minus
        //     sign (the input is x - fb) makes the loop negative, which
        //     is what produces resonance instead of oscillation.
        // ES: Realimentación de la salida saturada de la última etapa.
        //     El signo menos (la entrada es x - fb) hace negativo el lazo,
        //     que es lo que produce resonancia y no oscilación descontrolada.
        const float fb = r * tanhY[3];
        float u = x - fb;
        float in = tanh_approx(u);

        // EN: Four one-pole lowpass stages in series. Each stage saturates
        //     its output (tanh) before feeding the next, which gives the
        //     ladder its characteristic warmth.
        // ES: Cuatro etapas lowpass one-pole en serie. Cada etapa satura
        //     su salida (tanh) antes de alimentar a la siguiente, lo cual
        //     da al ladder su calidez característica.
        for (int i = 0; i < 4; ++i)
        {
            y[i] += g * (in - y[i]);     // EN: one-pole integration  |  ES: integración one-pole
            tanhY[i] = tanh_approx(y[i]); // EN: soft saturation         |  ES: saturación suave
            in = tanhY[i];           // EN: chain to next stage     |  ES: encadenar a la siguiente
        }

        return y[3];
    }


    // --- State / Estado ----------------------------------------------------

    float sampleRate = 44100.0f;
    float cutoffHz = 1000.0f;
    float resonance = 0.0f;

    float y[4] = { 0.0f, 0.0f, 0.0f, 0.0f };  // EN: stage outputs       |  ES: salidas de etapa
    float tanhY[4] = { 0.0f, 0.0f, 0.0f, 0.0f };  // EN: saturated outputs   |  ES: salidas saturadas
    float x1 = 0.0f;                         // EN: previous input      |  ES: entrada anterior


    // --- Huovilainen coefficients / Coeficientes de Huovilainen ------------

    float g = 0.0f;  // EN: integration coefficient        |  ES: coeficiente de integración
    float p = 0.0f;  // EN: pre-warped normalized cutoff   |  ES: cutoff normalizado pre-warped
    float k = 0.0f;  // EN: auxiliary (unused, see notes)  |  ES: auxiliar (sin uso, ver notas)
    float r = 0.0f;  // EN: resonance feedback gain        |  ES: ganancia de realimentación
};
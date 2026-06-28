/*
  ==============================================================================

    SVF.h
    Created: 5 Feb 2026 2:21:41pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: SVF
    Purpose:
        EN: State Variable Filter with topology-preserving (Zero-Delay
            Feedback) trapezoidal integrators. Provides a clean, neutral
            lowpass character that complements the warm, saturated sound
            of the Moog ladder.
        ES: Filtro State Variable con integradores trapezoidales que
            preservan la topología (Zero-Delay Feedback). Aporta un
            carácter lowpass limpio y neutro que complementa al sonido
            cálido y saturado del Moog ladder.

    Main responsibilities:
        EN:
          - Pre-warp the cutoff through the bilinear transform (tan warp)
          - Solve the implicit feedback loop in closed form each sample
          - Advance two integrator-capacitor state variables per sample
          - Expose a minimal API that an Adapter can wrap into IFilter
        ES:
          - Pre-warpear el cutoff con la transformada bilineal (tan warp)
          - Resolver el lazo implícito de realimentación en forma cerrada
            cada muestra
          - Avanzar las dos variables de estado "integrator capacitor"
            por muestra
          - Exponer una API mínima que un Adapter pueda envolver en IFilter

    Algorithm overview / Visión general del algoritmo:
        EN:
          Classic SVF topology (Chamberlin) has a one-sample delay in the
          feedback path, which makes it unstable near Nyquist. This
          implementation uses Zero-Delay Feedback: the implicit equations

                  v1 = integrator(  k·v1 + v2 + x  )
                  v2 = integrator(  v1             )

          are solved algebraically so the loop closes within the same
          sample. The result is the signal flow:

                             ┌────────────────────────┐
                             │                        │
              x ──►(+)──► v3 │──► a2·(·) ──►(+)──► v1 │
                    ▲        │                │       │
                    │        │                ▼       │
                    │        │             [ic1eq]    │
                    │        │                │       │
                    └────────┤──► a3·(·) ──►(+)──► v2 ──► y
                             │                │
                             │             [ic2eq]
                             └────────────────┘

          Output v2 is the lowpass signal. Bandpass (v1) and highpass
          (x - k·v1 - v2) are available from the same intermediate
          variables if multi-mode output is ever needed.

        ES:
          La topología SVF clásica (Chamberlin) tiene una muestra de
          retardo en la realimentación, lo que la vuelve inestable cerca
          de Nyquist. Esta implementación usa Zero-Delay Feedback: las
          ecuaciones implícitas

                  v1 = integrador(  k·v1 + v2 + x  )
                  v2 = integrador(  v1             )

          se resuelven algebraicamente para que el lazo cierre en la
          misma muestra. El diagrama de señal queda como arriba.
          La salida v2 es el lowpass. Bandpass (v1) y highpass
          (x - k·v1 - v2) están disponibles con las mismas variables
          intermedias si alguna vez se quiere salida multi-modo.

    Reference:
        Simper, A. (2013). "Linear Trapezoidal State Variable Filter SVF
        in state increment form: state += val". Cytomic Technical Papers.
        https://cytomic.com/files/dsp/SvfLinearTrapezoidalSin.pdf

    Notes:
        EN:
          - The pre-warp  g = tan(π·f/fs)  compensates for the bilinear
            transform's frequency compression, so the cutoff in Hz is
            accurate across the audible range.
          - Q is the classical quality factor: Q = 0.707 is Butterworth
            (maximally flat), Q > 3 starts producing audible peaks,
            Q ≈ 10 produces high resonance and prolonged ringing.
          - This class does NOT inherit from IFilter. The adapter
            SVFFilter (see SVFFilter.h) wraps it and exposes the IFilter
            contract, handling the resonance [0, 1] → Q conversion.
        ES:
          - El pre-warp  g = tan(π·f/fs)  compensa la compresión de
            frecuencias de la transformada bilineal, para que el cutoff
            en Hz sea preciso en todo el rango audible.
          - Q es el factor de calidad clásico: Q = 0.707 es Butterworth
            (máximamente plano), Q > 3 empieza a producir picos audibles,
            Q ≈ 10 produce alta resonancia y ringing prolongado.
          - Esta clase NO hereda de IFilter. El adaptador SVFFilter
            (ver SVFFilter.h) la envuelve y expone el contrato IFilter,
            además de convertir resonance [0, 1] → Q.
*/

#pragma once
#include <cmath>
#include "Constants.h"
#include <algorithm>


class SVF
{
public:
    // EN: Stores the host sample rate and clears state. Coefficients are
    //     NOT updated here; call updateCoefficients afterwards.
    // ES: Guarda la sample rate del host y limpia el estado. Los coeficientes
    //     NO se actualizan aquí; llamar updateCoefficients después.
    void prepare(float sr)
    {
        sampleRate = sr;
        reset();
    }

    // EN: Computes all filter coefficients from the user-facing cutoff
    //     (Hz) and Q. Should be called per audio block, not per sample.
    // ES: Calcula todos los coeficientes a partir del cutoff (Hz) y Q
    //     que ve el usuario. Debe llamarse por bloque, no por muestra.
    void updateCoefficients(float cutoff, float Q)
    {
        cutoff = std::clamp(cutoff, 20.0f, 0.45f * sampleRate);
        Q = std::clamp(Q, 0.707f, 10.0f);

        // EN: Pre-warp via the bilinear transform: maps the analog
        //     frequency π·f/fs into its digital equivalent. Without this,
        //     the perceived cutoff would shift as f approaches Nyquist.
        // ES: Pre-warp vía transformada bilineal: mapea la frecuencia
        //     analógica π·f/fs a su equivalente digital. Sin esto, el
        //     cutoff percibido se desplazaría al acercarse a Nyquist.
        g = std::tan(PI * cutoff / sampleRate);

        // EN: Damping coefficient derived from Q. Lower k = sharper peak.
        // ES: Coeficiente de amortiguación derivado de Q. Menor k = pico más agudo.
        k = 1.0f / Q;

        // EN: Closed-form solution of the ZDF implicit loop. These three
        //     coefficients make it possible to run the loop in one shot
        //     per sample instead of iterating.
        // ES: Solución en forma cerrada del lazo implícito ZDF. Estos
        //     tres coeficientes permiten ejecutar el lazo de una vez por
        //     muestra en vez de iterar.
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }

    // EN: Clears integrator state. Call on note-on or preset change.
    // ES: Limpia el estado de los integradores. Llamar en note-on o
    //     al cambiar de preset.
    void reset()
    {
        ic1eq = 0.0f;
        ic2eq = 0.0f;
    }

    // EN: Processes one audio sample and returns the lowpass output.
    //     The intermediate variables v1 (bandpass) and x - k·v1 - v2
    //     (highpass) are discarded here but could be exposed if the
    //     synth ever needs multi-mode filtering.
    // ES: Procesa una muestra y devuelve la salida lowpass. Las variables
    //     intermedias v1 (bandpass) y x - k·v1 - v2 (highpass) se descartan
    //     aquí pero podrían exponerse si el synth necesitara filtrado
    //     multi-modo.
    float render(float x)
    {
        // EN: v3 is the input minus the lowpass output (equivalent to a
        //     highpass of the previous sample). The solved form of the
        //     loop expresses v1 and v2 in terms of v3 and the stored
        //     integrator-equivalent states.
        // ES: v3 es la entrada menos la salida lowpass (equivalente a un
        //     highpass de la muestra anterior). La forma resuelta del lazo
        //     expresa v1 y v2 en términos de v3 y los estados equivalentes
        //     de los integradores.
        float v3 = x - ic2eq;
        float v1 = a1 * ic1eq + a2 * v3;
        float v2 = ic2eq + a2 * ic1eq + a3 * v3;

        // EN: Trapezoidal integrator update. The form  2v - state  is the
        //     the standard TPT (Topology-Preserving Transform) rule used to
        //     preserve the analog topology and obtain a well-behaved digital
        //     response.
        // ES: Actualización del integrador trapezoidal. La forma  2v - estado
        //     es la regla estándar TPT (Topology-Preserving Transform) usada para
        //     preservar la topología analógica y obtener una respuesta digital
        //     bien comportada.
        ic1eq = 2.0f * v1 - ic1eq;
        ic2eq = 2.0f * v2 - ic2eq;

        return v2;  // EN: lowpass output  |  ES: salida lowpass
    }


private:
    float sampleRate = 48000.0f;

    // EN: Cached coefficients, recomputed only when cutoff or Q changes.
    //     g  = pre-warped cutoff
    //     k  = damping factor (1/Q)
    //     a1, a2, a3 = closed-form ZDF solution coefficients
    // ES: Coeficientes cacheados, recalculados solo cuando cambian cutoff
    //     o Q.
    //     g  = cutoff pre-warped
    //     k  = factor de amortiguación (1/Q)
    //     a1, a2, a3 = coeficientes de la solución ZDF en forma cerrada
    float g = 0.0f, k = 0.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;

    // EN: Integrator-equivalent state variables. "ic" stands for
    //     "integrator capacitor": these are the ZDF analogues of the
    //     voltages stored on the analog circuit's capacitors between
    //     samples.
    // ES: Variables de estado equivalentes de los integradores. "ic"
    //     significa "integrator capacitor": son los equivalentes ZDF de
    //     los voltajes almacenados en los condensadores del circuito
    //     analógico entre muestras.
    float ic1eq = 0.0f, ic2eq = 0.0f;
};
/*
  ==============================================================================

    OscillatorDPW.h
    Created: 19 May 2026 2:24:58pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: OscillatorDPW
    Purpose:
        EN: Differentiated Polynomial Waveform (DPW) oscillator engine.
            Full alias-suppressed waveform set (sine, saw, square,
            squarePWM, triangle). This is the anti-aliasing technique
            ADOPTED by the synth for its pedagogical accessibility;
            OscillatorPolyBLEP is kept as a swappable alternative for the
            comparative module (see Oscillator.h, OscEngine).
        ES: Motor de oscilador basado en Differentiated Polynomial Waveforms
            (DPW). Conjunto completo de ondas con aliasing reducido (sine,
            saw, square, squarePWM, triangle). Es la tecnica de
            anti-aliasing ADOPTADA por el sintetizador por su accesibilidad
            pedagogica; OscillatorPolyBLEP se conserva como alternativa
            conmutable para el modulo comparativo (ver Oscillator.h,
            OscEngine).

    Main responsibilities:
        EN:
          - Advance and wrap the internal phase at the set frequency
          - Saw      -> DPW4: p(x) = x^4 - 2x^2, 3rd finite difference
          - Triangle -> DPW2: p(x) = x*(|x| - 1), 1st finite difference
          - Square   -> difference of two phase-shifted band-limited saws
          - SquarePWM-> same difference, with the shift = pulse width
          - Sine     -> direct sin() (already band-limited)
          - Support hard phase-sync between two oscillators
        ES:
          - Avanzar y envolver la fase interna a la frecuencia fijada
          - Saw      -> DPW4: p(x) = x^4 - 2x^2, tercera diferencia finita
          - Triangle -> DPW2: p(x) = x*(|x| - 1), primera diferencia finita
          - Square   -> resta de dos saws band-limited desfasados
          - SquarePWM-> misma resta, con el desfase = ancho de pulso
          - Sine     -> sin() directo (ya band-limited)
          - Permitir sync duro de fase entre dos osciladores

    Architectural role:
        EN: Owned privately by the Oscillator facade (see Oscillator.h)
            together with OscillatorPolyBLEP. DPW was chosen over
            BLEP/PolyBLEP because the latter requires teaching BLIT, BLEP
            and MinBLEP first, plus fractional-delay filters; DPW reaches
            the same pedagogical goal with a single, easily derivable
            "polynomial + finite-difference" idea, and square/PWM fall out
            of the same engine as "a square is two saws subtracted".
        ES: Propiedad privada de la fachada Oscillator (ver Oscillator.h)
            junto a OscillatorPolyBLEP. Se eligio DPW sobre BLEP/PolyBLEP
            porque este ultimo exige ensenar primero BLIT, BLEP y MinBLEP,
            mas filtros de retardo fraccional; DPW alcanza el mismo objetivo
            pedagogico con una sola idea facilmente derivable: "polinomio +
            diferencia finita", y square/PWM salen del mismo motor como
            "un square son dos saws restados".

    Notes:
        EN:
          - DPW builds a polynomial that is C^(N-1)-continuous across the
            phase wrap, so the discontinuity only appears in its N-th
            derivative. Differentiating (N-1) times recovers an approximation 
            of the classic waveform with reduced aliasing.
          - Differencing two band-limited saws yields a band-limited square
            with ZERO DC for free (no separate DC-correction term needed,
            unlike a naive +/-1 pulse).
          - DPW degrades gracefully but does alias more than PolyBLEP very
            close to Nyquist (very high notes). For that regime the
            comparative PolyBLEP/Hybrid engines exist (Oscillator.h).
          - All state is plain floats; no dynamic allocation, RT-safe.
        ES:
          - DPW construye un polinomio C^(N-1)-continuo a traves del wrap de
            fase, de modo que la discontinuidad solo aparece en su N-esima
            derivada. Diferenciar (N-1) veces recupera una onda clasica
            band-limited.
          - Restar dos saws band-limited da un square band-limited con
            media (DC) CERO gratis (no hace falta termino de correccion de
            DC, a diferencia de un pulso ingenuo +/-1).
          - DPW degrada suavemente pero produce mas aliasing que PolyBLEP
            muy cerca de Nyquist (notas muy agudas). Para ese regimen estan
            los motores comparativos PolyBLEP/Hibrido (Oscillator.h).
          - Todo el estado son floats simples; sin reservas dinamicas,
            seguro para el callback de audio en tiempo real.

    Reference:
        Valimaki, V., Nam, J., Smith, J. O. & Abel, J. S. (2010).
        "Alias-Suppressed Oscillators Based on Differentiated Polynomial
        Waveforms". IEEE Transactions on Audio, Speech, and Language
        Processing, 18(4), pp. 786-798.

    See also / Vease tambien:
        OscillatorDPW.cpp    - DSP implementation and scaling derivation
        OscillatorPolyBLEP.h - swappable comparative engine
        Oscillator.h         - facade class and engine selector (OscEngine)
*/

#pragma once

#include <cmath>
#include <algorithm>
#include "Constants.h"


class OscillatorDPW
{
public:
    // EN: Stores the host sample rate and recalculates internal increments.
    //     Must be called before the first sample-generating method.
    // ES: Guarda la sample rate del host y recalcula los incrementos internos.
    //     Debe llamarse antes del primer metodo que genere muestras.
    void prepare(double sampleRate);

    // EN: Sets the oscillator frequency in Hz and refreshes phaseInc.
    //     Negative values are clamped to 0 Hz.
    // ES: Fija la frecuencia del oscilador en Hz y actualiza phaseInc.
    //     Los valores negativos se limitan a 0 Hz.
    void setFrequency(float freq);

    // EN: Sets the pulse width used by squarePWM. Clamped to [0.05, 0.95]
    //     to avoid degenerate waveforms near silence. Mirrors the
    //     OscillatorPolyBLEP range so both engines behave identically.
    // ES: Fija el ancho de pulso usado por squarePWM. Limitado a
    //     [0.05, 0.95] para evitar formas degeneradas cerca del silencio.
    //     Replica el rango de OscillatorPolyBLEP para que ambos motores se
    //     comporten igual.
    void setPulseWidth(float width)
    {
        pulseWidth = std::clamp(width, 0.05f, 0.95f);
    }

    // EN: Resets phase and every finite-difference delay tap to zero.
    // ES: Reinicia la fase y todos los taps de diferencias finitas a cero.
    void reset();

    // EN: Copies another DPW oscillator's phase and full difference history
    //     into this one. Used by Oscillator::syncPhase for hard-sync.
    // ES: Copia la fase y todo el historial de diferencias de otro oscilador
    //     DPW en este. Lo usa Oscillator::syncPhase para el hard-sync.
    void syncPhase(const OscillatorDPW& other);


    // --- Waveform generators / Generadores de forma de onda ----------------
    // EN: Each method returns one normalized (~+/-1) sample and internally
    //     advances the phase. Amplitudes are matched to OscillatorPolyBLEP
    //     so switching engine or waveform never jumps in level.
    // ES: Cada metodo devuelve una muestra normalizada (~+/-1) y avanza la
    //     fase internamente. Las amplitudes estan emparejadas con
    //     OscillatorPolyBLEP para que cambiar de motor o de onda no salte.

    // EN: Pure sine. Already band-limited, no DPW correction needed.
    // ES: Sinusoide pura. Ya band-limited, no necesita correccion DPW.
    float sine();

    // EN: One sample of a fourth-order DPW sawtooth (DPW4).
    // ES: Una muestra de sawtooth DPW de cuarto orden (DPW4).
    float saw();

    // EN: Square as the difference of two band-limited saws half a period
    //     apart. Same DPW4 engine, zero DC by construction.
    // ES: Square como resta de dos saws band-limited a medio periodo.
    //     Mismo motor DPW4, DC cero por construccion.
    float square();

    // EN: Pulse-width square: difference of two band-limited saws shifted
    //     by `pulseWidth`. Zero DC for any duty cycle, no extra term.
    // ES: Square con ancho de pulso: resta de dos saws band-limited
    //     desfasados `pulseWidth`. DC cero para cualquier ciclo de
    //     trabajo, sin termino extra.
    float squarePWM();

    // EN: One sample of a second-order DPW triangle (DPW2).
    // ES: Una muestra de triangle DPW de segundo orden (DPW2).
    float triangle();

    // EN: Default sample output. Mirrors OscillatorPolyBLEP::nextSample():
    //     returns saw(), the richest harmonic starting point for
    //     subtractive synthesis. Kept for interface parity.
    // ES: Salida de muestra por defecto. Replica
    //     OscillatorPolyBLEP::nextSample(): devuelve saw(), el punto
    //     armonico mas rico para sintesis sustractiva. Paridad de interfaz.
    float nextSample();


private:
    // --- Runtime state / Estado en ejecucion -------------------------------

    // EN: Host sample rate. Default 48 kHz is replaced by prepare().
    // ES: Sample rate del host. El 48 kHz por defecto lo reemplaza prepare().
    float sampleRate = 48000.0f;

    // EN: Normalized phase in [0, 1). Wraps around on each cycle.
    // ES: Fase normalizada en [0, 1). Se envuelve en cada ciclo.
    float phase = 0.0f;

    // EN: Phase increment per sample = frequency / sampleRate.
    // ES: Incremento de fase por muestra = frequency / sampleRate.
    float phaseInc = 0.0f;

    // EN: Current oscillator frequency in Hz.
    // ES: Frecuencia actual del oscilador en Hz.
    float frequency = 440.0f;

    // EN: Pulse width for squarePWM in [0.05, 0.95]. Clamped by setter.
    // ES: Ancho de pulso para squarePWM en [0.05, 0.95]. Limitado en el set.
    float pulseWidth = 0.5f;


    // --- DPW history / Historial DPW ---------------------------------------
    // EN: Each waveform that uses finite differences keeps its OWN delay
    //     taps so switching waveform mid-note does not cross-contaminate.
    // ES: Cada onda que usa diferencias finitas mantiene SUS propios taps de
    //     retardo para no contaminarse al cambiar de onda en mitad de nota.

    // EN: Saw DPW4 history (3 taps for the 3rd finite difference).
    // ES: Historial saw DPW4 (3 taps para la tercera diferencia finita).
    float z1 = 0.0f, z2 = 0.0f, z3 = 0.0f;

    // EN: Square DPW4 history (operates on the saw-difference signal).
    // ES: Historial square DPW4 (opera sobre la senal saw-diferencia).
    float sqZ1 = 0.0f, sqZ2 = 0.0f, sqZ3 = 0.0f;

    // EN: SquarePWM DPW4 history.
    // ES: Historial squarePWM DPW4.
    float pwZ1 = 0.0f, pwZ2 = 0.0f, pwZ3 = 0.0f;

    // EN: Triangle DPW2 history (single tap for the 1st difference).
    // ES: Historial triangle DPW2 (un tap para la primera diferencia).
    float triZ1 = 0.0f;


    // --- Internal DSP / DSP interno ----------------------------------------

    // EN: Normalized phase -> bipolar phase [-1, 1). Consecutive samples
    //     are spaced dx = 2*phaseInc in x (relevant for scaling).
    // ES: Fase normalizada -> bipolar [-1, 1). Muestras consecutivas distan
    //     dx = 2*phaseInc en x (relevante para el escalado).
    float bipolarPhase() const;

    // EN: Bipolar value for an arbitrary normalized phase value p.
    // ES: Valor bipolar para una fase normalizada arbitraria p.
    float bipolarAt(float p) const;

    // EN: Wraps an arbitrary phase value into [0, 1).
    // ES: Envuelve una fase arbitraria a [0, 1).
    float wrap01(float p) const;

    // EN: Advances phase by phaseInc and wraps it into [0, 1).
    // ES: Avanza la fase en phaseInc y la envuelve a [0, 1).
    void advance();

    // EN: Fourth-order DPW saw polynomial: p(x) = x^4 - 2x^2.
    // ES: Polinomio DPW de saw de cuarto orden: p(x) = x^4 - 2x^2.
    float sawPolynomial4(float x) const;

    // EN: Third backward finite difference (recovers the saw from p).
    // ES: Tercera diferencia finita regresiva (recupera el saw desde p).
    float difference3(float x, float& d1, float& d2, float& d3);

    // EN: Analytical amplitude scaling for DPW4. Derivation in the .cpp.
    // ES: Escalado analitico de amplitud para DPW4. Derivacion en el .cpp.
    float sawScale4() const;
};
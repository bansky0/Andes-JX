/*
  ==============================================================================

    OscillatorPolyBLEP.cpp
    Created: 21 Nov 2025 11:39:49am
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: OscillatorPolyBLEP (implementation)
    Purpose:
        EN: DSP implementation of the band-limited oscillator declared in
            OscillatorPolyBLEP.h. Contains the phase advancement, the
            PolyBLEP polynomial correction, and the five waveform generators.
        ES: Implementación DSP del oscilador band-limited declarado en
            OscillatorPolyBLEP.h. Contiene el avance de fase, la corrección
            polinómica PolyBLEP y los cinco generadores de forma de onda.

    Algorithm overview / Visión general del algoritmo:
        EN: A naive digital saw or square produced by direct computation
            aliases heavily because its spectrum extends beyond Nyquist.
            PolyBLEP fixes this by adding a small polynomial residual near
            each discontinuity that cancels the first two alias components
            (order-2 correction). The residual is cheap (two multiplies)
            and evaluated only within ±1 sample of the discontinuity.

        ES: Una saw o square digital ingenua produce aliasing porque su
            espectro se extiende más allá de Nyquist. PolyBLEP lo corrige
            sumando un residuo polinómico cerca de cada discontinuidad que
            cancela los dos primeros componentes de alias (corrección de
            orden 2). El residuo es barato (dos multiplicaciones) y se
            evalúa solo dentro de ±1 muestra de la discontinuidad.

    Reference:
        Välimäki, V. & Huovilainen, A. (2007). "Antialiasing Oscillators
        in Subtractive Synthesis". IEEE Signal Processing Magazine, 24(2),
        pp. 116-125.

    See also / Véase también:
        OscillatorPolyBLEP.h - interface declarations / declaraciones
        Oscillator.h         - facade class and historical BLIT-DSF version
                               fachada y versión histórica con BLIT-DSF
*/

#include "OscillatorPolyBLEP.h"
#include <cmath>


// ============================================================================
//  Helpers
// ============================================================================

// EN: Advances the normalized phase by one sample and wraps it into [0, 1).
//     Returns the updated phase. Using a normalized [0, 1) range (instead
//     of [0, 2π)) simplifies the PolyBLEP discontinuity math.
// ES: Avanza la fase normalizada una muestra y la envuelve en [0, 1).
//     Devuelve la fase actualizada. Usar rango normalizado [0, 1) (en
//     lugar de [0, 2π)) simplifica las matemáticas de discontinuidad.
float OscillatorPolyBLEP::advance()
{
    phase += phaseInc;

    if (phase >= 1.0f) {
        phase -= 1.0f;
    }
    return phase;
}


// EN: Returns the PolyBLEP residual to be added (or subtracted) at a
//     discontinuity. Parameter `t` is the normalized phase in [0, 1).
//
//     The function evaluates a piecewise quadratic polynomial valid only
//     in the ±dt neighborhood of the discontinuity, where dt = f / fs is
//     the per-sample phase increment. Elsewhere the residual is zero.
//
//     The exact polynomial forms
//         rising  edge (t < dt):       2t - t² - 1
//         falling edge (t > 1 - dt):   t² + 2t + 1
//     are derived in Välimäki & Huovilainen (2007) and yield an order-2
//     correction of the ideal band-limited step.
//
// ES: Devuelve el residuo PolyBLEP a sumar (o restar) en una discontinuidad.
//     El parámetro `t` es la fase normalizada en [0, 1).
//
//     La función evalúa un polinomio cuadrático a trozos válido solo en
//     la vecindad ±dt de la discontinuidad, donde dt = f / fs es el
//     incremento de fase por muestra. Fuera de ahí el residuo es cero.
//
//     Las formas exactas
//         flanco ascendente  (t < dt):     2t - t² - 1
//         flanco descendente (t > 1 - dt): t² + 2t + 1
//     se derivan en Välimäki & Huovilainen (2007) y producen una
//     corrección de orden 2 del step band-limited ideal.
float OscillatorPolyBLEP::polyBLEP(float t) const
{
    float dt = frequency / sampleRate;
    dt = std::min(dt, 0.5f);    // EN: guard against absurd dt if f > fs/2
    // ES: protección ante dt absurdo si f > fs/2

// --- rising edge / flanco ascendente ---
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0f;   // 2t - t² - 1
    }
    // --- falling edge / flanco descendente ---
    else if (t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;   // t² + 2t + 1
    }

    return 0.0f;
}


// ============================================================================
//  Public API
// ============================================================================

// EN: Host notifies the sample rate here. Phase and integrator are zeroed
//     so every new session starts from a known state.
// ES: El host notifica aquí la sample rate. Se ponen a cero la fase y el
//     integrador para que cada sesión arranque desde un estado conocido.
void OscillatorPolyBLEP::prepare(double newSampleRate)
{
    sampleRate = (float)newSampleRate;
    phase = 0.0f;
    integrator = 0.0f;
}

// EN: Updates the target frequency. The per-sample increment is recomputed
//     lazily inside advance() and polyBLEP() on demand.
// ES: Actualiza la frecuencia objetivo. El incremento por muestra se
//     recalcula de forma perezosa dentro de advance() y polyBLEP().
void OscillatorPolyBLEP::setFrequency(float newFreq)
{
    frequency = newFreq;
    phaseInc = frequency / sampleRate;
}


// ============================================================================
//  Waveform generators
// ============================================================================

// EN: Pure sine. Already band-limited by construction, so no PolyBLEP
//     correction is needed.
// ES: Sinusoide pura. Por construcción ya es band-limited, así que no
//     requiere corrección PolyBLEP.
float OscillatorPolyBLEP::sine()
{
    float out = std::sin(TWO_PI * phase);
    advance();
    return out;
}


// EN: Sawtooth wave: a naive ramp 2t-1 plus the PolyBLEP residual at
//     the single discontinuity (phase wrap-around at t = 0/1).
// ES: Onda sawtooth: rampa ingenua 2t-1 más el residuo PolyBLEP en la
//     única discontinuidad (envoltura de fase en t = 0/1).
float OscillatorPolyBLEP::saw()
{
    float t = phase;

    float value = 2.0f * t - 1.0f;   // naive saw / saw ingenua
    value += polyBLEP(t);            // correction at wrap / corrección en el wrap

    advance();
    return value;
}


// EN: Square wave: two discontinuities per cycle (at t = 0 and t = 0.5).
//     The PolyBLEP residual is added at the rising edge and subtracted
//     at the falling edge (which is detected by shifting the phase by 0.5).
// ES: Onda square: dos discontinuidades por ciclo (en t = 0 y t = 0.5).
//     El residuo PolyBLEP se suma en el flanco ascendente y se resta en el
//     descendente (detectado desplazando la fase 0.5).
float OscillatorPolyBLEP::square()
{
    float t = phase;
    float value = (t < 0.5f) ? 1.0f : -1.0f;   // naive square / square ingenua
    value += polyBLEP(t);                       // rising edge / flanco ascendente

    // EN: Shifted phase for the falling edge located at t = 0.5.
    // ES: Fase desplazada para el flanco descendente ubicado en t = 0.5.
    float t2 = t + 0.5f;
    if (t2 >= 1.0f) t2 -= 1.0f;
    value -= polyBLEP(t2);

    advance();
    return value;
}


// EN: Triangle wave, derived by integrating a band-limited square.
//     A plain integrator would drift (accumulate DC), so we use a
//     leaky integrator:
//         integrator = integrator * 0.9999 + square * k
//     The leak factor 0.9999 keeps low frequencies almost intact while
//     preventing DC buildup. The gain k = 4·f/fs compensates for the
//     1/f amplitude attenuation that integration introduces.
// ES: Onda triangle, derivada integrando una square band-limited.
//     Un integrador puro derivaría (acumula DC), por eso usamos un
//     integrador leaky:
//         integrator = integrator * 0.9999 + square * k
//     El factor de fuga 0.9999 preserva las bajas frecuencias casi
//     intactas a la vez que evita acumulación de DC. La ganancia
//     k = 4·f/fs compensa la atenuación 1/f que introduce la integración.
float OscillatorPolyBLEP::triangle()
{
    float t = phase;
    float value = (t < 0.5f) ? 1.0f : -1.0f;
    value += polyBLEP(t);
    value -= polyBLEP(fmod(t + 0.5f, 1.0f));

    float k = 4.0f * frequency / sampleRate;
    integrator = integrator * 0.9999f + value * k;

    advance();
    return integrator;
}


// EN: Pulse-width-modulated square. Both edges need their own PolyBLEP
//     correction because the falling edge is now at t = pulseWidth, not
//     at t = 0.5. A DC-correction term keeps the output centered when
//     the duty cycle is asymmetric.
// ES: Cuadrada con ancho de pulso variable. Ambos flancos necesitan su
//     propia corrección PolyBLEP porque el flanco descendente está ahora
//     en t = pulseWidth, no en t = 0.5. Un término de corrección de DC
//     mantiene la salida centrada cuando el ciclo de trabajo es asimétrico.
float OscillatorPolyBLEP::squarePWM()
{
    float t = phase;

    // EN: Naive pulse wave: high for a fraction `pulseWidth` of the cycle.
    // ES: Onda pulso ingenua: alta durante una fracción `pulseWidth` del ciclo.
    float value = (t < pulseWidth) ? 1.0f : -1.0f;

    // EN: PolyBLEP correction at the rising edge (t = 0).
    // ES: Corrección PolyBLEP en el flanco ascendente (t = 0).
    value += polyBLEP(t);

    // EN: Phase aligned to the falling edge at t = pulseWidth.
    // ES: Fase alineada al flanco descendente en t = pulseWidth.
    float t2 = t + (1.0f - pulseWidth);
    if (t2 >= 1.0f) t2 -= 1.0f;
    value -= polyBLEP(t2);

    // EN: DC offset compensation for asymmetric duty cycles. Without
    //     this, pulseWidth != 0.5 produces an audible DC bias.
    // ES: Compensación de offset de DC para ciclos de trabajo asimétricos.
    //     Sin esto, pulseWidth != 0.5 produce un sesgo de DC audible.
    value -= (2.0f * pulseWidth - 1.0f);

    advance();
    return value;
}


// EN: Hard phase sync. Copies the other oscillator's phase into this one
//     so both restart together each cycle of the master.
// ES: Sync duro de fase. Copia la fase del otro oscilador en este para
//     que ambos reinicien juntos en cada ciclo del maestro.
void OscillatorPolyBLEP::syncPhase(const OscillatorPolyBLEP& other)
{
    phase = other.phase;
}


// EN: Default sample output. Used when the high-level facade has not
//     selected a specific waveform. Saw is chosen because it is the
//     richest harmonic starting point for subtractive synthesis.
// ES: Salida de muestra por defecto. Se usa cuando la fachada de alto
//     nivel no ha seleccionado una forma de onda específica. Se elige
//     saw por ser el punto armónico más rico para síntesis sustractiva.
float OscillatorPolyBLEP::nextSample()
{
    return saw();
}
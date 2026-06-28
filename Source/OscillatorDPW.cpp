/*
  ==============================================================================

    OscillatorDPW.cpp
    Created: 19 May 2026 2:24:58pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: OscillatorDPW (implementation)
    Purpose:
        EN: DSP implementation of the alias-suppressed oscillator declared
            in OscillatorDPW.h. Contains the phase advancement, the DPW
            polynomials, the finite-difference stages, the analytical
            amplitude scaling, and the "square = difference of two saws"
            construction for square / squarePWM.
        ES: Implementacion DSP del oscilador con aliasing reducido declarado
            en OscillatorDPW.h. Contiene el avance de fase, los polinomios
            DPW, las etapas de diferencias finitas, el escalado analitico de
            amplitud y la construccion "square = resta de dos saws" para
            square / squarePWM.

    Algorithm overview / Vision general del algoritmo:
        EN: A naive digital saw/square aliases because its spectrum extends
            beyond Nyquist. DPW avoids this by NOT generating the
            discontinuous waveform directly. It builds a smooth polynomial
            p(x) of the bipolar phase x in [-1, 1) that is
            C^(N-1)-continuous across the wrap, so the discontinuity only
            appears in its N-th derivative. Differentiating p(x) numerically 
            (N-1) times recovers an approximation of the classic waveform 
            with significantly reduced aliasing:

              - saw      -> DPW4: p(x) = x^4 - 2x^2, 3rd finite difference
              - triangle -> DPW2: p(x) = x*(|x| - 1), 1st finite difference

            Square and squarePWM reuse the SAME DPW4 saw engine via the
            identity (the finite difference is linear and time-invariant):

              square(t) = saw(t - d) - saw(t)
              =>  diff3( p(x_b) - p(x_a) ) = diff3(p(x_b)) - diff3(p(x_a))

            with d = 0.5 for the square and d = pulseWidth for the PWM
            pulse. Subtracting two saws also cancels the DC automatically,
            so no separate DC-correction term is needed.

        ES: Una saw/square digital ingenua produce aliasing porque su
            espectro supera Nyquist. DPW lo evita NO generando la onda
            discontinua directamente. Construye un polinomio suave p(x) de
            la fase bipolar x en [-1, 1), C^(N-1)-continuo en el wrap, de
            modo que la discontinuidad solo aparece en su N-esima derivada.
            Diferenciar p(x) numericamente (N-1) veces recupera la onda
            clasica, band-limited por construccion:

              - saw      -> DPW4: p(x) = x^4 - 2x^2, tercera diferencia
              - triangle -> DPW2: p(x) = x*(|x| - 1), primera diferencia

            Square y squarePWM reutilizan el MISMO motor DPW4 del saw via la
            identidad (la diferencia finita es lineal e invariante):

              square(t) = saw(t - d) - saw(t)
              =>  diff3( p(x_b) - p(x_a) ) = diff3(p(x_b)) - diff3(p(x_a))

            con d = 0.5 para el square y d = pulseWidth para el pulso PWM.
            Restar dos saws cancela ademas el DC automaticamente, asi que no
            hace falta termino de correccion de DC.

    Scaling note / Nota de escalado:
        EN: The bipolar phase is x = 2*phase - 1, so consecutive samples are
            spaced dx = 2*phaseInc in x, NOT phaseInc. Every finite
            difference introduces a gain proportional to dx; the scaling
            factors divide it back out. Using phaseInc instead of dx leaves
            the saw 2^3 = 8x too loud and the triangle 2x too loud, breaking
            level-matching with the PolyBLEP engine.
        ES: La fase bipolar es x = 2*phase - 1, asi que las muestras
            consecutivas distan dx = 2*phaseInc en x, NO phaseInc. Cada
            diferencia finita introduce una ganancia proporcional a dx; los
            factores de escalado la deshacen. Usar phaseInc en vez de dx
            deja la saw 8x y el triangle 2x mas fuertes, rompiendo el
            emparejamiento de nivel con el motor PolyBLEP.

    Reference:
        Valimaki, V., Nam, J., Smith, J. O. & Abel, J. S. (2010).
        "Alias-Suppressed Oscillators Based on Differentiated Polynomial
        Waveforms". IEEE Transactions on Audio, Speech, and Language
        Processing, 18(4), pp. 786-798.

    See also / Vease tambien:
        OscillatorDPW.h        - interface declarations / declaraciones
        OscillatorPolyBLEP.cpp - swappable comparative engine
        Oscillator.h           - facade class and engine selector (OscEngine)
*/

#include "OscillatorDPW.h"


// ============================================================================
//  Public API
// ============================================================================

// EN: Host notifies the sample rate. Increment is recomputed and all state
//     is cleared so each session starts from a known state.
// ES: El host notifica la sample rate. Se recalcula el incremento y se
//     limpia todo el estado para arrancar desde un estado conocido.
void OscillatorDPW::prepare(double newSampleRate)
{
    sampleRate = static_cast<float>(newSampleRate);
    phaseInc = frequency / sampleRate;

    reset();
}


// EN: Updates frequency (clamped to >= 0 Hz) and refreshes phaseInc.
// ES: Actualiza la frecuencia (limitada a >= 0 Hz) y refresca phaseInc.
void OscillatorDPW::setFrequency(float freq)
{
    frequency = std::max(0.0f, freq);
    phaseInc = frequency / sampleRate;
}


// EN: Clears phase and every finite-difference delay tap.
// ES: Limpia la fase y todos los taps de diferencias finitas.
void OscillatorDPW::reset()
{
    phase = 0.0f;

    // Prime DPW histories with phase-consistent previous samples.
    // This avoids a large finite-difference impulse on note start.

    const float p0 = phase;

    auto sawPolyAt = [this](float p)
        {
            const float x = bipolarAt(wrap01(p));
            return sawPolynomial4(x);
        };

    auto squarePolyAt = [this](float p)
        {
            const float xa = bipolarAt(wrap01(p));
            const float xb = bipolarAt(wrap01(p + 0.5f));
            return sawPolynomial4(xb) - sawPolynomial4(xa);
        };

    auto pwmPolyAt = [this](float p)
        {
            const float xa = bipolarAt(wrap01(p));
            const float xb = bipolarAt(wrap01(p + (1.0f - pulseWidth)));
            return sawPolynomial4(xb) - sawPolynomial4(xa);
        };

    auto triPolyAt = [this](float p)
        {
            const float x = bipolarAt(wrap01(p));
            return x * (std::abs(x) - 1.0f);
        };

    z1 = sawPolyAt(p0 - phaseInc);
    z2 = sawPolyAt(p0 - 2.0f * phaseInc);
    z3 = sawPolyAt(p0 - 3.0f * phaseInc);

    sqZ1 = squarePolyAt(p0 - phaseInc);
    sqZ2 = squarePolyAt(p0 - 2.0f * phaseInc);
    sqZ3 = squarePolyAt(p0 - 3.0f * phaseInc);

    pwZ1 = pwmPolyAt(p0 - phaseInc);
    pwZ2 = pwmPolyAt(p0 - 2.0f * phaseInc);
    pwZ3 = pwmPolyAt(p0 - 3.0f * phaseInc);

    triZ1 = triPolyAt(p0 - phaseInc);
}

// EN: Hard phase sync. Copies the other oscillator's phase AND all of its
//     difference histories so both restart together and no click appears
//     in the recovered waveform right after the sync.
// ES: Sync duro de fase. Copia la fase del otro oscilador Y todos sus
//     historiales de diferencias para que ambos reinicien juntos y no
//     aparezca un click en la onda recuperada justo tras el sync.
void OscillatorDPW::syncPhase(const OscillatorDPW& other)
{
    phase = other.phase;

    z1 = other.z1;   z2 = other.z2;   z3 = other.z3;
    sqZ1 = other.sqZ1; sqZ2 = other.sqZ2; sqZ3 = other.sqZ3;
    pwZ1 = other.pwZ1; pwZ2 = other.pwZ2; pwZ3 = other.pwZ3;
    triZ1 = other.triZ1;
}


// EN: Default sample output. Mirrors OscillatorPolyBLEP::nextSample().
// ES: Salida por defecto. Replica OscillatorPolyBLEP::nextSample().
float OscillatorDPW::nextSample()
{
    return saw();
}


// ============================================================================
//  Waveform generators
// ============================================================================

// EN: Pure sine. Identical to the PolyBLEP engine's sine so switching
//     engines on a sine note is bit-for-bit transparent.
// ES: Sinusoide pura. Identica a la del motor PolyBLEP para que cambiar de
//     motor en una nota seno sea transparente muestra a muestra.
float OscillatorDPW::sine()
{
    const float out = std::sin(TWO_PI * phase);
    advance();
    return out;
}


// EN: Fourth-order DPW sawtooth. Polynomial of the bipolar phase, third
//     finite difference, advance, analytical scaling -> normalized saw.
// ES: Sawtooth DPW de cuarto orden. Polinomio de la fase bipolar, tercera
//     diferencia finita, avance, escalado analitico -> saw normalizada.
float OscillatorDPW::saw()
{
    const float x = bipolarPhase();

    const float polynomial = sawPolynomial4(x);
    const float diff = difference3(polynomial, z1, z2, z3);

    advance();

    return diff * sawScale4();
}


// EN: Square = saw(phase - 0.5) - saw(phase). Because difference3 is
//     linear, we feed it the difference of the two saw polynomials and run
//     a SINGLE DPW4 pipeline. The polarity (xb - xa with xb half a period
//     ahead) matches OscillatorPolyBLEP::square (+1 in the first half).
//     A 50% square has zero DC, so nothing extra is needed.
// ES: Square = saw(phase - 0.5) - saw(phase). Como difference3 es lineal,
//     le pasamos la resta de los dos polinomios de saw y corremos UNA sola
//     tuberia DPW4. La polaridad (xb - xa, con xb medio periodo adelante)
//     coincide con OscillatorPolyBLEP::square (+1 en la primera mitad).
//     Un square al 50% tiene DC cero, no hace falta nada extra.
float OscillatorDPW::square()
{
    const float xa = bipolarPhase();                       // saw at phase
    const float xb = bipolarAt(wrap01(phase + 0.5f));      // saw half ahead

    const float polynomial = sawPolynomial4(xb) - sawPolynomial4(xa);
    const float diff = difference3(polynomial, sqZ1, sqZ2, sqZ3);

    advance();

    return diff * sawScale4();
}


// EN: Pulse-width square = saw(phase - pulseWidth) - saw(phase). Same DPW4
//     engine; the shift is the pulse width instead of 0.5. Subtracting two
//     saws yields zero DC for ANY duty cycle, so (unlike a naive +/-1
//     pulse) no DC-correction term is needed. Levels and peak-to-peak
//     match OscillatorPolyBLEP::squarePWM.
// ES: Square con ancho de pulso = saw(phase - pulseWidth) - saw(phase).
//     Mismo motor DPW4; el desfase es el ancho de pulso en vez de 0.5.
//     Restar dos saws da DC cero para CUALQUIER ciclo de trabajo, asi que
//     (a diferencia de un pulso ingenuo +/-1) no hace falta termino de
//     correccion de DC. Niveles y pico a pico coinciden con
//     OscillatorPolyBLEP::squarePWM.
float OscillatorDPW::squarePWM()
{
    const float xa = bipolarPhase();                                   // at phase
    const float xb = bipolarAt(wrap01(phase + (1.0f - pulseWidth)));   // at phase - pw

    const float polynomial = sawPolynomial4(xb) - sawPolynomial4(xa);
    const float diff = difference3(polynomial, pwZ1, pwZ2, pwZ3);

    advance();

    return diff * sawScale4();
}


// EN: Second-order DPW triangle.
//     p(x) = x*(|x| - 1)  ->  p'(x) = 2|x| - 1, exactly a +/-1 triangle.
//     One finite difference + 1/dx scaling recovers a normalized triangle.
//     dx = 2*phaseInc is the per-sample spacing of x; |p'|max = 1, so one
//     difference gives |diff|max ~ dx, hence scale = 1/dx.
// ES: Triangle DPW de segundo orden.
//     p(x) = x*(|x| - 1)  ->  p'(x) = 2|x| - 1, exactamente un triangle
//     +/-1. Una diferencia finita + escalado 1/dx recupera un triangle
//     normalizado. dx = 2*phaseInc es la separacion por muestra de x;
//     |p'|max = 1, una diferencia da |diff|max ~ dx, de ahi scale = 1/dx.
float OscillatorDPW::triangle()
{
    const float x = bipolarPhase();

    const float polynomial = x * (std::abs(x) - 1.0f);

    const float diff = polynomial - triZ1;
    triZ1 = polynomial;

    advance();

    // dx = 2 * phaseInc  ->  scale = 1 / dx = 1 / (2 * phaseInc)
    const float scale = (phaseInc > 0.0f)
        ? (1.0f / (2.0f * phaseInc))
        : 0.0f;

    return diff * scale;
}


// ============================================================================
//  Internal DSP
// ============================================================================

// EN: Maps the current normalized phase [0, 1) to bipolar [-1, 1).
// ES: Mapea la fase normalizada actual [0, 1) a bipolar [-1, 1).
float OscillatorDPW::bipolarPhase() const
{
    return (2.0f * phase) - 1.0f;
}


// EN: Bipolar value for an arbitrary normalized phase p in [0, 1).
// ES: Valor bipolar para una fase normalizada arbitraria p en [0, 1).
float OscillatorDPW::bipolarAt(float p) const
{
    return (2.0f * p) - 1.0f;
}


// EN: Wraps an arbitrary phase value into [0, 1). The arguments here are
//     always within one period of the range, so a single fold is enough.
// ES: Envuelve una fase arbitraria a [0, 1). Los argumentos aqui estan
//     siempre dentro de un periodo del rango, basta un solo pliegue.
float OscillatorDPW::wrap01(float p) const
{
    if (p >= 1.0f) p -= 1.0f;
    if (p < 0.0f)  p += 1.0f;
    return p;
}


// EN: Advances the normalized phase by one sample and wraps into [0, 1).
// ES: Avanza la fase normalizada una muestra y la envuelve en [0, 1).
void OscillatorDPW::advance()
{
    phase += phaseInc;

    if (phase >= 1.0f)
        phase -= 1.0f;
}


// EN: Fourth-order DPW saw polynomial: p(x) = x^4 - 2x^2.
//     Continuous up to its 2nd derivative across the wrap
//     (p(+/-1) = -1, p'(+/-1) = 0, p''(+/-1) = 8), so the discontinuity
//     surfaces only in p'''(x) = 24x, proportional to the trivial saw.
// ES: Polinomio DPW de saw de cuarto orden: p(x) = x^4 - 2x^2.
//     Continuo hasta su 2a derivada en el wrap (p(+/-1) = -1,
//     p'(+/-1) = 0, p''(+/-1) = 8), asi la discontinuidad solo aparece en
//     p'''(x) = 24x, proporcional a la saw trivial.
float OscillatorDPW::sawPolynomial4(float x) const
{
    const float x2 = x * x;
    return (x2 * x2) - (2.0f * x2);
}


// EN: Third backward finite difference:
//         y[n] = x[n] - 3*x[n-1] + 3*x[n-2] - x[n-3]
//     Approximates the third derivative needed to recover a saw-like
//     waveform from the fourth-order polynomial.
// ES: Tercera diferencia finita regresiva:
//         y[n] = x[n] - 3*x[n-1] + 3*x[n-2] - x[n-3]
//     Aproxima la tercera derivada necesaria para recuperar una forma tipo
//     saw desde el polinomio de cuarto orden.
float OscillatorDPW::difference3(float x, float& d1, float& d2, float& d3)
{
    const float y = x - (3.0f * d1) + (3.0f * d2) - d3;

    d3 = d2;
    d2 = d1;
    d1 = x;

    return y;
}


// EN: Analytical amplitude scaling for DPW4.
//
//     x = 2*phase - 1  =>  consecutive samples are spaced dx = 2*phaseInc.
//     The third backward difference of the polynomial approximates
//     p'''(x) * dx^3. Since p'''(x) = 24x and the target output is the
//     trivial saw (x), divide by  24 * dx^3  =  N! * dx^(N-1)  (N = 4).
//     Equivalent closed form: 1 / (192 * phaseInc^3).
//     Reused unchanged by square()/squarePWM() because difference3 is
//     linear (same scale applies to the saw-difference signal).
//
// ES: Escalado analitico de amplitud para DPW4.
//
//     x = 2*phase - 1  =>  muestras consecutivas distan dx = 2*phaseInc.
//     La tercera diferencia regresiva del polinomio aproxima
//     p'''(x) * dx^3. Como p'''(x) = 24x y la salida objetivo es la saw
//     trivial (x), dividir entre  24 * dx^3  =  N! * dx^(N-1)  (N = 4).
//     Forma cerrada equivalente: 1 / (192 * phaseInc^3).
//     Lo reutilizan sin cambios square()/squarePWM() porque difference3 es
//     lineal (el mismo escalado vale para la senal saw-diferencia).
float OscillatorDPW::sawScale4() const
{
    if (phaseInc <= 0.0f)
        return 0.0f;

    const float dx = 2.0f * phaseInc;       // spacing of x per sample
    const float dx3 = dx * dx * dx;

    return 1.0f / (24.0f * dx3);            // 1 / (4! * dx^3)
}
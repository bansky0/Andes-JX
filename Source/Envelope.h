/*
  ==============================================================================
    Envelope.h

    ESP:
    Envolvente para controlar la amplitud (u otro parámetro) por muestra.
    Implementa una transición suave exponencial hacia un objetivo (target)
    usando un "multiplier" para definir la rapidez (attack/decay/release).
    Diseñada para ser liviana en CPU y fácil de integrar por voz.

    ENG:
    Per-sample envelope for controlling amplitude (or any parameter).
    Uses an exponential smoothing approach towards a target value, with a
    "multiplier" controlling the rate (attack/decay/release). Lightweight
    and voice-friendly.
  ==============================================================================
*/

#pragma once

//==============================================================================
// ESP: Umbral mínimo para considerar la envolvente "activa". Ayuda a tratar
//      valores muy pequeños como silencio y permite apagar voces.
// ENG: Minimum threshold to consider the envelope "active". Treats very small
//      values as silence and allows voice deactivation.
static constexpr float SILENCE = 0.0001f;

class Envelope
{
private:
    //==========================================================================
    // ESP: Estado interno del generador de envolvente.
    //      target: valor objetivo al que la envolvente converge suavemente.
    //      multiplier: coeficiente del suavizado exponencial (define la rapidez
    //      del movimiento hacia target).
    // ENG: Internal state.
    //      target: destination value the envelope converges to.
    //      multiplier: exponential smoothing coefficient (controls how fast the
    //      envelope moves towards target).
    float target = 0.0f;
    float multiplier = 0.0f;

public:
    //==========================================================================
    // ESP: Parámetros (configurados desde el motor / voz).
    //      Multipliers típicamente se calculan en función del tiempo y sampleRate.
    // ENG: Parameters (set by the synth engine / voice).
    //      Multipliers are typically computed from time constants and sampleRate.
    float attackMultiplier  = 0.0f;
    float decayMultiplier   = 0.0f;
    float sustainLevel      = 0.0f;
    float releaseMultiplier = 0.0f;

    //==========================================================================
    // ESP: Nivel actual de la envolvente (salida).
    // ENG: Current envelope level (output).
    float level = 0.0f;

    //==========================================================================
    // ESP: Avanza la envolvente una muestra y devuelve el siguiente valor.
    //      El cálculo implementa un acercamiento exponencial hacia "target".
    //
    //      Convención de estado:
    //      - Ataque: target = 2.0f (se usa un valor > 1.0 para detectar "attack")
    //      - Luego cambia automáticamente a decay/sustain cuando se acerca al target.
    //
    // ENG: Advances the envelope by one sample and returns the next value.
    //      This performs an exponential approach towards "target".
    //
    //      State convention:
    //      - Attack: target = 2.0f (a value > 1.0 is used to mark "attack")
    //      - Automatically switches to decay/sustain when close to the target.
    float nextValue()
    {
        level = multiplier * (level - target) + target;

        // ESP: Al final del ataque (casi llegando al target=2.0), cambia a decay
        //      y ajusta el nuevo objetivo al sustain.
        // ENG: Near the end of attack (close to target=2.0), switch to decay
        //      and set the new target to the sustain level.
        if (target >= 2.0f && level >= target * 0.99f)
        {
            multiplier = decayMultiplier;
            target     = sustainLevel;
        }

        return level;
    }

    //==========================================================================
    // ESP: Indica si la envolvente aún tiene nivel suficiente para considerarse audible.
    // ENG: Returns true if the envelope level is still above the silence threshold.
    inline bool isActive() const { return level > SILENCE; }

    //==========================================================================
    // ESP: Indica si la envolvente está en fase de ataque (según la convención target>=2.0).
    // ENG: Returns true if the envelope is currently in attack phase (target>=2.0 by convention).
    inline bool isInAttack() const { return target >= 2.0f; }

    //==========================================================================
    // ESP: Dispara el ataque. Se fuerza un "arranque" mínimo para evitar quedarse
    //      pegado en cero por precisión numérica.
    // ENG: Triggers attack. Adds a tiny offset to avoid staying stuck at zero
    //      due to numerical precision.
    void attack()
    {
        level += SILENCE + SILENCE;
        target = 2.0f;
        multiplier = attackMultiplier;
    }

    //==========================================================================
    // ESP: Dispara el release hacia cero.
    // ENG: Triggers release towards zero.
    void release()
    {
        target = 0.0f;
        multiplier = releaseMultiplier;
    }

    //==========================================================================
    // ESP: Reinicia el estado interno de la envolvente.
    // ENG: Resets the internal envelope state.
    void reset()
    {
        level = 0.0f;
        target = 0.0f;
        multiplier = 0.0f;
    }
};
/*
  ==============================================================================

    Envelope.h
    Created: 7 Dec 2025 12:36:44pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: Envelope
    Purpose:
        EN: Compact ADSR envelope generator based on a single-pole
            exponential model. Drives amplitude and filter modulation
            inside each Voice.
        ES: Generador de envolvente ADSR compacto basado en un modelo
            exponencial de un solo polo. Modula amplitud y filtro
            dentro de cada Voice.

    Main responsibilities:
        EN:
          - Produce one envelope value per audio sample
          - Trigger Attack on note-on and Release on note-off
          - Auto-transition from Attack to Decay when the level is reached
          - Report whether the envelope is still active (above silence)
        ES:
          - Producir un valor de envolvente por muestra de audio
          - Disparar Attack en note-on y Release en note-off
          - Transicionar automcamente de Attack a Decay al llegar al nivel
          - Reportar si la envolvente sigue activa (por encima del silencio)

    Architectural role:
        EN: Each Voice owns two envelopes: one for amplitude (envAttack..
            envRelease parameters) and one for the filter (filterAttack..
            filterRelease). The per-stage multipliers are computed by the
            Voice/Synth from the user-facing time parameters.
        ES: Cada Voice posee dos envolventes: una para amplitud (parmetros
            envAttack..envRelease) y otra para el filtro (filterAttack..
            filterRelease). Los multipliers por fase los calculan Voice/Synth
            a partir de los parmetros temporales que ve el usuario.

    Notes:
        EN:
          - Unified model: instead of four discrete states, the envelope
            tracks a moving `target` with a per-sample `multiplier`. The
            update rule
                level = multiplier * (level - target) + target
            is a one-pole IIR that approaches `target` exponentially.
          - Attack overshoots on purpose: target is set to 2.0 and the
            transition to Decay fires at 99% of target (~1.98). This makes
            Attack reach 1.0 cleanly instead of asymptotically.
          - The convention `target >= 2.0f` doubles as a phase flag
            (used by isInAttack()). No enum or state machine needed.
        ES:
          - Modelo unificado: en lugar de cuatro estados discretos, la
            envolvente sigue un `target` mvil con un `multiplier` por
            muestra. La regla de actualizacin
                level = multiplier * (level - target) + target
            es un IIR de un polo que se acerca a `target` exponencialmente.
          - Attack hace overshoot a propsito: target se fija en 2.0 y la
            transicin a Decay se dispara al 99% del target (~1.98). As

            el ataque llega a 1.0 limpio en vez de asintticamente.
          - La convencin `target >= 2.0f` funciona tambi	n como bandera
            de fase (la usa isInAttack()). No hace falta enum ni mquina
            de estados.
*/

#pragma once


// EN: Threshold below which the envelope is considered silent and the
//     voice can be deactivated. Avoids leaving inaudible tails alive.
// ES: Umbral por debajo del cual se considera silencio y la voz puede
//     desactivarse. Evita dejar colas inaudibles vivas.
const float SILENCE = 0.0001f;


class Envelope
{
private:
    // EN: Current target the level is moving toward (1.0 amp / 2.0 attack
    //     overshoot / sustainLevel during decay-sustain / 0.0 during release).
    // ES: Objetivo actual al que tiende el nivel (1.0 amp / 2.0 overshoot
    //     en attack / sustainLevel en decay-sustain / 0.0 en release).
    float target;

    // EN: Per-sample exponential coefficient. Values closer to 1.0 produce
    //     slower (longer) transitions; values farther from 1.0 are faster.
    // ES: Coeficiente exponencial por muestra. Valores cercanos a 1.0
    //     producen transiciones ms lentas; valores lejanos, ms rpidas.
    float multiplier;

public:
    // EN: Pre-computed coefficients per stage. The Voice/Synth layer
    //     calculates these from the user-facing A/D/R time values.
    // ES: Coeficientes precalculados por fase. La capa Voice/Synth los
    //     calcula a partir de los tiempos A/D/R que ve el usuario.
    float attackMultiplier;
    float decayMultiplier;
    float sustainLevel;
    float releaseMultiplier;

    // EN: Current envelope value, exposed so consumers (amp, filter) can
    //     read it directly without an extra getter call.
    // ES: Valor actual de la envolvente, expuesto para que los consumidores
    //     (amp, filtro) lo lean directamente sin un getter extra.
    float level;


    // EN: Advances the envelope by one sample and returns the new level.
    //     Handles the Attack -> Decay transition automatically.
    // ES: Avanza la envolvente una muestra y devuelve el nuevo nivel.
    //     Maneja automticamente la transicin Attack -> Decay.
    float nextValue()
    {
        // EN: One-pole exponential update toward `target`.
        // ES: Actualizacin exponencial de un polo hacia `target`.
        level = multiplier * (level - target) + target;

        // EN: If we are in Attack (target == 2.0) and have reached 99% of it,
        //     switch to Decay phase: aim at sustainLevel using decayMultiplier.
        // ES: Si estamos en Attack (target == 2.0) y llegamos al 99% del
        //     objetivo, pasamos a Decay: apuntamos a sustainLevel con
        //     decayMultiplier.
        if (target >= 2.0f && level >= target * 0.99f) {
            multiplier = decayMultiplier;
            target = sustainLevel;
        }

        return level;
    }

    // EN: True while the envelope produces audible output.
    // ES: Verdadero mientras la envolvente produzca salida audible.
    inline bool isActive() const
    {
        return level > SILENCE;
    }

    // EN: True while in the Attack phase. The check `target >= 2.0f` works
    //     because no other phase ever sets target above 1.0.
    // ES: Verdadero mientras estemos en Attack. El chequeo `target >= 2.0f`
    //     funciona porque ninguna otra fase fija target por encima de 1.0.
    inline bool isInAttack() const
    {
        return target >= 2.0f;
    }

    // EN: Triggers the Attack phase (note-on). The tiny bump on `level`
    //     guarantees the voice is treated as active even from full silence.
    // ES: Dispara la fase de Attack (note-on). El pequeo impulso en `level`
    //     garantiza que la voz se considere activa incluso desde silencio.
    void attack()
    {
        level += SILENCE + SILENCE;
        target = 2.0f;
        multiplier = attackMultiplier;
    }

    // EN: Triggers the Release phase (note-off). The level decays toward 0.
    // ES: Dispara la fase de Release (note-off). El nivel decae hacia 0.
    void release()
    {
        target = 0.0f;
        multiplier = releaseMultiplier;
    }

    // EN: Hard reset to a neutral inactive state. Used on voice allocation.
    // ES: Reset duro a estado neutro inactivo. Se usa al asignar la voz.
    void reset()
    {
        level = 0.0f;
        target = 0.0f;
        multiplier = 0.0f;
    }
};
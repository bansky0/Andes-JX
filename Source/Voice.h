/*
  ==============================================================================

    Voice.h
    Created: 10 Nov 2025 6:45:29pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: Voice
    Purpose:
        EN: Single polyphonic voice of AndesJX. Combines two oscillators,
            a noise source, two envelopes (amplitude and filter) and one
            interchangeable filter into a complete monophonic synth slice.
        ES: Voz polifónica individual de AndesJX. Combina dos osciladores,
            una fuente de ruido, dos envolventes (amplitud y filtro) y un
            filtro intercambiable en una rebanada monofónica completa de
            sintetizador.

    Main responsibilities:
        EN:
          - Hold the per-voice state: active MIDI note, frequency, glide,
            velocity, pan, and the pointer to the active filter
          - Mix the two oscillators with constant-power crossfade
          - Apply the amplitude envelope to the mixed output
          - Compute the filter envelope value for Synth to consume
          - Compute per-note stereo panning based on MIDI pitch
        ES:
          - Mantener el estado por voz: nota MIDI activa, frecuencia, glide,
            velocidad, paneo y el puntero al filtro activo
          - Mezclar ambos osciladores con crossfade de potencia constante
          - Aplicar la envolvente de amplitud a la salida mezclada
          - Calcular el valor de la envolvente de filtro para que lo use Synth
          - Calcular el paneo estéreo por nota según la altura MIDI

    Architectural role:
        EN: Owned by Synth in a fixed-size pool (MAX_VOICES). Synth assigns
            MIDI notes to voices, routes parameters to them each block, and
            sums their outputs. Voice itself does not know about polyphony
            or MIDI timing; it only processes audio samples on demand.
        ES: Propiedad de Synth en un pool de tamaño fijo (MAX_VOICES).
            Synth asigna notas MIDI a las voces, les enruta parámetros por
            bloque y suma sus salidas. Voice en sí no sabe de polifonía ni
            de timing MIDI; solo procesa muestras de audio cuando se le pide.

    Notes:
        EN:
          - Both filter adapters (SVFFilter and MoogFilter) are stored
            inline so that switching filter algorithms is a matter of
            reassigning the `filter` pointer, with no allocations.
          - The oscillator mix uses a square-root crossfade (equal-power
            law) instead of a linear one. See the comment in render() for
            the reasoning and alternatives that were explored.
          - Panning behavior is piano-like: low notes stay centered and
            high notes spread outward. This is uncommon in synths and is
            part of AndesJX's sonic identity.
        ES:
          - Ambos adaptadores de filtro (SVFFilter y MoogFilter) se
            almacenan en línea, de modo que intercambiar algoritmos de
            filtro es solo reasignar el puntero `filter`, sin reservas.
          - La mezcla de osciladores usa crossfade por raíz cuadrada (ley
            de potencia constante) en lugar de uno lineal. Ver el comentario
            en render() con la razón y las alternativas exploradas.
          - El paneo imita la disposición de un piano: las notas graves
            permanecen al centro y las agudas se abren al estéreo. Esto es
            poco común en sintes y es parte de la identidad sonora de
            AndesJX.
*/

#pragma once

#include "Oscillator.h"
#include "Envelope.h"
#include "IFilter.h"
#include "SVFFilter.h"
#include "MoogFilter.h"


struct Voice
{
    // --- Note state / Estado de la nota ------------------------------------

    // EN: MIDI note currently active, or -1 if the voice is idle.
    // ES: Nota MIDI actualmente activa, o -1 si la voz está libre.
    int note = -1;

    // EN: Base frequency in Hz derived from the MIDI note by Synth.
    // ES: Frecuencia base en Hz que Synth deriva de la nota MIDI.
    float freq = 0.0f;

    // EN: Gain scaling from MIDI velocity. 0.997 is a "near full" default
    //     applied before the first note-on updates it.
    // ES: Ganancia derivada de la velocidad MIDI. 0.997 es el valor "casi
    //     completo" por defecto, antes de que el primer note-on lo ajuste.
    float velocityGain = 0.997f;

    // EN: True once release() has been called; amplitude env is fading out.
    // ES: Verdadero cuando se ha llamado release(); la envolvente de
    //     amplitud está desvaneciendo.
    bool released = false;


    // --- Glide / Glide -----------------------------------------------------

    // EN: Frequency actually heard, smoothed toward freqTarget over time.
    // ES: Frecuencia que realmente se oye, suavizada hacia freqTarget.
    float freqCurrent = 0.0f;

    // EN: Target frequency of the newly played note.
    // ES: Frecuencia objetivo de la nueva nota tocada.
    float freqTarget = 0.0f;

    // EN: Smoothing coefficient per sample. 1.0 means no glide (instant).
    // ES: Coeficiente de suavizado por muestra. 1.0 significa sin glide.
    float glideRateThisNote = 1.0f;


    // --- Oscillator mix / Mezcla de osciladores ----------------------------

    // EN: Relative amount of osc2 in the output mix, in [0, 1].
    //     Combined with osc1 using an equal-power crossfade (see render()).
    // ES: Cantidad relativa de osc2 en la mezcla, en [0, 1]. Se combina
    //     con osc1 por un crossfade de potencia constante (ver render()).
    float osc2Gain = 1.0f;


    // --- Stereo placement / Ubicación estéreo ------------------------------

    // EN: Left/right gains, filled by updatePanning() before output.
    // ES: Ganancias izquierda/derecha, asignadas por updatePanning() antes
    //     de la salida.
    float panLeft = 0.707f;
    float panRight = 0.707f;

    // EN: Per-voice random offset in [-1, 1] to add subtle pan variation.
    // ES: Desplazamiento aleatorio por voz en [-1, 1] para variar sutilmente
    //     el paneo.
    float randomPan = 0.0f;

    // EN: Global stereo width multiplier applied on top of randomPan.
    // ES: Multiplicador global del ancho estéreo aplicado sobre randomPan.
    float stereoWidth = 1.0f;


    // --- Filter state / Estado del filtro ----------------------------------

    // EN: Smoothed cutoff in Hz, updated by Synth to avoid zipper noise
    //     when the user sweeps the control.
    // ES: Cutoff suavizado en Hz, actualizado por Synth para evitar el
    //     ruido de "zipper" cuando el usuario mueve el control.
    float cutoffZipHz = 0.0f;

    // EN: Scale factor applied to the filter envelope depth before the
    //     cutoff is modulated. Set by Synth from the preset.
    // ES: Factor que se aplica a la profundidad de la envolvente de filtro
    //     antes de modular el cutoff. Lo fija Synth desde el preset.
    float filterEnvDepthMultiplier = 1.0f;

    // EN: Latest filter envelope value, computed in render() and read by
    //     Synth for filter cutoff modulation.
    // ES: Último valor de la envolvente de filtro, calculado en render() y
    //     leído por Synth para modular el cutoff.
    float filterEnvValue = 0.0f;

    // EN: Normalized filter resonance in [0, 1], forwarded to the active
    //     filter via setFilter().
    // ES: Resonancia normalizada del filtro en [0, 1], reenviada al filtro
    //     activo mediante setFilter().
    float filterResonance = 0.0f;

    // EN: Last cutoff value written through setFilter(). Cached for lookup.
    // ES: Último valor de cutoff escrito por setFilter(). Se cachea para
    //     consulta.
    float filterCutoffHz = 1000.0f;


    // --- DSP modules / Módulos DSP -----------------------------------------

    Oscillator osc1;
    Oscillator osc2;
    Envelope   env;        // EN: amplitude envelope   |  ES: envolvente de amplitud
    Envelope   filterEnv;  // EN: filter envelope      |  ES: envolvente de filtro

    // EN: Both filter adapters are kept as members so the voice can switch
    //     between them by simply reassigning `filter`. This avoids any
    //     allocation during playback.
    // ES: Ambos adaptadores de filtro se mantienen como miembros para que
    //     la voz cambie entre ellos con solo reasignar `filter`. Así se
    //     evita cualquier reserva durante la reproducción.
    SVFFilter  svfFilter;
    MoogFilter moogFilter;

    // EN: Points to the currently active filter (either svfFilter or
    //     moogFilter). Set by Synth from the filterType preset parameter.
    // ES: Apunta al filtro activo (svfFilter o moogFilter). Lo fija Synth
    //     a partir del parámetro filterType del preset.
    IFilter* filter = nullptr;


    // --- Lifecycle / Ciclo de vida -----------------------------------------

    // EN: Marks the voice as playing a given MIDI note. Frequency
    //     conversion from note number to Hz is done by Synth, not here.
    // ES: Marca la voz como tocando una nota MIDI dada. La conversión de
    //     número de nota a Hz la hace Synth, no esta clase.
    void startNote(int midiNote)
    {
        note = midiNote;
        released = false;
    }

    // EN: Marks the voice as idle. Does not touch DSP state; use reset()
    //     for a full clean-up.
    // ES: Marca la voz como inactiva. No toca el estado DSP; usar reset()
    //     para una limpieza completa.
    void stopNote()
    {
        note = -1;
        freq = 0.0f;
        freqCurrent = 0.0f;
        freqTarget = 0.0f;
        released = false;
    }

    // EN: Full reset to a clean neutral state. Called when a voice is
    //     allocated from the pool or when presets change.
    // ES: Reset completo a un estado neutro limpio. Se llama al asignar
    //     una voz del pool o al cambiar de preset.
    void reset()
    {
        stopNote();
        released = false;
        cutoffZipHz = 0.0f;

        env.reset();
        osc1.reset();
        osc2.reset();

        if (filter != nullptr)
            filter->reset();

        panLeft = 0.707f;
        panRight = 0.707f;

        filterEnvValue = 0.0f;
        filterEnv.reset();
    }

    // EN: Marks the note as released. The amplitude and filter envelopes
    //     begin their release phase; the voice keeps producing sound
    //     until env.isActive() returns false.
    // ES: Marca la nota como liberada. Las envolventes de amplitud y de
    //     filtro inician su fase de release; la voz sigue sonando hasta
    //     que env.isActive() devuelva false.
    void release()
    {
        released = true;
        env.release();
        filterEnv.release();
    }


    // --- Audio processing / Procesamiento de audio -------------------------

    // EN: Produces one audio sample. The `noise` argument is supplied by
    //     Synth from the shared NoiseGenerator so that all voices share a
    //     single noise stream (cheaper and sonically richer than one per
    //     voice). The `usePwm` flag is accepted for historical compatibility
    //     and is currently unused.
    // ES: Produce una muestra de audio. El argumento `noise` lo suministra
    //     Synth desde el NoiseGenerator compartido para que todas las voces
    //     reciban el mismo flujo de ruido (más barato y sonoramente más
    //     rico que uno por voz). El flag `usePwm` se acepta por
    //     compatibilidad histórica y actualmente no se usa.
    float render(float noise, bool /*usePwm*/)
    {
        float sample1 = osc1.nextSample();
        float sample2 = osc2.nextSample();

        // EN: Equal-power crossfade between osc1 and osc2. Using sqrt on
        //     both gains keeps the perceived loudness constant as osc2Gain
        //     sweeps from 0 to 1, the same principle behind the stereo
        //     pan law. See alternative mixing strategies documented at
        //     the end of this file.
        // ES: Crossfade de potencia constante entre osc1 y osc2. Usar sqrt
        //     en ambas ganancias mantiene la sonoridad percibida constante
        //     al mover osc2Gain de 0 a 1, el mismo principio que rige la
        //     ley de paneo estéreo. Ver estrategias alternativas
        //     documentadas al final de este archivo.
        const float osc1Gain = std::sqrt(1.0f - osc2Gain);
        const float osc2LinGain = std::sqrt(osc2Gain);
        float output = sample1 * osc1Gain + sample2 * osc2LinGain + noise;

        // EN: Filter stage (skipped only if no filter has been assigned).
        // ES: Etapa de filtro (se omite solo si no hay filtro asignado).
        if (filter != nullptr)
            output = filter->render(output);

        // EN: Both envelopes advance one sample. The amplitude envelope
        //     shapes the output; the filter envelope is stored for Synth
        //     to read and use when modulating the cutoff of the next block.
        // ES: Ambas envolventes avanzan una muestra. La envolvente de
        //     amplitud da forma a la salida; la de filtro se guarda para
        //     que Synth la lea y module el cutoff del siguiente bloque.
        float envelope = env.nextValue();
        filterEnvValue = filterEnv.nextValue();

        return output * envelope * velocityGain;
    }


    // --- Panning / Paneo ---------------------------------------------------

    // EN: Computes the per-voice pan gains based on the MIDI note. Low
    //     notes stay near the center, high notes spread outward; the
    //     result is modulated by randomPan and the global stereoWidth.
    // ES: Calcula las ganancias de paneo por voz según la nota MIDI. Las
    //     notas graves permanecen cerca del centro, las agudas se abren
    //     al estéreo; el resultado se modula con randomPan y el
    //     stereoWidth global.
    void updatePanning(int /*voiceIndex*/)
    {
        // EN: t controls how much stereo effect to apply: 0 for notes
        //     below MIDI 40 (mono-centered), 1 by MIDI 88 (fully spread).
        // ES: t controla cuánto efecto estéreo aplicar: 0 para notas por
        //     debajo de MIDI 40 (mono al centro), 1 en MIDI 88 (paneo total).
        float t = std::clamp((note - 40.0f) / 48.0f, 0.0f, 1.0f);

        // EN: Baseline direction. A pianist-like layout would also set
        //     this from the voice index (alternating left/right); keeping
        //     it fixed produces a subtler, less "ping-pong" stereo image.
        // ES: Dirección de base. Una disposición tipo pianista también
        //     podría fijarla desde el índice de voz (alternando
        //     izquierda/derecha); mantenerla fija produce una imagen
        //     estéreo más sutil, menos "ping-pong".
        float pianoDirection = 1.0f;

        // EN: Blend fixed direction (80%) with a pinch of randomness (20%).
        // ES: Mezcla dirección fija (80%) con una pizca de aleatoriedad (20%).
        float panningBase = (pianoDirection * 0.8f) + (randomPan * 0.2f);

        // EN: Apply stereo width and the note-intensity factor.
        // ES: Aplicar ancho estéreo y el factor de intensidad por nota.
        float panning = panningBase * stereoWidth * t;

        // EN: Constant-power pan law: sine/cosine pair keeps the total
        //     energy level invariant across the stereo field.
        // ES: Ley de paneo de potencia constante: el par seno/coseno
        //     mantiene la energía total invariante en el campo estéreo.
        panLeft = std::sin(PI_OVER_4 * (1.0f - panning));
        panRight = std::sin(PI_OVER_4 * (1.0f + panning));
    }


    // --- Filter control / Control del filtro -------------------------------

    // EN: Updates the cached cutoff/resonance and pushes them to the
    //     active filter. Called by Synth each block as modulation sources
    //     (LFO, filter envelope, keytrack) recompute the final cutoff.
    // ES: Actualiza el cutoff/resonance cacheados y los envía al filtro
    //     activo. Synth la llama cada bloque conforme las fuentes de
    //     modulación (LFO, envolvente de filtro, keytrack) recalculan el
    //     cutoff final.
    void setFilter(float cutoffHz, float resonance)
    {
        filterCutoffHz = cutoffHz;
        filterResonance = resonance;

        if (filter != nullptr)
            filter->updateCoefficients(filterCutoffHz, filterResonance);
    }
};


/*
    ============================================================================
    ALTERNATIVE MIXING STRATEGIES / ESTRATEGIAS ALTERNATIVAS DE MEZCLA
    ============================================================================

    EN: Three oscillator-mixing approaches were evaluated during AndesJX's
        development. The active code in render() uses the third one
        (equal-power crossfade). The other two are preserved here as a
        record of the design trade-offs considered.

        (1) Subtractive mix (classic Roland JX style):
              float sample1 = osc1.nextSample();
              float sample2 = osc2.nextSample() * osc2Gain;
              float output  = sample1 - sample2 + noise;

            Subtracting osc2 from osc1 emphasizes their phase differences
            and gives the "scooped", detuned character associated with the
            JX-3P / JX-8P family. It was discarded because it makes the
            mix depend strongly on the relative phase of both oscillators
            and loses energy quickly when they are close in pitch.

        (2) Linear crossfade:
              float osc1Gain = 1.0f - osc2Gain;
              float output   = sample1 * osc1Gain + sample2 * osc2Gain + noise;

            Easy to reason about, but perceived loudness dips in the
            middle of the crossfade (osc2Gain ~ 0.5), because two
            uncorrelated signals at half amplitude sum to less than one
            at full amplitude. Workable for static mixes, audible when
            osc2Gain is modulated.

        (3) Equal-power crossfade (currently active):
              float osc1Gain    = std::sqrt(1.0f - osc2Gain);
              float osc2LinGain = std::sqrt(osc2Gain);
              float output      = sample1 * osc1Gain + sample2 * osc2LinGain + noise;

            The sqrt pair preserves total energy across the sweep. It
            produces a thicker, more consistent blend and is the same
            law used in the stereo panner below.

    ES: Se evaluaron tres enfoques de mezcla de osciladores durante el
        desarrollo de AndesJX. El código activo en render() usa el tercero
        (crossfade de potencia constante). Los otros dos se conservan aquí
        como registro de los trade-offs considerados.

        (1) Mezcla por sustracción (estilo Roland JX clásico):
              float sample1 = osc1.nextSample();
              float sample2 = osc2.nextSample() * osc2Gain;
              float output  = sample1 - sample2 + noise;

            Sustraer osc2 de osc1 enfatiza las diferencias de fase entre
            ambos y da el carácter "vaciado" y detuneado asociado a la
            familia JX-3P / JX-8P. Se descartó porque la mezcla depende
            mucho de la fase relativa de ambos osciladores y pierde
            energía rápido cuando están cercanos en altura.

        (2) Crossfade lineal:
              float osc1Gain = 1.0f - osc2Gain;
              float output   = sample1 * osc1Gain + sample2 * osc2Gain + noise;

            Fácil de razonar, pero la sonoridad percibida baja en el medio
            del crossfade (osc2Gain ~ 0.5), porque dos señales no
            correlacionadas a media amplitud suman menos que una a
            amplitud completa. Usable para mezclas estáticas, audible al
            modular osc2Gain.

        (3) Crossfade de potencia constante (activo):
              float osc1Gain    = std::sqrt(1.0f - osc2Gain);
              float osc2LinGain = std::sqrt(osc2Gain);
              float output      = sample1 * osc1Gain + sample2 * osc2LinGain + noise;

            El par sqrt preserva la energía total en todo el recorrido.
            Produce una mezcla más gruesa y consistente, y es la misma
            ley que usa el panner estéreo de más arriba.
*/
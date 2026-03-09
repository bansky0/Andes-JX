/*
  ==============================================================================
    Voice.h

    ESP:
    Define la estructura "Voice" (una voz de síntesis). Una Voice encapsula el
    estado de reproducción de UNA nota: osciladores, envolventes, filtro,
    paneo y lógica mínima de ciclo de vida (start/release/stop/reset).
    Se usa típicamente dentro de un motor polifónico que mezcla varias voces.

    ENG:
    Defines the "Voice" structure (one synth voice). A Voice encapsulates the
    playback state for ONE note: oscillators, envelopes, filter, panning, and
    minimal lifecycle logic (start/release/stop/reset). Typically used inside
    a polyphonic engine that mixes multiple voices.
  ==============================================================================
*/

#pragma once

#include "Oscillator.h"
#include "Envelope.h"
#include "IFilter.h"
#include "SVFFilter.h"
#include "MoogFilter.h"

// NOTE (build/deps):
// - Uses std::clamp (requires <algorithm>) and std::sin (requires <cmath>).
// - Uses PI_OVER_4 (make sure Constants.h (or equivalent) is included in the TU).

struct Voice
{
    //==========================================================================
    // ESP: Estado básico de nota/voz.
    //      note = nota MIDI activa (-1 significa "sin nota").
    //      released = indica que se llamó a release() (etapa de apagado).
    // ENG: Basic note/voice state.
    //      note = active MIDI note (-1 means "idle").
    //      released = set when release() is triggered (release stage).
    int note = -1;
    bool released = false;

    //==========================================================================
    // ESP: Ganancias/mezcla por voz.
    //      velocityGain: factor por velocidad (o curva ya mapeada).
    //      osc2Gain: mezcla relativa del oscilador 2.
    // ENG: Per-voice gains/mix.
    //      velocityGain: factor from MIDI velocity (or mapped curve).
    //      osc2Gain: relative mix for oscillator 2.
    float velocityGain = 0.997f;
    float osc2Gain = 1.0f;

    //==========================================================================
    // ESP: Paneo y anchura estéreo.
    //      panLeft/panRight: ganancias L/R (ley de potencia constante).
    //      randomPan: variación aleatoria [-1..1] (se mezcla con paneo base).
    //      stereoWidth: escala global del paneo (0=mono, 1=normal).
    // ENG: Panning and stereo width.
    //      panLeft/panRight: L/R gains (constant-power law).
    //      randomPan: random variation [-1..1] (blended with base panning).
    //      stereoWidth: global panning scale (0=mono, 1=normal).
    float panLeft, panRight;
    float randomPan = 0.0f; // [-1..1]
    float stereoWidth = 1.0f;

    //==========================================================================
    // ESP: Frecuencias para glide (portamento).
    //      freqCurrent: frecuencia efectiva que suena en este instante.
    //      freqTarget: frecuencia objetivo (nota nueva).
    //      glideRateThisNote: factor por nota (1.0 = sin glide).
    // ENG: Glide (portamento) frequency state.
    //      freqCurrent: current audible frequency.
    //      freqTarget: target frequency (new note).
    //      glideRateThisNote: per-note factor (1.0 = no glide).
    float freqCurrent = 0.0f;
    float freqTarget  = 0.0f;
    float glideRateThisNote = 1.0f;

    //==========================================================================
    // ESP: Modulación de filtro por envolvente.
    //      filterEnvDepthMultiplier: escala externa de profundidad.
    //      filterEnvValue: valor actual de la envolvente del filtro.
    //      filterResonance: resonancia actual aplicada.
    // ENG: Filter envelope modulation.
    //      filterEnvDepthMultiplier: external depth scaling.
    //      filterEnvValue: current filter envelope value.
    //      filterResonance: current resonance value.
    float filterEnvDepthMultiplier = 1.0f;
    float filterEnvValue = 0.0f;
    float filterResonance = 0.0f;

    //==========================================================================
    // ESP: Generadores principales de la voz.
    // ENG: Main generators for the voice.
    Oscillator osc1;
    Oscillator osc2;

    //==========================================================================
    // ESP: Envolvente principal (amplitud) y envolvente para filtro.
    // ENG: Main envelope (amplitude) and filter envelope.
    Envelope env;
    Envelope filterEnv;

    //==========================================================================
    // ESP: Filtros disponibles y puntero al filtro activo.
    //      filter apunta a uno de los filtros (SVF/Moog) o nullptr si no se usa.
    // ENG: Available filters and pointer to the active filter.
    //      filter points to one filter (SVF/Moog) or nullptr if disabled.
    SVFFilter svfFilter;
    MoogFilter moogFilter;
    IFilter* filter = nullptr;

    //==========================================================================
    // ESP: Ciclo de vida de nota.
    // ENG: Note lifecycle.
    void startNote(int midiNote)
    {
        note = midiNote;
        released = false;
    }

    // ESP: Detiene y limpia el estado de nota (voz inactiva).
    // ENG: Stops and clears note state (voice becomes idle).
    void stopNote()
    {
        note = -1;
        freqCurrent = 0.0f;
        freqTarget  = 0.0f;
        released = false;
    }

    //==========================================================================
    // ESP: Reinicia la voz a un estado conocido (silenciosa y lista).
    //      Importante para evitar clicks/estado sucio entre notas.
    // ENG: Resets the voice to a known state (silent and ready).
    //      Important to avoid clicks/stale state between notes.
    void reset()
    {
        stopNote();
        released = false;

        env.reset();
        osc1.reset();
        osc2.reset();

        if (filter != nullptr)
            filter->reset();

        // ESP/ENG: Centro (ley potencia constante aproximada)
        panLeft  = 0.707f;
        panRight = 0.707f;

        filterEnvValue = 0.0f;
        filterEnv.reset();
    }

    //==========================================================================
    // ESP: Renderiza una muestra mono de esta voz.
    //      - Mezcla osc1, osc2 y ruido.
    //      - Aplica filtro (si está activo).
    //      - Aplica envolventes (amplitud y filtro).
    //      Nota: El paneo se calcula aparte (updatePanning) y típicamente se
    //      aplica en la etapa de mezcla estéreo del motor.
    //
    // ENG: Renders one mono sample for this voice.
    //      - Mixes osc1, osc2 and noise.
    //      - Applies filter (if enabled).
    //      - Applies envelopes (amplitude and filter).
    //      Note: Panning is computed separately (updatePanning) and typically
    //      applied during the engine's stereo mix stage.
    float render(float noise, bool /*usePwm*/)
    {
        // ESP/ENG: Oscillator mix (JX-style: osc1 - osc2 + noise)
        float sample1 = osc1.nextSample();
        float sample2 = osc2.nextSample() * osc2Gain;
        float output  = sample1 - sample2 + noise;

        if (filter != nullptr)
            output = filter->render(output);

        // ESP/ENG: Advance envelopes per sample
        float envelope = env.nextValue();
        filterEnvValue = filterEnv.nextValue();

        return output * envelope * velocityGain;
    }

    //==========================================================================
    // ESP: Dispara la fase de release (apagado) de la voz.
    // ENG: Triggers the voice release stage.
    void release()
    {
        released = true;
        env.release();
        filterEnv.release();
    }

    //==========================================================================
    // ESP: Actualiza el paneo con una lógica tipo "piano spread":
    //      graves más al centro y agudos con mayor estéreo.
    //      panning se calcula con ley de potencia constante usando seno.
    // ENG: Updates panning using a "piano spread" logic:
    //      low notes stay centered, higher notes get wider stereo.
    //      Uses a constant-power panning law via sine.
    void updatePanning(int /*voiceIndex*/)
    {
        // ESP: t controla la intensidad del paneo según la altura de la nota.
        //      Notas < 40: casi mono. Notas más altas: más estéreo.
        // ENG: t controls panning intensity based on note pitch.
        //      Notes < 40: near mono. Higher notes: more stereo.
        float t = std::clamp((note - 40.0f) / 48.0f, 0.0f, 1.0f);

        // ESP: Dirección base (placeholder). Aquí se puede mapear por voz o por registro.
        // ENG: Base direction (placeholder). Can be mapped by voice or register.
        float pianoDirection = 1.0f;

        // ESP: Mezcla de paneo base con un toque de randomPan para variación.
        // ENG: Blend base panning with a bit of randomPan for variation.
        float panningBase = (pianoDirection * 0.8f) + (randomPan * 0.2f);

        // ESP: Aplica ancho estéreo y la intensidad por nota (t).
        // ENG: Apply stereo width and per-note intensity (t).
        float panning = panningBase * stereoWidth * t;

        // ESP/ENG: Constant-power panning law
        panLeft  = std::sin(PI_OVER_4 * (1.0f - panning));
        panRight = std::sin(PI_OVER_4 * (1.0f + panning));
    }

    //==========================================================================
    // ESP: Estado expuesto del filtro (últimos valores configurados).
    // ENG: Exposed filter state (last configured values).
    float filterCutoffHz = 1000.0f;
    float filterRes = 0.0f;

    //==========================================================================
    // ESP: Configura y actualiza coeficientes del filtro activo.
    // ENG: Sets and updates coefficients of the active filter.
    void setFilter(float cutoffHz, float resonance)
    {
        filterCutoffHz   = cutoffHz;
        filterResonance  = resonance;

        if (filter != nullptr)
            filter->updateCoefficients(filterCutoffHz, filterResonance);
    }
};
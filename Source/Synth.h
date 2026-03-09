/*
  ==============================================================================
    Synth.h

    ESP:
    Motor principal del sintetizador. Administra:
      - Polifonía/monofonía y asignación de voces (voice allocation)
      - Parámetros globales (ADSR, mezcla de osciladores, glide, LFO, filtro, etc.)
      - Manejo de MIDI (note on/off, pitch bend, CC, aftertouch)
      - Render de audio estéreo (mezcla de voces + saturación suave + protección)

    Este archivo define el "estado global" del instrumento y las funciones
    públicas que el PluginProcessor suele llamar (allocateResources, reset,
    render, midiMessage).

    ENG:
    Main synth engine. Manages:
      - Poly/mono voice allocation
      - Global parameters (ADSR, oscillator mix, glide, LFO, filter, etc.)
      - MIDI handling (note on/off, pitch bend, CC, aftertouch)
      - Stereo audio rendering (voice summing + gentle saturation + safety)

    This file defines the instrument's global state and the public methods
    typically called by the PluginProcessor (allocateResources, reset, render,
    midiMessage).
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Voice.h"
#include "NoiseGenerator.h"
#include "Oscillator.h"
#include "Constants.h"

class Synth
{
public:
    Synth();
    ~Synth();

    //==========================================================================
    // ESP: Parámetros globales (normalmente controlados por UI/APVTS).
    //      Nota: estos valores se interpretan dentro del motor en render()/noteOn().
    // ENG: Global parameters (typically driven by UI/APVTS).
    int   numVoices{};
    float envAttack{};
    float envDecay{};
    float envSustain{};
    float envRelease{};

    float oscMix{};
    float detune{};
    float tune{};
    float pitchBend{};
    float stereoWidth{};
    float volumeTrim{};

    float velocitySensitivity = 0.0f;
    bool  ignoreVelocity = false;

    //==========================================================================
    // ESP: LFO global (principalmente vibrato y modulación PWM / filtro).
    // ENG: Global LFO (primarily vibrato and PWM / filter modulation).
    float lfoRateHz      = 5.0f;
    float lfoDepthSemis  = 0.0f;
    float pwmDepth       = 0.0f;

    //==========================================================================
    // ESP: Glide/portamento.
    //      glideMode: define el comportamiento mono/legato (según implementación).
    // ENG: Glide/portamento.
    int   glideMode = 0;
    float glideRate = 1.0f;
    float glideBend = 0.0f;

    //==========================================================================
    // ESP: Filtro global (base). La modulación por voz se calcula en render().
    // ENG: Global filter base. Per-voice modulation is computed in render().
    float filterCutoff    = 5000.0f;
    float filterResonance = 0.0f;

    //==========================================================================
    // ESP: Key tracking del filtro (seguimiento de nota).
    //      amount=0 desactiva; amount=1 tracking completo relativo a keycenter.
    // ENG: Filter key tracking.
    float filterKeytrackAmount = 0.0f;
    int   filterKeycenterNote  = 60;   // C4

    //==========================================================================
    // ESP: Modulación adicional del filtro (LFO/zip/envelope/velocidad).
    // ENG: Additional filter modulation (LFO/zip/envelope/velocity).
    float filterLFODepthSemis = 0.0f;
    float filterZipSemis      = 0.0f;

    float filterEnvAmountSemis  = 0.0f; // bipolar
    float filterVelocityAmount  = 0.0f;

    float filterEnvAttack{};
    float filterEnvDecay{};
    float filterEnvSustain{};
    float filterEnvRelease{};

    //==========================================================================
    // ESP: Suavizador de nivel de salida (evita clicks al cambiar volumen/estado).
    // ENG: Output level smoother (avoids clicks on gain/state changes).
    juce::LinearSmoothedValue<float> outputLevelSmoother;

    //==========================================================================
    // ESP: Selección de tipo de filtro (implementaciones internas).
    // ENG: Filter type selection (internal implementations).
    enum FilterType
    {
        FILTER_SVF  = 0,   // State Variable Filter
        FILTER_MOOG = 1    // Moog Ladder (Huovilainen)
    };

    FilterType filterType = FILTER_SVF;   // default

    // ESP/ENG: Cambia el tipo de filtro para nuevas configuraciones por voz.
    void setFilterType(FilterType type);

    //==========================================================================
    // ESP: Estado de modulación MIDI-CC (NO escribe en APVTS).
    //      Se usa para controlar modulación en tiempo real sin “pelear” con parámetros.
    // ENG: MIDI CC modulation state (NO APVTS writes).
    struct CCState
    {
        float modWheel   = 0.0f; // CC1   [0..1]
        float expression = 1.0f; // CC11  [0..1] unity=1
        float brightness = 0.0f; // CC74  [0..1]
        float resonance  = 0.0f; // CC71  [0..1]
        float attack     = 1.0f; // CC73  [0..1]
        float release    = 1.0f; // CC72  [0..1]
        bool  sustain    = false;// CC64
        float aftertouch = 0.0f; // channel pressure [0..1]
        float filterPlus  = 0.0f;// custom (0..1)
        float filterMinus = 0.0f;// custom (0..1)
    };

    void setCCState(const CCState& s);

    //==========================================================================
    // ESP: Ciclo de vida del motor.
    // ENG: Engine lifecycle.
    void allocateResources(double sampleRate, int samplesPerBlock);
    // void deallocateResources();
    void reset();

    //==========================================================================
    // ESP: Audio + MIDI.
    // ENG: Audio + MIDI.
    void render(float** outputBuffers, int sampleCount);
    void midiMessage(uint8_t data0, uint8_t data1, uint8_t data2);

    //==========================================================================
    // ESP: Gestión de voces.
    // ENG: Voice management.
    void startVoice(int v, int note, int velocity);
    int  findVoiceForNote(int note) const;
    int  findFreeVoice(int note) const;
    float calcBaseFreq(int v, int note) const;

    //==========================================================================
    // ESP: MIDI CC y utilidades de modulación.
    // ENG: MIDI CC and modulation utilities.
    void controlChange(uint8_t data1, uint8_t data2);
    void setLfoRateHz(float hz);
    void setLfoDepthSemis(float semis);
    void setPwmDepth(float depth);

    //==========================================================================
    // ESP: Selección de waveform por oscilador.
    // ENG: Oscillator waveform selection.
    void setOsc1Wave(WaveType wt);
    void setOsc2Wave(WaveType wt);

    //==========================================================================
    // ESP: Key tracking: cutoff por nota (base + tracking).
    // ENG: Key tracking: per-note cutoff from base + tracking.
    float calculateKeyTrackedCutoff(int midiNote, float baseCutoff) const;

    //==========================================================================
    // ESP: Mezcla de ruido global.
    // ENG: Global noise mix.
    float noiseMix{};

private:
    //==========================================================================
    // ESP: Waveforms actuales (cache global) para aplicar a voces.
    // ENG: Current waveforms (global cache) applied to voices.
    WaveType osc1Wave = WaveType::Saw;
    WaveType osc2Wave = WaveType::Saw;

    //==========================================================================
    // ESP: “Key stack” para lógica mono/legato (últimas teclas presionadas).
    // ENG: Key stack for mono/legato logic (last pressed keys).
    std::array<bool, 128> keyDown{};
    std::array<int,  128> keyStack{};
    int keyStackSize = 0;

    bool noteIsDown(int note) const;
    void pushKey(int note);
    void releaseKey(int note);
    int  topKey() const;                       // last pressed or -1
    bool legatoOnThisNoteOn(int note) const;   // true if another key was already held

    void noteOn(int note, int velocity);
    void noteOff(int note);
    void restartMonoVoice(int note, int velocity);

    // ESP/ENG: Conveniencia para saber si el stack implica legato.
    bool isPlayingLegatoStyle() const;
    int  lastNote = -1;

    //==========================================================================
    // ESP: Sustain pedal (CC64).
    // ENG: Sustain pedal (CC64).
    bool sustainPedalPressed{};

    //==========================================================================
    // ESP: Recursos internos.
    // ENG: Internal resources.
    float sampleRate{};
    std::array<Voice, MAX_VOICES> voices;
    NoiseGenerator noiseGen;
    juce::Random rng;

    //==========================================================================
    // ESP: LFO global.
    // ENG: Global LFO.
    Oscillator lfo;
    int   lfoCounter = 0;
    float lfoPitchMul = 1.0f;

    //==========================================================================
    // ESP: Estado CC de modulación (runtime).
    // ENG: Runtime CC modulation state.
    CCState cc;
};
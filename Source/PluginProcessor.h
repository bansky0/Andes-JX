/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

/*
    Module: AndesJXAudioProcessor (header)
    Purpose:
        EN: Plugin entry point. Bridges the host (DAW) and the synthesis
            engine: declares the AudioProcessor JUCE class, owns the
            APVTS parameter tree, captures MIDI, and routes everything
            into the Synth instance.
        ES: Punto de entrada del plugin. Hace de puente entre el host
            (DAW) y el motor de síntesis: declara la clase AudioProcessor
            de JUCE, posee el árbol de parámetros APVTS, captura MIDI y
            enruta todo hacia la instancia de Synth.

    Main responsibilities:
        EN:
          - Implement the JUCE AudioProcessor lifecycle (prepareToPlay,
            processBlock, releaseResources, reset)
          - Declare the 32 plugin parameters and bind them to the APVTS
          - Receive MIDI from the host and forward it to Synth
          - Manage factory presets (load, save, custom-preset detection)
          - Persist the plugin state for session save/load
        ES:
          - Implementar el ciclo de vida AudioProcessor de JUCE
            (prepareToPlay, processBlock, releaseResources, reset)
          - Declarar los 32 parámetros del plugin y enlazarlos al APVTS
          - Recibir MIDI del host y reenviarlo a Synth
          - Gestionar presets de fábrica (cargar, guardar, detección
            de preset custom)
          - Persistir el estado del plugin para guardar/cargar sesión

    Architectural role:
        EN: Sits between host and Synth. The host calls processBlock()
            with audio + MIDI; this class converts APVTS parameter
            values into Synth fields, dispatches MIDI events, and asks
            Synth to render the audio. Synth never talks to JUCE
            classes directly: this is the boundary layer.
        ES: Vive entre el host y Synth. El host llama a processBlock()
            con audio + MIDI; esta clase convierte los valores de
            parámetros APVTS en campos de Synth, despacha los eventos
            MIDI y pide a Synth que renderice el audio. Synth nunca
            habla con clases JUCE directamente: esta es la capa
            frontera.

    Notes:
        EN:
          - The parameter IDs in the namespace below MUST match (in
            order) the layout in NUM_PARAMS (Constants.h) and the
            constructor of Preset (Preset.h). Adding a parameter
            requires updating all three places.
          - Inheritance is triple: AudioProcessor (the plugin contract),
            ValueTree::Listener (to react to parameter changes), and
            ChangeBroadcaster (to notify the editor when the
            "isCustomPreset" flag flips).
          - MIDI CC values are stored as std::atomic to be safely shared
            between the MIDI thread and the audio thread without locks.
        ES:
          - Los IDs de parámetro del namespace de abajo DEBEN coincidir
            (en orden) con la disposición de NUM_PARAMS (Constants.h)
            y con el constructor de Preset (Preset.h). Añadir un
            parámetro obliga a actualizar los tres lugares.
          - La herencia es triple: AudioProcessor (el contrato del
            plugin), ValueTree::Listener (para reaccionar a cambios de
            parámetros) y ChangeBroadcaster (para notificar al editor
            cuando cambia el flag "isCustomPreset").
          - Los valores de los CC MIDI se almacenan como std::atomic
            para compartirlos sin locks entre el hilo MIDI y el hilo
            de audio.
*/

#pragma once

#include <JuceHeader.h>
#include "Synth.h"
#include "Preset.h"
#include <juce_dsp/juce_dsp.h>


// ============================================================================
//  PARAMETER IDS / IDS DE PARÁMETROS
// ============================================================================

// EN: Macro that declares a JUCE ParameterID with the same name and ID
//     string. Avoids repeating  juce::ParameterID id { "id", 1 };  for
//     every parameter. The "1" is the parameter version hint for hosts
//     that track parameter compatibility across plugin versions.
// ES: Macro que declara un juce::ParameterID con el mismo nombre y string
//     de ID. Evita repetir  juce::ParameterID id { "id", 1 };  para cada
//     parámetro. El "1" es el hint de versión de parámetro para hosts
//     que rastrean la compatibilidad entre versiones del plugin.
#define PARAMETER_ID(str) inline const juce::ParameterID str { #str, 1 };

// EN: The 32 parameter IDs of AndesJX. Their order is the canonical
//     reference for the entire codebase: it must match the index order
//     used in Preset.h (param[0] = osc1Wave, ..., param[31] = stereoWidth)
//     and the size declared in Constants.h::NUM_PARAMS.
// ES: Los 32 IDs de parámetro de AndesJX. Su orden es la referencia
//     canónica para todo el código: debe coincidir con el orden de
//     índices usado en Preset.h (param[0] = osc1Wave, ...,
//     param[31] = stereoWidth) y con el tamaño declarado en
//     Constants.h::NUM_PARAMS.
namespace ParameterID
{
    PARAMETER_ID(osc1Wave)
        PARAMETER_ID(osc2Wave)
        PARAMETER_ID(oscMix)
        PARAMETER_ID(oscTune)
        PARAMETER_ID(oscFine)
        PARAMETER_ID(glideMode)
        PARAMETER_ID(glideRate)
        PARAMETER_ID(glideBend)
        PARAMETER_ID(filterType)
        PARAMETER_ID(filterFreq)
        PARAMETER_ID(filterReso)
        PARAMETER_ID(filterEnv)
        PARAMETER_ID(filterLFO)
        PARAMETER_ID(filterVelocity)
        PARAMETER_ID(filterKeytrack)
        PARAMETER_ID(filterKeycenter)
        PARAMETER_ID(filterAttack)
        PARAMETER_ID(filterDecay)
        PARAMETER_ID(filterSustain)
        PARAMETER_ID(filterRelease)
        PARAMETER_ID(envAttack)
        PARAMETER_ID(envDecay)
        PARAMETER_ID(envSustain)
        PARAMETER_ID(envRelease)
        PARAMETER_ID(lfoRate)
        PARAMETER_ID(vibrato)
        PARAMETER_ID(noise)
        PARAMETER_ID(octave)
        PARAMETER_ID(tuning)
        PARAMETER_ID(outputLevel)
        PARAMETER_ID(polyMode)
        PARAMETER_ID(stereoWidth)
}
#undef PARAMETER_ID


//==============================================================================

class AndesJXAudioProcessor : public juce::AudioProcessor,
    private juce::ValueTree::Listener,
    public juce::ChangeBroadcaster
{
public:
    //==============================================================================
    AndesJXAudioProcessor();
    ~AndesJXAudioProcessor() override;


    // ========================================================================
    //  AUDIO LIFECYCLE / CICLO DE VIDA DE AUDIO
    // ========================================================================

    // EN: Called by the host before playback starts. Forwards sample rate
    //     and block size into Synth.
    // ES: El host lo llama antes de iniciar la reproducción. Reenvía
    //     sample rate y tamaño de bloque a Synth.
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    void releaseResources() override;
    void reset() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    // EN: The hot path of the plugin. Called once per audio block; pulls
    //     APVTS values into Synth, dispatches MIDI events, asks Synth
    //     to render the audio.
    // ES: La ruta caliente del plugin. Se llama una vez por bloque de
    //     audio; lee valores APVTS hacia Synth, despacha eventos MIDI,
    //     y pide a Synth que renderice el audio.
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;


    // ========================================================================
    //  EDITOR / EDITOR
    // ========================================================================

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;


    // ========================================================================
    //  PLUGIN METADATA / METADATOS DEL PLUGIN
    // ========================================================================

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;


    // ========================================================================
    //  PRESET MANAGEMENT / GESTIÓN DE PRESETS
    // ========================================================================

    // EN: Standard JUCE preset interface. Each "program" maps to one
    //     entry in the `presets` vector built in createPrograms().
    // ES: Interfaz estándar de presets de JUCE. Cada "program"
    //     corresponde a una entrada del vector `presets` que construye
    //     createPrograms().
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;


    // ========================================================================
    //  SESSION STATE / ESTADO DE SESIÓN
    // ========================================================================

    // EN: Persist and restore the full plugin state across DAW sessions.
    //     The host calls these when saving / loading a project.
    // ES: Persistir y restaurar el estado completo del plugin entre
    //     sesiones del DAW. El host las llama al guardar / cargar un
    //     proyecto.
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;


    // ========================================================================
    //  CUSTOM-PRESET QUERIES / CONSULTAS DE PRESET CUSTOM
    // ========================================================================

    // EN: True when the current parameter values no longer match the
    //     factory preset they were loaded from. Used by the editor to
    //     show an "edited" indicator.
    // ES: Verdadero cuando los valores actuales de parámetros ya no
    //     coinciden con el preset de fábrica del que fueron cargados.
    //     El editor lo usa para mostrar un indicador de "editado".
    bool isCustomPresetActive() const noexcept { return isCustomPreset; }

    // EN: True while a preset is in the middle of being loaded. Used to
    //     suppress the "preset became custom" detection during the load
    //     itself (avoids false positives).
    // ES: Verdadero mientras se está cargando un preset. Se usa para
    //     suprimir la detección de "preset se volvió custom" durante la
    //     carga (evita falsos positivos).
    bool isLoadingPreset() const noexcept { return loadingPreset; }


    // ========================================================================
    //  APVTS / APVTS
    // ========================================================================

    // EN: The single AudioProcessorValueTreeState instance owns every
    //     parameter exposed to the host. Built once at construction
    //     using the layout returned by createParameterLayout().
    // ES: La única instancia de AudioProcessorValueTreeState es dueña de
    //     cada parámetro expuesto al host. Se construye una vez al
    //     construirse el plugin con el layout que devuelve
    //     createParameterLayout().
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout() };


private:
    // ========================================================================
    //  PARAMETER LAYOUT AND CHANGE TRACKING
    //  LAYOUT DE PARÁMETROS Y RASTREO DE CAMBIOS
    // ========================================================================

    // EN: Builds the APVTS parameter layout. Defines name, range, default,
    //     step, and string formatting of all 32 parameters. Implementation
    //     in PluginProcessor.cpp.
    // ES: Construye el layout de parámetros del APVTS. Define nombre,
    //     rango, valor por defecto, paso y formato de string de los 32
    //     parámetros. Implementación en PluginProcessor.cpp.
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // EN: Listener callback fired whenever any parameter changes. Two jobs:
    //       1. Set parametersChanged so processBlock() knows it must
    //          refresh Synth's fields next time it runs.
    //       2. Detect the "custom preset" transition: if the user moved
    //          a control while NOT loading a preset, the current state
    //          stops matching any factory preset; broadcast a change
    //          message so the editor can update its UI.
    // ES: Callback del listener que se dispara cuando cambia cualquier
    //     parámetro. Dos trabajos:
    //       1. Activar parametersChanged para que processBlock() sepa
    //          que debe refrescar los campos de Synth al correr.
    //       2. Detectar la transición a "custom preset": si el usuario
    //          mueve un control sin estar cargando un preset, el estado
    //          actual deja de coincidir con cualquier preset de fábrica;
    //          se broadcastea un mensaje para que el editor actualice
    //          su UI.
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) override
    {
        // EN: Identifiers stored as static so they hash once.
        // ES: Identifiers static para que se hasheen una sola vez.
        static const juce::Identifier currentProgramID{ "currentProgram" };
        static const juce::Identifier isCustomPresetID{ "isCustomPreset" };

        parametersChanged.store(true);

        // EN: Skip the custom-preset check while a preset is loading,
        //     and ignore changes to the bookkeeping properties themselves.
        // ES: Omitir el chequeo de custom-preset mientras se carga un
        //     preset, e ignorar cambios en las propiedades de
        //     bookkeeping mismas.
        if (!loadingPreset)
        {
            if (property != currentProgramID && property != isCustomPresetID)
            {
                const bool newIsCustom = !currentStateMatchesProgram();

                if (isCustomPreset != newIsCustom)
                {
                    isCustomPreset = newIsCustom;
                    sendChangeMessage();
                }
            }
        }
    }

    // EN: Atomic flag set by the listener and cleared by processBlock()
    //     after pulling parameter values into Synth. The atomic avoids
    //     a lock between the message thread (where the listener fires)
    //     and the audio thread (where processBlock reads it).
    // ES: Bandera atómica que activa el listener y limpia processBlock()
    //     tras leer los valores de parámetros hacia Synth. El atomic
    //     evita un lock entre el hilo de mensajes (donde dispara el
    //     listener) y el hilo de audio (donde processBlock la lee).
    std::atomic<bool> parametersChanged{ false };


    // ========================================================================
    //  INTERNAL AUDIO HELPERS / HELPERS INTERNOS DE AUDIO
    // ========================================================================

    // EN: Reads APVTS values and writes them into the corresponding
    //     fields of Synth. Called from processBlock() when
    //     parametersChanged is true.
    // ES: Lee los valores del APVTS y los escribe en los campos
    //     correspondientes de Synth. Se llama desde processBlock()
    //     cuando parametersChanged es true.
    void update();

    // EN: Builds the AndesJX factory preset bank. Each entry is a Preset
    //     constructed with its 32 parameter values in canonical order.
    // ES: Construye el banco de presets de fábrica de AndesJX. Cada
    //     entrada es un Preset construido con sus 32 valores de
    //     parámetros en el orden canónico.
    void createPrograms();

    // EN: Splits the audio block at every MIDI event so that note-ons
    //     and CCs land on the correct sample boundary, then renders
    //     each sub-block. The "Optimized" variant uses juce::dsp blocks.
    // ES: Divide el bloque de audio en cada evento MIDI para que los
    //     note-ons y CCs caigan en la frontera de muestra correcta, y
    //     luego renderiza cada sub-bloque. La variante "Optimized" usa
    //     bloques juce::dsp.
    void splitBufferByEvents(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void splitBufferByEventsOptimized(juce::dsp::AudioBlock<float>& block, juce::MidiBuffer& midiMessages);

    // EN: Routes a single MIDI message: captures CCs into the std::atomic
    //     fields below, forwards everything to Synth.
    // ES: Enruta un único mensaje MIDI: captura los CCs en los campos
    //     std::atomic de abajo y reenvía todo a Synth.
    void handleMIDI(uint8_t data0, uint8_t data1, uint8_t data2);

    // EN: Renders `sampleCount` frames starting at offset `bufferOffset`
    //     within the supplied audio buffer. Thin wrapper over
    //     Synth::render().
    // ES: Renderiza `sampleCount` cuadros a partir del offset
    //     `bufferOffset` dentro del buffer de audio dado. Wrapper fino
    //     sobre Synth::render().
    void render(juce::AudioBuffer<float>& buffer, int sampleCount, int bufferOffset);


    // ========================================================================
    //  PRESET STATE / ESTADO DE PRESETS
    // ========================================================================

    // EN: Factory preset bank, populated by createPrograms() in
    //     PluginProcessor.cpp. Indexed by currentProgram.
    // ES: Banco de presets de fábrica, lo puebla createPrograms() en
    //     PluginProcessor.cpp. Se indexa con currentProgram.
    std::vector<Preset> presets;
    int  currentProgram = 0;
    bool isCustomPreset = false;
    bool loadingPreset = false;

    // EN: Compares the current APVTS values against the active preset
    //     and returns true if they all match. Used by the listener to
    //     detect the "edited" state.
    // ES: Compara los valores actuales del APVTS con el preset activo
    //     y devuelve true si todos coinciden. Lo usa el listener para
    //     detectar el estado "editado".
    bool currentStateMatchesProgram() const;


    // ========================================================================
    //  SYNTH ENGINE / MOTOR DE SÍNTESIS
    // ========================================================================

    Synth synth;


    // ========================================================================
    //  CACHED APVTS PARAMETER POINTERS
    //  PUNTEROS CACHEADOS A PARÁMETROS DEL APVTS
    // ========================================================================

    // EN: Cached typed pointers to the APVTS parameters. They are
    //     resolved once at construction (via castParameter in Utils.h)
    //     so that update() can read the values without doing a lookup
    //     every block.
    // ES: Punteros tipados cacheados a los parámetros del APVTS. Se
    //     resuelven una sola vez en la construcción (con castParameter
    //     de Utils.h) para que update() lea los valores sin hacer
    //     lookup en cada bloque.
    juce::AudioParameterChoice* osc1WaveParam;
    juce::AudioParameterChoice* osc2WaveParam;
    juce::AudioParameterFloat* oscMixParam;
    juce::AudioParameterFloat* oscTuneParam;
    juce::AudioParameterFloat* oscFineParam;
    juce::AudioParameterChoice* glideModeParam;
    juce::AudioParameterFloat* glideRateParam;
    juce::AudioParameterFloat* glideBendParam;
    juce::AudioParameterChoice* filterTypeParam;
    juce::AudioParameterFloat* filterFreqParam;
    juce::AudioParameterFloat* filterResoParam;
    juce::AudioParameterFloat* filterEnvParam;
    juce::AudioParameterFloat* filterLFOParam;
    juce::AudioParameterFloat* filterVelocityParam;
    juce::AudioParameterFloat* filterKeytrackParam;
    juce::AudioParameterFloat* filterKeycenterParam;
    juce::AudioParameterFloat* filterAttackParam;
    juce::AudioParameterFloat* filterDecayParam;
    juce::AudioParameterFloat* filterSustainParam;
    juce::AudioParameterFloat* filterReleaseParam;
    juce::AudioParameterFloat* envAttackParam;
    juce::AudioParameterFloat* envDecayParam;
    juce::AudioParameterFloat* envSustainParam;
    juce::AudioParameterFloat* envReleaseParam;
    juce::AudioParameterFloat* lfoRateParam;
    juce::AudioParameterFloat* vibratoParam;
    juce::AudioParameterFloat* noiseParam;
    juce::AudioParameterFloat* octaveParam;
    juce::AudioParameterFloat* tuningParam;
    juce::AudioParameterFloat* outputLevelParam;
    juce::AudioParameterChoice* polyModeParam;
    juce::AudioParameterFloat* stereoWidthParam;


    // ========================================================================
    //  MIDI CC ATOMIC STATE / ESTADO ATÓMICO DE LOS CC MIDI
    // ========================================================================

    // EN: MIDI CC values captured by handleMIDI() and consumed by Synth
    //     through CCState. std::atomic guarantees a safe handoff
    //     between the MIDI thread (writes) and the audio thread (reads)
    //     without using locks.
    // ES: Valores MIDI CC que captura handleMIDI() y consume Synth a
    //     través de CCState. std::atomic garantiza un traspaso seguro
    //     entre el hilo MIDI (escribe) y el hilo de audio (lee) sin
    //     usar locks.
    std::atomic<float> ccModWheel{ 0.0f };  // EN: CC1   |  ES: CC1
    std::atomic<float> ccExpression{ 1.0f };  // EN: CC11 (1 = unity)  |  ES: CC11 (1 = unidad)
    std::atomic<float> ccBrightness{ 0.0f };  // EN: CC74  |  ES: CC74
    std::atomic<float> ccResonance{ 0.0f };  // EN: CC71  |  ES: CC71
    std::atomic<float> ccAttack{ 0.0f };  // EN: CC73  |  ES: CC73
    std::atomic<float> ccRelease{ 0.0f };  // EN: CC72  |  ES: CC72
    std::atomic<bool>  ccSustainDown{ false }; // EN: CC64  |  ES: CC64


    // ========================================================================
    //  OVERSAMPLING / OVERSAMPLING
    // ========================================================================

    // EN: Optional juce::dsp oversampler. Its actual usage lives in
    //     PluginProcessor.cpp; if it is not currently wired into the
    //     audio path, it is reserved for future quality modes.
    // ES: Oversampler opcional de juce::dsp. Su uso real vive en
    //     PluginProcessor.cpp; si no está actualmente conectado al
    //     camino de audio, queda reservado para modos de calidad
    //     futuros.
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AndesJXAudioProcessor)
};
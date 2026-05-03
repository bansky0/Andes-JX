/*
  ==============================================================================

    Synth.h
    Created: 10 Nov 2025 6:45:11pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: Synth
    Purpose:
        EN: Polyphonic synthesis engine. Owns the voice pool, routes MIDI
            events to voices, applies global modulation (LFO, filter
            envelope, key tracking, pitch bend, MIDI CC), and renders the
            final stereo output to the host.
        ES: Motor de síntesis polifónica. Posee el pool de voces, enruta
            eventos MIDI hacia las voces, aplica modulación global (LFO,
            envolvente de filtro, key tracking, pitch bend, MIDI CC) y
            renderiza la salida estéreo final hacia el host.

    Main responsibilities:
        EN:
          - Maintain the voice pool of size MAX_VOICES
          - Translate MIDI note-on / note-off / CC messages into voice
            actions (start, release, modulation)
          - Implement voice allocation (free-voice search and stealing)
          - Track which keys are physically down to support legato
          - Run a shared LFO and a key-track calculation each block
          - Sum and pan all active voices into the stereo output buffer
        ES:
          - Mantener el pool de voces de tamaño MAX_VOICES
          - Traducir mensajes MIDI note-on / note-off / CC en acciones de
            voz (arranque, liberación, modulación)
          - Implementar asignación de voces (búsqueda libre y robo)
          - Rastrear qué teclas están físicamente pulsadas para soportar
            legato
          - Ejecutar un LFO compartido y un cálculo de key tracking por
            bloque
          - Sumar y panear todas las voces activas en el buffer estéreo

    Architectural role:
        EN: Instantiated once by PluginProcessor. Does not talk to the host
            directly: PluginProcessor forwards MIDI in, calls render() each
            audio block, and passes the result to the host. Synth knows
            nothing about VST/AU/JUCE parameter trees; it only exposes
            plain fields that PluginProcessor writes to per block.
        ES: PluginProcessor lo instancia una vez. No habla directamente
            con el host: PluginProcessor reenvía el MIDI de entrada, llama
            render() en cada bloque de audio y pasa el resultado al host.
            Synth no sabe nada de árboles de parámetros VST/AU/JUCE; solo
            expone campos planos que PluginProcessor escribe por bloque.

    Notes:
        EN:
          - Parameters are public fields rather than getters/setters. This
            matches the per-block write pattern used by PluginProcessor and
            keeps the audio callback branch-free.
          - The key tracking system (keyDown + keyStack) implements
            last-note-priority legato, the behavior of classic monophonic
            synthesizers such as the Minimoog.
          - The LFO runs at reduced rate (LFO_MAX samples, see Constants.h)
            because it is sub-audio and need not be computed every sample.
          - Current filter choices are SVF (clean, 24 dB/oct cascade) and
            Moog (warm, saturated ladder). They are selected at runtime
            through the FilterType enum.
        ES:
          - Los parámetros son campos públicos en vez de getters/setters.
            Se alinea con el patrón de escritura por bloque que usa
            PluginProcessor y mantiene al callback de audio sin ramas.
          - El sistema de key tracking (keyDown + keyStack) implementa
            prioridad última-nota para el legato, como los sintetizadores
            monofónicos clásicos tipo Minimoog.
          - El LFO corre a tasa reducida (LFO_MAX muestras, ver
            Constants.h) porque es sub-audio y no hace falta calcularlo
            en cada muestra.
          - Las opciones actuales de filtro son SVF (limpio, cascada
            24 dB/oct) y Moog (cálido, ladder saturado). Se seleccionan
            en tiempo de ejecución mediante el enum FilterType.
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


    // ========================================================================
    //  PUBLIC PARAMETERS - written by PluginProcessor each audio block
    //  PARÁMETROS PÚBLICOS - los escribe PluginProcessor cada bloque de audio
    // ========================================================================

    // --- Voice configuration / Configuración de voces ----------------------

    // EN: Number of active polyphony voices (<= MAX_VOICES).
    // ES: Número de voces polifónicas activas (<= MAX_VOICES).
    int numVoices{};


    // --- Amplitude envelope / Envolvente de amplitud -----------------------

    float envAttack{};
    float envDecay{};
    float envSustain{};
    float envRelease{};


    // --- Oscillator mix / Mezcla de osciladores ----------------------------

    // EN: Balance between osc1 and osc2 (0 = only osc1, 1 = only osc2).
    // ES: Balance entre osc1 y osc2 (0 = solo osc1, 1 = solo osc2).
    float oscMix{};

    // EN: Fine detune in cents applied to osc2 relative to osc1.
    // ES: Detune fino en cents aplicado a osc2 respecto a osc1.
    float detune{};

    // EN: Global tuning offset in semitones.
    // ES: Desplazamiento global de afinación en semitonos.
    float tune{};


    // --- Real-time modulation / Modulación en tiempo real ------------------

    // EN: Pitch bend wheel, expressed as a semitone offset.
    // ES: Rueda de pitch bend, expresada como desplazamiento en semitonos.
    float pitchBend{};

    // EN: Stereo width applied to the per-voice pan law.
    // ES: Ancho estéreo aplicado sobre la ley de paneo por voz.
    float stereoWidth{};

    // EN: Extra output gain trim, typically used for preset calibration.
    // ES: Ganancia extra de salida, típicamente para calibrar presets.
    float volumeTrim{};

    // EN: 0 = velocity ignored; higher values scale its effect on volume.
    // ES: 0 = se ignora la velocity; valores mayores escalan su efecto
    //     sobre el volumen.
    float velocitySensitivity = 0.0f;

    // EN: Hard switch to bypass velocity and always play at full loudness.
    // ES: Interruptor directo para ignorar velocity y tocar siempre a
    //     volumen completo.
    bool ignoreVelocity = false;


    // --- LFO / LFO ---------------------------------------------------------

    // EN: Global LFO rate in Hz.
    // ES: Frecuencia del LFO global en Hz.
    float lfoRateHz = 5.0f;

    // EN: LFO depth applied to pitch, in semitones.
    // ES: Profundidad del LFO aplicada al pitch, en semitonos.
    float lfoDepthSemis = 0.0f;

    // EN: PWM modulation depth driven by the LFO on osc2 pulse width.
    // ES: Profundidad PWM del LFO aplicada al ancho de pulso de osc2.
    float pwmDepth = 0.0f;


    // --- Glide / Glide -----------------------------------------------------

    // EN: Glide mode selector (0 = off, 1 = legato only, 2 = always).
    // ES: Selector de modo glide (0 = off, 1 = solo legato, 2 = siempre).
    int glideMode = 0;

    // EN: Glide smoothing coefficient (1.0 = instant, lower = slower).
    // ES: Coeficiente de suavizado del glide (1.0 = instantáneo, menor = más lento).
    float glideRate = 1.0f;

    // EN: Initial pitch bend at note start, in semitones (drift effect).
    // ES: Curvatura inicial al arrancar la nota, en semitonos (efecto drift).
    float glideBend = 0.0f;


    // --- Filter / Filtro ---------------------------------------------------

    // EN: Base filter cutoff in Hz, before key tracking and modulation.
    // ES: Cutoff base del filtro en Hz, antes de key tracking y modulación.
    float filterCutoff = 5000.0f;

    // EN: Normalized filter resonance in [0, 1]. The active filter wrapper
    //     converts this to its internal Q units.
    // ES: Resonancia normalizada del filtro en [0, 1]. El wrapper del filtro
    //     activo la convierte a sus unidades internas de Q.
    float filterResonance = 0.0f;

    // EN: Key tracking amount. 0 = cutoff constant across keyboard,
    //     1 = cutoff follows pitch 1:1 (one octave per octave).
    // ES: Cantidad de key tracking. 0 = cutoff constante en todo el teclado,
    //     1 = el cutoff sigue el pitch 1:1 (una octava por octava).
    float filterKeytrackAmount = 0.0f;

    // EN: Reference note for key tracking. Notes above this open the
    //     filter, notes below close it. Default C4 (MIDI 60).
    // ES: Nota de referencia del key tracking. Notas por encima abren el
    //     filtro, por debajo lo cierran. Por defecto C4 (MIDI 60).
    int filterKeycenterNote = 60;

    // EN: Depth of the LFO routed to the filter cutoff, in semitones.
    // ES: Profundidad del LFO enrutado al cutoff del filtro, en semitonos.
    float filterLFODepthSemis = 0.0f;

    // EN: Smoothed cutoff offset in semitones; prevents zipper noise when
    //     the user sweeps the cutoff control.
    // ES: Offset de cutoff suavizado en semitonos; evita el ruido de zipper
    //     cuando el usuario mueve el control de cutoff.
    float filterZipSemis = 0.0f;

    // EN: Bipolar amount of the filter envelope applied to the cutoff,
    //     in semitones (positive opens, negative closes).
    // ES: Cantidad bipolar de la envolvente de filtro aplicada al cutoff,
    //     en semitonos (positivo abre, negativo cierra).
    float filterEnvAmountSemis = 0.0f;

    // EN: How much MIDI velocity modulates the filter cutoff.
    // ES: Cuánto modula la velocity MIDI al cutoff del filtro.
    float filterVelocityAmount = 0.0f;


    // --- Filter envelope / Envolvente del filtro ---------------------------

    float filterEnvAttack{};
    float filterEnvDecay{};
    float filterEnvSustain{};
    float filterEnvRelease{};


    // --- Output / Salida ---------------------------------------------------

    // EN: Smoothed output level to avoid zipper noise on volume changes.
    // ES: Nivel de salida suavizado para evitar zipper noise al cambiar volumen.
    juce::LinearSmoothedValue<float> outputLevelSmoother;

    // EN: Amount of noise mixed into each voice.
    // ES: Cantidad de ruido añadida a cada voz.
    float noiseMix{};


    // ========================================================================
    //  FILTER SELECTION / SELECCIÓN DE FILTRO
    // ========================================================================

    // EN: Available filter algorithms. Voice holds instances of both
    //     wrappers (SVFFilter, MoogFilter); this enum chooses which one
    //     is assigned to each voice's `filter` pointer.
    // ES: Algoritmos de filtro disponibles. Voice posee instancias de
    //     ambos wrappers (SVFFilter, MoogFilter); este enum elige cuál
    //     se asigna al puntero `filter` de cada voz.
    enum class FilterType
    {
        SVF = 0, // EN: State Variable (Simper ZDF)  |  ES: State Variable (Simper ZDF)
        Moog = 1  // EN: Moog Ladder (Huovilainen)    |  ES: Moog Ladder (Huovilainen)
    };

    // EN: Currently selected filter algorithm. Default is SVF.
    // ES: Algoritmo de filtro seleccionado actualmente. Por defecto SVF.
    FilterType filterType = FilterType::SVF;

    // EN: Switches the active filter for every voice. Implementation
    //     lives in Synth.cpp.
    // ES: Intercambia el filtro activo de cada voz. La implementación
    //     está en Synth.cpp.
    void setFilterType(FilterType type);


    // ========================================================================
    //  MIDI CC STATE / ESTADO DE LOS CC MIDI
    // ========================================================================

    // EN: Bundle of real-time MIDI controller values. Held here instead
    //     of being written into the plugin's parameter tree, because CCs
    //     are performance controls (wheels, pedals, aftertouch) that the
    //     host does not need to automate or persist.
    // ES: Paquete de valores MIDI CC en tiempo real. Se guarda aquí en
    //     vez de escribirse en el árbol de parámetros del plugin, porque
    //     los CCs son controles de interpretación (ruedas, pedales,
    //     aftertouch) que el host no necesita automatizar ni persistir.
    struct CCState
    {
        float modWheel = 0.0f;  // EN: CC1   [0..1]              |  ES: CC1   [0..1]
        float expression = 1.0f;  // EN: CC11  [0..1], 1 = unity   |  ES: CC11  [0..1], 1 = unidad
        float brightness = 0.0f;  // EN: CC74  [0..1]              |  ES: CC74  [0..1]
        float resonance = 0.0f;  // EN: CC71  [0..1]              |  ES: CC71  [0..1]
        float attack = 1.0f;  // EN: CC73  [0..1]              |  ES: CC73  [0..1]
        float release = 1.0f;  // EN: CC72  [0..1]              |  ES: CC72  [0..1]
        bool  sustain = false; // EN: CC64                      |  ES: CC64
        float aftertouch = 0.0f;
        float filterPlus = 0.0f;  // EN: open-cutoff amount   [0..1]   |  ES: cantidad de apertura [0..1]
        float filterMinus = 0.0f;  // EN: close-cutoff amount  [0..1]   |  ES: cantidad de cierre   [0..1]
    };

    // EN: Copies a new CC snapshot into the engine. Called by
    //     PluginProcessor whenever a CC message arrives.
    // ES: Copia un nuevo snapshot de CCs al motor. Lo llama
    //     PluginProcessor cuando llega un mensaje de CC.
    void setCCState(const CCState& s);


    // ========================================================================
    //  LIFECYCLE AND AUDIO / CICLO DE VIDA Y AUDIO
    // ========================================================================

    // EN: Called by the host before playback starts. Stores sample rate
    //     and prepares buffers for the given block size.
    // ES: El host lo llama antes de iniciar la reproducción. Guarda la
    //     sample rate y prepara buffers para el tamaño de bloque dado.
    void allocateResources(double sampleRate, int samplesPerBlock);

    // EN: Clears all voice state, the LFO, and the key tracking stack.
    //     Safe to call on preset changes or transport resets.
    // ES: Limpia el estado de todas las voces, el LFO y el stack de key
    //     tracking. Seguro de llamar al cambiar de preset o al resetear
    //     el transporte.
    void reset();

    // EN: Main audio entry point. Renders `sampleCount` frames into the
    //     stereo output buffers (outputBuffers[0] = L, outputBuffers[1] = R).
    // ES: Punto de entrada principal de audio. Renderiza `sampleCount`
    //     cuadros en los buffers de salida estéreo
    //     (outputBuffers[0] = L, outputBuffers[1] = R).
    void render(float** outputBuffers, int sampleCount);


    // ========================================================================
    //  MIDI HANDLING / MANEJO DE MIDI
    // ========================================================================

    // EN: Routes a single MIDI message to the appropriate handler
    //     (note-on, note-off, control change, pitch bend).
    // ES: Enruta un único mensaje MIDI al handler correspondiente
    //     (note-on, note-off, control change, pitch bend).
    void midiMessage(uint8_t data0, uint8_t data1, uint8_t data2);

    // EN: Starts a specific voice with the given note and velocity.
    //     Usually called by the note-on handler after voice allocation.
    // ES: Arranca una voz específica con la nota y velocity dadas.
    //     Normalmente lo llama el handler de note-on tras asignar la voz.
    void startVoice(int v, int note, int velocity);

    // EN: Returns the voice index already playing `note`, or -1 if none.
    //     Used to avoid double-triggering on duplicate note-ons.
    // ES: Devuelve el índice de la voz que ya toca `note`, o -1 si
    //     ninguna. Se usa para evitar doble disparo en note-ons duplicados.
    int findVoiceForNote(int note) const;

    // EN: Returns the index of a free voice, or the index of the voice
    //     that should be stolen if none is free.
    // ES: Devuelve el índice de una voz libre, o el de la voz que se
    //     debe robar si ninguna está libre.
    int findFreeVoice(int note) const;

    // EN: Converts MIDI note number into the base oscillator frequency
    //     for voice `v`, including analog drift (see Constants::ANALOG).
    // ES: Convierte el número de nota MIDI a la frecuencia base del
    //     oscilador para la voz `v`, incluyendo drift analógico
    //     (ver Constants::ANALOG).
    float calcBaseFreq(int v, int note) const;

    // EN: Handles a Control Change MIDI message.
    // ES: Maneja un mensaje MIDI de Control Change.
    void controlChange(uint8_t data1, uint8_t data2);


    // ========================================================================
    //  LFO AND OSCILLATOR SETTERS / SETTERS DE LFO Y OSCILADORES
    // ========================================================================

    void setLfoRateHz(float hz);
    void setLfoDepthSemis(float semis);
    void setPwmDepth(float depth);

    void setOsc1Wave(WaveType wt);
    void setOsc2Wave(WaveType wt);


    // ========================================================================
    //  KEY TRACKING / SEGUIMIENTO DE NOTA
    // ========================================================================

    // EN: Given a MIDI note and the base cutoff, returns the cutoff that
    //     results from applying filterKeytrackAmount around the center
    //     note filterKeycenterNote. Used each block per voice.
    // ES: Dada una nota MIDI y el cutoff base, devuelve el cutoff que
    //     resulta de aplicar filterKeytrackAmount alrededor de la nota
    //     central filterKeycenterNote. Se usa cada bloque por voz.
    float calculateKeyTrackedCutoff(int midiNote, float baseCutoff) const;


    // ============================================================================
    //  PRIVATE IMPLEMENTATION / IMPLEMENTACIÓN PRIVADA
    // ============================================================================
private:

    // --- Oscillator waveforms (private mirror) -----------------------------
    // --- Formas de onda de los osciladores (espejo privado) ----------------
    // EN: Cached waveform selection; the public setters propagate these
    //     to all voices on change.
    // ES: Selección de onda cacheada; los setters públicos propagan estos
    //     valores a todas las voces al cambiar.
    WaveType osc1Wave = WaveType::Saw;
    WaveType osc2Wave = WaveType::Saw;


    // --- Keyboard state for legato / Estado del teclado para legato --------
    // EN: Classic last-note-priority legato implementation: keyDown tracks
    //     which MIDI notes are physically held; keyStack records the order
    //     in which they were pressed so we can fall back to the previous
    //     held note on note-off.
    // ES: Implementación clásica de legato con prioridad a la última nota:
    //     keyDown indica qué notas MIDI están físicamente pulsadas;
    //     keyStack registra el orden en que se pulsaron para poder volver
    //     a la anterior cuando se suelta la actual.
    std::array<bool, 128> keyDown{};
    std::array<int, 128>  keyStack{};
    std::array<int, 128>  keyVelocities{};
    int keyStackSize = 0;

    bool noteIsDown(int note) const;
    void pushKey(int note);
    void releaseKey(int note);

    // EN: Returns the most recently pressed note still held, or -1.
    // ES: Devuelve la última nota pulsada que aún se mantiene, o -1.
    int  topKey() const;

    // EN: True if another key was held before this note-on (legato case).
    // ES: Verdadero si había otra tecla pulsada antes de este note-on
    //     (caso legato).
    bool legatoOnThisNoteOn(int note) const;


    // --- Internal note handlers / Handlers internos de nota ----------------

    void noteOn(int note, int velocity);
    void noteOff(int note);

    // EN: In monophonic glide modes, reuse the single active voice instead
    //     of allocating a new one, so phase and envelope state carry over.
    // ES: En modos glide monofónicos, reutiliza la única voz activa en
    //     vez de asignar una nueva, para que la fase y la envolvente se
    //     mantengan.
    void restartMonoVoice(int note, int velocity);

    bool isPlayingLegatoStyle() const;

    // EN: Last note-on seen; used by legato and glide logic.
    // ES: Último note-on visto; lo usan la lógica de legato y glide.
    int lastNote = -1;


    // --- Pedal and voice pool / Pedal y pool de voces ----------------------

    bool sustainPedalPressed{};

    float sampleRate{};

    std::array<Voice, MAX_VOICES> voices;

    // EN: Shared across all voices to produce a unified noise texture.
    // ES: Compartido por todas las voces para generar una textura de
    //     ruido unificada.
    NoiseGenerator noiseGen;

    // EN: Random source used for per-voice pan offsets and similar.
    // ES: Fuente aleatoria para offsets de paneo por voz y similares.
    juce::Random rng;


    // --- Global LFO / LFO global -------------------------------------------
    // EN: One shared LFO for all voices. Runs at reduced rate
    //     (LFO_MAX samples, see Constants.h) because it is sub-audio.
    // ES: Un único LFO compartido por todas las voces. Corre a tasa
    //     reducida (LFO_MAX muestras, ver Constants.h) por ser sub-audio.
    Oscillator lfo;
    int   lfoCounter = 0;
    float lfoValue = 0.0f;

    // EN: LFO-derived pitch multiplier applied to osc frequencies.
    // ES: Multiplicador de pitch derivado del LFO, aplicado a las
    //     frecuencias de los osciladores.
    float lfoPitchMul = 1.0f;


    // --- MIDI CC snapshot / Snapshot de MIDI CC ----------------------------
    // EN: Current CC state, updated whenever setCCState() is called.
    // ES: Estado actual de los CC, actualizado cuando se llama setCCState().
    CCState cc;
};
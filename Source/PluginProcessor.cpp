/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

/*
    Module: AndesJXAudioProcessor (implementation)
    Purpose:
        EN: Implements the JUCE AudioProcessor declared in
            PluginProcessor.h. Bridges the host (DAW) and the Synth
            engine: builds the parameter layout, captures MIDI, manages
            factory presets, persists session state, and drives the audio
            block processing.
        ES: Implementa el AudioProcessor de JUCE declarado en
            PluginProcessor.h. Hace de puente entre el host (DAW) y el
            motor Synth: construye el layout de parámetros, captura MIDI,
            gestiona los presets de fábrica, persiste el estado de sesión
            y dirige el procesamiento de bloques de audio.

    Structure / Estructura:
        EN:
          1. Construction and parameter pointer caching
          2. Plugin metadata (getName, acceptsMidi, etc.)
          3. Preset management (program lookup, load, custom detection)
          4. Audio lifecycle (prepareToPlay, releaseResources, reset)
          5. Audio processing (processBlock + split-by-events helpers)
          6. Parameter pull (update: APVTS -> Synth)
          7. MIDI dispatch + render wrapper
          8. Editor and session state
          9. Factory preset bank (createPrograms)
         10. APVTS layout (createParameterLayout)
        ES:
          1. Construcción y caché de punteros a parámetros
          2. Metadatos del plugin (getName, acceptsMidi, etc.)
          3. Gestión de presets (búsqueda, carga, detección de custom)
          4. Ciclo de vida de audio (prepareToPlay, releaseResources, reset)
          5. Procesamiento de audio (processBlock + helpers de split)
          6. Lectura de parámetros (update: APVTS -> Synth)
          7. Despacho MIDI + wrapper de render
          8. Editor y persistencia de sesión
          9. Banco de presets de fábrica (createPrograms)
         10. Layout APVTS (createParameterLayout)
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Utils.h"
#include "Constants.h"


// ============================================================================
//  1. CONSTRUCTION AND PARAMETER POINTER CACHING
//     CONSTRUCCIÓN Y CACHÉ DE PUNTEROS A PARÁMETROS
// ============================================================================

// EN: Constructor. Three jobs:
//       (1) Configure the audio bus layout (mono or stereo I/O).
//       (2) Cache typed pointers to every APVTS parameter using
//           castParameter (Utils.h). This avoids hash lookups in the
//           audio callback.
//       (3) Build the factory preset bank, load preset 0, and start
//           listening for parameter changes.
// ES: Constructor. Tres trabajos:
//       (1) Configurar el layout de buses (E/S mono o estéreo).
//       (2) Cachear punteros tipados a cada parámetro APVTS usando
//           castParameter (Utils.h). Evita lookups de hash en el
//           callback de audio.
//       (3) Construir el banco de presets de fábrica, cargar el preset 0
//           y empezar a escuchar cambios de parámetros.
AndesJXAudioProcessor::AndesJXAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
    castParameter(apvts, ParameterID::osc1Wave, osc1WaveParam);
    castParameter(apvts, ParameterID::osc2Wave, osc2WaveParam);
    castParameter(apvts, ParameterID::oscMix, oscMixParam);
    castParameter(apvts, ParameterID::oscTune, oscTuneParam);
    castParameter(apvts, ParameterID::oscFine, oscFineParam);
    castParameter(apvts, ParameterID::glideMode, glideModeParam);
    castParameter(apvts, ParameterID::glideRate, glideRateParam);
    castParameter(apvts, ParameterID::glideBend, glideBendParam);
    castParameter(apvts, ParameterID::filterFreq, filterFreqParam);
    castParameter(apvts, ParameterID::filterReso, filterResoParam);
    castParameter(apvts, ParameterID::filterEnv, filterEnvParam);
    castParameter(apvts, ParameterID::filterLFO, filterLFOParam);
    castParameter(apvts, ParameterID::filterVelocity, filterVelocityParam);
    castParameter(apvts, ParameterID::filterKeytrack, filterKeytrackParam);
    castParameter(apvts, ParameterID::filterKeycenter, filterKeycenterParam);
    castParameter(apvts, ParameterID::filterAttack, filterAttackParam);
    castParameter(apvts, ParameterID::filterDecay, filterDecayParam);
    castParameter(apvts, ParameterID::filterSustain, filterSustainParam);
    castParameter(apvts, ParameterID::filterRelease, filterReleaseParam);
    castParameter(apvts, ParameterID::envAttack, envAttackParam);
    castParameter(apvts, ParameterID::envDecay, envDecayParam);
    castParameter(apvts, ParameterID::envSustain, envSustainParam);
    castParameter(apvts, ParameterID::envRelease, envReleaseParam);
    castParameter(apvts, ParameterID::lfoRate, lfoRateParam);
    castParameter(apvts, ParameterID::vibrato, vibratoParam);
    castParameter(apvts, ParameterID::noise, noiseParam);
    castParameter(apvts, ParameterID::octave, octaveParam);
    castParameter(apvts, ParameterID::tuning, tuningParam);
    castParameter(apvts, ParameterID::outputLevel, outputLevelParam);
    castParameter(apvts, ParameterID::polyMode, polyModeParam);
    castParameter(apvts, ParameterID::stereoWidth, stereoWidthParam);
    castParameter(apvts, ParameterID::filterType, filterTypeParam);

    createPrograms();
    setCurrentProgram(0);

    // EN: Subscribe to APVTS changes so valueTreePropertyChanged()
    //     (declared inline in PluginProcessor.h) gets called whenever
    //     any parameter is touched.
    // ES: Suscribirse a los cambios del APVTS para que se invoque
    //     valueTreePropertyChanged() (declarado inline en
    //     PluginProcessor.h) cuando se toque cualquier parámetro.
    apvts.state.addListener(this);
}

// EN: Destructor. Unregisters the listener to avoid dangling callbacks
//     during teardown.
// ES: Destructor. Da de baja al listener para evitar callbacks colgantes
//     durante el cierre.
AndesJXAudioProcessor::~AndesJXAudioProcessor()
{
    apvts.state.removeListener(this);
}


// ============================================================================
//  2. PLUGIN METADATA
//     METADATOS DEL PLUGIN
// ============================================================================

// EN: Standard JUCE metadata callbacks. The macros JucePlugin_* are
//     defined by the JUCE Projucer/CMake configuration of the project,
//     so changing the plugin name or capability flags happens there,
//     not here.
// ES: Callbacks estándar de metadatos de JUCE. Los macros JucePlugin_*
//     los define la configuración del proyecto (Projucer/CMake), así
//     que cambiar el nombre del plugin o las capacidades se hace allí,
//     no aquí.

const juce::String AndesJXAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AndesJXAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AndesJXAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool AndesJXAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

// EN: Tail length is 0 because every voice's release ends at the SILENCE
//     threshold, after which the voice is recycled. There is no
//     freestanding reverb tail to wait for.
// ES: La cola es 0 porque el release de cada voz termina al cruzar el
//     umbral SILENCE y luego la voz se recicla. No hay cola de reverb
//     independiente que esperar.
double AndesJXAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}


// ============================================================================
//  3. PRESET MANAGEMENT
//     GESTIÓN DE PRESETS
// ============================================================================

int AndesJXAudioProcessor::getNumPrograms()
{
    return int(presets.size());
}

int AndesJXAudioProcessor::getCurrentProgram()
{
    return currentProgram;
}


// EN: Compares the current APVTS values against the parameters stored
//     in the active preset. Returns true if every parameter matches
//     within a small float tolerance.
//     The comparison uses convertFrom0to1() because APVTS internally
//     stores values in [0, 1] but the Preset stores them in their
//     real-world units. A tolerance of 0.001 absorbs the rounding
//     noise of those conversions.
// ES: Compara los valores actuales del APVTS contra los parámetros
//     guardados en el preset activo. Devuelve true si cada parámetro
//     coincide dentro de una pequeña tolerancia de float.
//     La comparación usa convertFrom0to1() porque el APVTS guarda
//     internamente en [0, 1] pero el Preset guarda en unidades reales.
//     Una tolerancia de 0.001 absorbe el ruido de redondeo de esas
//     conversiones.
bool AndesJXAudioProcessor::currentStateMatchesProgram() const
{
    if (currentProgram < 0 || currentProgram >= static_cast<int>(presets.size()))
        return false;

    const Preset& preset = presets[currentProgram];

    // EN: Parameter pointer table in the canonical order (must mirror
    //     Preset's constructor order; see Preset.h).
    // ES: Tabla de punteros a parámetros en el orden canónico (debe
    //     reflejar el orden del constructor de Preset; ver Preset.h).
    const juce::RangedAudioParameter* params[NUM_PARAMS] =
    {
        osc1WaveParam,    osc2WaveParam,    oscMixParam,        oscTuneParam,
        oscFineParam,     glideModeParam,   glideRateParam,     glideBendParam,
        filterTypeParam,  filterFreqParam,  filterResoParam,    filterEnvParam,
        filterLFOParam,   filterVelocityParam, filterKeytrackParam, filterKeycenterParam,
        filterAttackParam, filterDecayParam, filterSustainParam, filterReleaseParam,
        envAttackParam,   envDecayParam,    envSustainParam,    envReleaseParam,
        lfoRateParam,     vibratoParam,     noiseParam,         octaveParam,
        tuningParam,      outputLevelParam, polyModeParam,      stereoWidthParam
    };

    for (int i = 0; i < NUM_PARAMS; ++i)
    {
        const float currentValue = params[i]->convertFrom0to1(params[i]->getValue());
        const float presetValue = preset.param[i];

        // EN: Tolerance for float equality. 0.001 is well below the
        //     resolution of any parameter range used in AndesJX.
        // ES: Tolerancia para igualdad de float. 0.001 está muy por
        //     debajo de la resolución de cualquier rango de parámetro
        //     usado en AndesJX.
        if (std::abs(currentValue - presetValue) > 0.001f)
            return false;
    }

    return true;
}


// EN: Loads preset `index` into the APVTS. The loadingPreset flag
//     temporarily silences the "became custom" detection inside the
//     valueTreePropertyChanged listener, so that programmatic updates
//     do not trigger false positives. After loading, reset() is called
//     to flush the synth state and a change message is broadcast so the
//     editor can refresh.
// ES: Carga el preset `index` en el APVTS. La bandera loadingPreset
//     silencia temporalmente la detección de "se volvió custom" del
//     listener valueTreePropertyChanged, para que las actualizaciones
//     programáticas no disparen falsos positivos. Tras la carga, se
//     llama a reset() para limpiar el estado del synth y se broadcastea
//     un mensaje para que el editor se refresque.
void AndesJXAudioProcessor::setCurrentProgram(int index)
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return;

    loadingPreset = true;

    currentProgram = index;
    isCustomPreset = false;

    juce::RangedAudioParameter* params[NUM_PARAMS] =
    {
        osc1WaveParam,    osc2WaveParam,    oscMixParam,        oscTuneParam,
        oscFineParam,     glideModeParam,   glideRateParam,     glideBendParam,
        filterTypeParam,  filterFreqParam,  filterResoParam,    filterEnvParam,
        filterLFOParam,   filterVelocityParam, filterKeytrackParam, filterKeycenterParam,
        filterAttackParam, filterDecayParam, filterSustainParam, filterReleaseParam,
        envAttackParam,   envDecayParam,    envSustainParam,    envReleaseParam,
        lfoRateParam,     vibratoParam,     noiseParam,         octaveParam,
        tuningParam,      outputLevelParam, polyModeParam,      stereoWidthParam
    };

    const Preset& preset = presets[index];

    // EN: setValueNotifyingHost makes the host (DAW) aware of the change
    //     so it can update its own display and any parameter automation.
    //     convertTo0to1() maps from real-world units to the [0, 1] range
    //     that APVTS uses internally.
    // ES: setValueNotifyingHost informa al host (DAW) del cambio para
    //     que actualice su display y cualquier automatización del
    //     parámetro. convertTo0to1() mapea de unidades reales al rango
    //     [0, 1] que el APVTS usa internamente.
    for (int i = 0; i < NUM_PARAMS; ++i)
        params[i]->setValueNotifyingHost(params[i]->convertTo0to1(preset.param[i]));

    loadingPreset = false;

    reset();
    sendChangeMessage();
}


// EN: Returns the display name of the preset at `index`, or "Custom"
//     if the index is out of range. The host uses this to populate the
//     preset menu it shows in its plugin window.
// ES: Devuelve el nombre visible del preset en `index`, o "Custom" si
//     el índice está fuera de rango. El host lo usa para poblar el
//     menú de presets que muestra en su ventana de plugin.
const juce::String AndesJXAudioProcessor::getProgramName(int index)
{
    if (index >= 0 && index < static_cast<int>(presets.size()))
        return presets[index].name;

    return "Custom";
}

// EN: AndesJX presets are read-only factory bank entries; renaming is
//     not supported. The function exists only to satisfy the JUCE
//     AudioProcessor interface.
// ES: Los presets de AndesJX son entradas de fábrica de solo lectura;
//     no se admite renombrarlos. La función existe solo para cumplir la
//     interfaz AudioProcessor de JUCE.
void AndesJXAudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/)
{
}

// ============================================================================
//  4. AUDIO LIFECYCLE
//     CICLO DE VIDA DE AUDIO
// ============================================================================

// EN: Called by the host once before playback starts. Three jobs:
//       (1) Build the juce::dsp oversampling chain.
//       (2) Allocate Synth resources at the OVERSAMPLED rate (sampleRate
//           * 2 and samplesPerBlock * 2, because the synth runs inside
//           the upsampled domain).
//       (3) Force a parameter pull on the next audio block so the synth
//           starts in sync with the current APVTS values.
// ES: El host lo llama una vez antes de iniciar la reproducción. Tres
//     trabajos:
//       (1) Construir la cadena de oversampling de juce::dsp.
//       (2) Asignar recursos del Synth a la sample rate SOBREMUESTREADA
//           (sampleRate * 2 y samplesPerBlock * 2, porque el synth
//           corre dentro del dominio sobremuestreado).
//       (3) Forzar una lectura de parámetros en el siguiente bloque de
//           audio para que el synth arranque sincronizado con los
//           valores actuales del APVTS.
void AndesJXAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    // EN: Oversampling configuration. The "1" passed as the second
    //     argument is the oversampling factor exponent: 2^1 = 2x.
    //     The IIR polyphase half-band filter is the cheapest of the
    //     supported anti-alias filters and is well suited for a real-time
    //     synthesizer (low latency, low CPU).
    //     2x is enough to push the alias products of the synth's
    //     non-linearities (Moog ladder tanh, soft clip) above the
    //     audible range.
    // ES: Configuración del oversampling. El "1" pasado como segundo
    //     argumento es el exponente del factor: 2^1 = 2x.
    //     El filtro IIR polyphase half-band es el más barato de los
    //     filtros antialiasing disponibles y se adapta bien a un
    //     sintetizador en tiempo real (baja latencia, bajo CPU).
    //     2x es suficiente para empujar los productos de aliasing de
    //     las no linealidades del synth (tanh del Moog ladder, soft
    //     clip) por encima del rango audible.
    oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
        spec.numChannels,
        1,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);

    oversampling->initProcessing(samplesPerBlock);

    // EN: Synth runs inside the oversampled domain, so it must be
    //     prepared with the doubled sample rate and block size. The
    //     factor 2 here MUST match the 2^1 oversampling factor above.
    // ES: El Synth corre dentro del dominio sobremuestreado, así que
    //     debe prepararse con sample rate y tamaño de bloque
    //     duplicados. El factor 2 aquí DEBE coincidir con el factor
    //     2^1 de oversampling de arriba.
    synth.allocateResources(sampleRate * 2, samplesPerBlock * 2);

    parametersChanged.store(true);
    reset();
}


// EN: Called by the host when audio is stopped or the plugin is being
//     unloaded. AndesJX has no allocated resources to release explicitly
//     (everything is RAII-managed), so the body is empty.
// ES: El host lo llama al detener el audio o al descargar el plugin.
//     AndesJX no tiene recursos que liberar explícitamente (todo es
//     gestionado por RAII), así que el cuerpo queda vacío.
void AndesJXAudioProcessor::releaseResources()
{
}


// EN: Hard reset. Forwarded directly to the synth, which clears all
//     voices, the LFO, the key stack and the smoothed cutoff state.
// ES: Reset duro. Se reenvía directo al synth, que limpia todas las
//     voces, el LFO, el key stack y el estado suavizado del cutoff.
void AndesJXAudioProcessor::reset()
{
    synth.reset();
}


#ifndef JucePlugin_PreferredChannelConfigurations
// EN: Standard JUCE bus-layout validator. AndesJX accepts mono or
//     stereo output. The input check (when not a synth) ensures input
//     and output layouts match, which is required by some hosts
//     (notably older GarageBand versions).
// ES: Validador estándar de layout de buses de JUCE. AndesJX acepta
//     salida mono o estéreo. El chequeo de entrada (cuando no es synth)
//     asegura que entrada y salida coincidan, requerido por algunos
//     hosts (en particular versiones antiguas de GarageBand).
bool AndesJXAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif


// ============================================================================
//  5. AUDIO PROCESSING
//     PROCESAMIENTO DE AUDIO
// ============================================================================

// EN: The hot path of the plugin. Called once per audio block (~few ms)
//     by the host. The flow is:
//       (1) Disable denormals for performance.
//       (2) Clear any output channels that have no input data.
//       (3) Pull APVTS values into Synth (via update()) if any
//           parameter has changed since the last block.
//       (4) Capture relevant MIDI CC values into the std::atomic fields
//           so they can be read from the audio thread without locks.
//       (5) Build a CCState snapshot and hand it to Synth.
//       (6) Upsample the buffer 2x, render the synth into the
//           oversampled buffer (split by MIDI events for sample-accurate
//           timing), then downsample back to the host rate.
// ES: La ruta caliente del plugin. El host la llama una vez por bloque
//     de audio (~pocos ms). El flujo es:
//       (1) Desactivar denormales por rendimiento.
//       (2) Limpiar cualquier canal de salida sin datos de entrada.
//       (3) Leer valores del APVTS hacia Synth (vía update()) si algún
//           parámetro ha cambiado desde el bloque anterior.
//       (4) Capturar los CC MIDI relevantes en los campos std::atomic
//           para que el hilo de audio pueda leerlos sin locks.
//       (5) Construir un snapshot CCState y entregárselo al Synth.
//       (6) Sobremuestrear el buffer 2x, renderizar el synth en el
//           buffer sobremuestreado (dividido por eventos MIDI para
//           timing preciso por muestra), y volver a la tasa del host.
void AndesJXAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // EN: Clear any output channels that don't contain input data.
    //     Standard JUCE boilerplate: prevents leftover garbage in the
    //     output of channels the host did not feed.
    // ES: Limpiar cualquier canal de salida sin datos de entrada.
    //     Boilerplate estándar de JUCE: evita basura residual en la
    //     salida de canales que el host no alimentó.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // EN: Atomic compare-and-swap: if parametersChanged was true, set
    //     it to false and run update(). The strong variant guarantees
    //     no spurious failures, and the whole operation is lockless.
    // ES: Compare-and-swap atómico: si parametersChanged era true, lo
    //     pone en false y ejecuta update(). La variante fuerte
    //     garantiza que no haya fallos espurios, y toda la operación
    //     es lockless.
    bool expected = true;
    if (parametersChanged.compare_exchange_strong(expected, false))
        update();


    // ------------------------------------------------------------------------
    //  MIDI CC capture / Captura de CCs MIDI
    // ------------------------------------------------------------------------

    // EN: Walk the MIDI buffer once and store every relevant CC into
    //     its corresponding std::atomic field. Using memory_order_relaxed
    //     because the audio thread reads each value independently and
    //     does not need ordering guarantees across them.
    // ES: Recorrer el buffer MIDI una vez y almacenar cada CC relevante
    //     en su campo std::atomic correspondiente. Se usa
    //     memory_order_relaxed porque el hilo de audio lee cada valor
    //     de forma independiente y no necesita garantías de orden
    //     entre ellos.
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();

        if (msg.isController())
        {
            const int   cc = msg.getControllerNumber();
            const float v = msg.getControllerValue() / 127.0f;

            switch (cc)
            {
            case 1:  ccModWheel.store(v, std::memory_order_relaxed); break;  // EN/ES: ModWheel
            case 11: ccExpression.store(v, std::memory_order_relaxed); break;  // EN/ES: Expression
            case 74: ccBrightness.store(v, std::memory_order_relaxed); break;  // EN/ES: Brightness
            case 71: ccResonance.store(v, std::memory_order_relaxed); break;  // EN/ES: Resonance
            case 73: ccAttack.store(v, std::memory_order_relaxed); break;  // EN/ES: Attack
            case 72: ccRelease.store(v, std::memory_order_relaxed); break;  // EN/ES: Release
            case 64: ccSustainDown.store(v >= 0.5f, std::memory_order_relaxed); break;  // EN/ES: Sustain
            default: break;
            }
        }
    }

    // EN: Build the CCState snapshot from the atomics and hand it to
    //     Synth. From this point on Synth has a consistent view of the
    //     CC values for the current block.
    // ES: Construir el snapshot CCState desde los atomics y entregarlo
    //     al Synth. A partir de aquí Synth tiene una vista consistente
    //     de los valores de CC para el bloque actual.
    Synth::CCState s;
    s.modWheel = ccModWheel.load(std::memory_order_relaxed);
    s.expression = ccExpression.load(std::memory_order_relaxed);
    s.brightness = ccBrightness.load(std::memory_order_relaxed);
    s.resonance = ccResonance.load(std::memory_order_relaxed);
    s.attack = ccAttack.load(std::memory_order_relaxed);
    s.release = ccRelease.load(std::memory_order_relaxed);
    s.sustain = ccSustainDown.load(std::memory_order_relaxed);

    synth.setCCState(s);


    // ------------------------------------------------------------------------
    //  Oversampled rendering / Renderizado sobremuestreado
    // ------------------------------------------------------------------------

    // EN: 2x oversampling pipeline:
    //       (1) processSamplesUp:   buffer (host rate) -> osBlock (2x).
    //       (2) splitBufferByEventsOptimized: render the synth into
    //           osBlock, splitting at every MIDI event so notes align
    //           on the correct sample.
    //       (3) processSamplesDown: osBlock (2x) -> buffer (host rate),
    //           applying the half-band filter to remove anti-alias
    //           images.
    //     The legacy alternative splitBufferByEvents() (no oversampling)
    //     is preserved further below for reference.
    // ES: Pipeline de oversampling 2x:
    //       (1) processSamplesUp:   buffer (tasa host) -> osBlock (2x).
    //       (2) splitBufferByEventsOptimized: renderizar el synth en
    //           osBlock, dividiendo en cada evento MIDI para que las
    //           notas caigan en la muestra correcta.
    //       (3) processSamplesDown: osBlock (2x) -> buffer (tasa host),
    //           aplicando el filtro half-band para eliminar las
    //           imágenes de aliasing.
    //     La alternativa heredada splitBufferByEvents() (sin
    //     oversampling) se conserva más abajo como referencia.
    juce::dsp::AudioBlock<float> block(buffer);
    auto osBlock = oversampling->processSamplesUp(block);

    splitBufferByEventsOptimized(osBlock, midiMessages);

    oversampling->processSamplesDown(block);
}

// EN: Sample-accurate render driver. Walks the MIDI buffer in order:
//     for every event, renders the synth up to the event's sample
//     position, then dispatches the event, then renders the rest.
//     This guarantees that note-ons land on the correct sample even
//     when many events arrive in the same audio block.
//
//     The osFactor of 2 multiplies every MIDI sample position because
//     this function operates inside the OVERSAMPLED buffer (sampleRate
//     * 2). One MIDI sample at the host rate equals two samples in the
//     oversampled domain.
//
// ES: Driver de render con precisión por muestra. Recorre el buffer
//     MIDI en orden: por cada evento, renderiza el synth hasta la
//     posición del evento, luego despacha el evento, luego renderiza el
//     resto. Esto garantiza que los note-on caigan en la muestra
//     correcta incluso cuando llegan muchos eventos en un mismo bloque.
//
//     El osFactor de 2 multiplica cada posición de muestra MIDI porque
//     esta función opera dentro del buffer SOBREMUESTREADO (sampleRate
//     * 2). Una muestra MIDI a la tasa del host equivale a dos muestras
//     en el dominio sobremuestreado.
void AndesJXAudioProcessor::splitBufferByEventsOptimized(juce::dsp::AudioBlock<float>& block,
    juce::MidiBuffer& midiMessages)
{
    constexpr int osFactor = 2;
    int bufferOffset = 0;
    const int totalSamples = static_cast<int>(block.getNumSamples());

    for (const auto metadata : midiMessages)
    {
        const int eventPosOS = metadata.samplePosition * osFactor;

        // EN: Render the audio segment that lies BEFORE the event.
        //     If two events share the same sample position, the second
        //     iteration will compute samplesThisSegment == 0 and skip
        //     this block, going straight to dispatching the event.
        // ES: Renderizar el segmento de audio ANTES del evento.
        //     Si dos eventos comparten la misma posición de muestra, la
        //     segunda iteración calculará samplesThisSegment == 0 y se
        //     saltará este bloque, despachando directamente el evento.
        int samplesThisSegment = eventPosOS - bufferOffset;
        if (samplesThisSegment > 0)
        {
            float* outputBuffers[2] = { nullptr, nullptr };
            outputBuffers[0] = block.getChannelPointer(0) + bufferOffset;
            if (block.getNumChannels() > 1)
                outputBuffers[1] = block.getChannelPointer(1) + bufferOffset;

            synth.render(outputBuffers, samplesThisSegment);
            bufferOffset += samplesThisSegment;
        }

        // EN: Dispatch the MIDI event. Long messages (sysex, > 3 bytes)
        //     are ignored by this synth.
        // ES: Despachar el evento MIDI. Los mensajes largos (sysex,
        //     > 3 bytes) los ignora este synth.
        if (metadata.numBytes <= 3)
        {
            uint8_t data1 = (metadata.numBytes >= 2) ? metadata.data[1] : 0;
            uint8_t data2 = (metadata.numBytes == 3) ? metadata.data[2] : 0;
            handleMIDI(metadata.data[0], data1, data2);
        }
    }

    // EN: Render the tail after the last MIDI event. If there were no
    //     events at all, this single call renders the entire block.
    // ES: Renderizar la cola después del último evento MIDI. Si no hubo
    //     eventos, esta única llamada renderiza el bloque entero.
    int samplesLastSegment = totalSamples - bufferOffset;
    if (samplesLastSegment > 0)
    {
        float* outputBuffers[2] = { nullptr, nullptr };
        outputBuffers[0] = block.getChannelPointer(0) + bufferOffset;
        if (block.getNumChannels() > 1)
            outputBuffers[1] = block.getChannelPointer(1) + bufferOffset;

        synth.render(outputBuffers, samplesLastSegment);
    }

    midiMessages.clear();
}


// ============================================================================
//  6. PARAMETER PULL (APVTS -> SYNTH)
//     LECTURA DE PARÁMETROS (APVTS -> SYNTH)
// ============================================================================

// EN: Translates the APVTS parameter values into the internal fields
//     that Synth consumes. Called from processBlock() whenever the
//     parametersChanged flag is set, i.e. once per block at most when
//     a parameter has been touched.
//
//     This function is the SINGLE place where user-facing units (%, dB,
//     semitones, Hz curves) become the internal coefficients used by
//     the synth (envelope rates as IIR multipliers, cutoff as Hz,
//     detune as a frequency multiplier, etc.). Adding a new parameter
//     means adding (a) its declaration in createParameterLayout, and
//     (b) its conversion line here.
// ES: Traduce los valores de los parámetros APVTS a los campos internos
//     que consume Synth. Se llama desde processBlock() cuando la
//     bandera parametersChanged está activa, es decir, una vez por
//     bloque a lo sumo cuando se ha tocado un parámetro.
//
//     Esta función es el ÚNICO lugar donde las unidades visibles al
//     usuario (%, dB, semitonos, curvas de Hz) se convierten en los
//     coeficientes internos que usa el synth (velocidades de
//     envolvente como multiplicadores IIR, cutoff como Hz, detune como
//     multiplicador de frecuencia, etc.). Añadir un parámetro nuevo
//     implica agregar (a) su declaración en createParameterLayout y
//     (b) su línea de conversión aquí.
void AndesJXAudioProcessor::update()
{
    // ------------------------------------------------------------------------
    //  Oscillator waveforms / Formas de onda de los osciladores
    // ------------------------------------------------------------------------

    const int w1 = osc1WaveParam->getIndex();
    const int w2 = osc2WaveParam->getIndex();

    // EN: Map the APVTS choice index to the WaveType enum. The lambda
    //     decouples the GUI ordering from the enum ordering, so reordering
    //     the dropdown does not change the audio behavior.
    // ES: Mapear el índice de choice del APVTS al enum WaveType. La
    //     lambda desacopla el orden de la GUI del orden del enum, así
    //     reordenar el dropdown no cambia el comportamiento de audio.
    auto toWave = [](int idx) -> WaveType
        {
            switch (idx)
            {
            case 0: return WaveType::Sine;
            case 1: return WaveType::Saw;
            case 2: return WaveType::Square;
            case 3: return WaveType::Triangle;
            case 4: return WaveType::SquarePWM;
            default:return WaveType::Saw;
            }
        };

    synth.setOsc1Wave(toWave(w1));
    synth.setOsc2Wave(toWave(w2));


    // ------------------------------------------------------------------------
    //  Filter type / Tipo de filtro
    // ------------------------------------------------------------------------

    const int filterIndex = filterTypeParam->getIndex();

    auto toFilterType = [](int idx) -> Synth::FilterType
        {
            switch (idx)
            {
            case 0:  return Synth::FilterType::SVF;
            case 1:  return Synth::FilterType::Moog;
            default: return Synth::FilterType::SVF;
            }
        };

    synth.setFilterType(toFilterType(filterIndex));


    // ------------------------------------------------------------------------
    //  Sample rate context / Contexto de sample rate
    // ------------------------------------------------------------------------

    // EN: All envelope coefficients below are computed against the
    //     OVERSAMPLED rate (host * 2) because Synth runs in that
    //     domain. Keeping inverseSampleRate = 1 / sampleRate avoids
    //     repeated divisions inside the formulas.
    // ES: Todos los coeficientes de envolvente de abajo se calculan
    //     contra la sample rate SOBREMUESTREADA (host * 2) porque
    //     Synth corre en ese dominio. Tener inverseSampleRate = 1 /
    //     sampleRate evita divisiones repetidas dentro de las fórmulas.
    float sampleRate = float(getSampleRate()) * 2.0f;
    float inverseSampleRate = 1.0f / sampleRate;


    // ------------------------------------------------------------------------
    //  Amplitude envelope / Envolvente de amplitud
    // ------------------------------------------------------------------------

    // EN: Convert each ADSR knob (in [0, 100]) into the per-sample IIR
    //     coefficient consumed by Envelope. The formula
    //         coef = exp( -1/fs * exp(5.5 - 0.075 * knob) )
    //     produces a coefficient that maps a knob of 0 to a fast curve
    //     and a knob of 100 to a slow curve, with smooth perceptual
    //     spacing (the inner exp is the time constant in samples).
    //     Sustain is a level in [0, 1], not a time, so it is just a
    //     percent-to-fraction conversion.
    // ES: Convertir cada knob ADSR (en [0, 100]) al coeficiente IIR
    //     por muestra que consume Envelope. La fórmula
    //         coef = exp( -1/fs * exp(5.5 - 0.075 * knob) )
    //     produce un coeficiente que mapea un knob 0 a una curva
    //     rápida y un knob 100 a una curva lenta, con espaciado
    //     perceptual suave (el exp interno es la constante de tiempo
    //     en muestras).
    //     Sustain es un nivel en [0, 1], no un tiempo, así que es solo
    //     una conversión de porcentaje a fracción.
    synth.envAttack = std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * envAttackParam->get()));
    synth.envDecay = std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * envDecayParam->get()));
    synth.envSustain = envSustainParam->get() / 100.0f;

    // EN: Polyphony selector. Mono uses voice 0 only; Poly uses the
    //     full pool (MAX_VOICES).
    // ES: Selector de polifonía. Mono usa solo la voz 0; Poly usa todo
    //     el pool (MAX_VOICES).
    synth.numVoices = (polyModeParam->getIndex() == 0) ? 1 : MAX_VOICES;

    // EN: Release uses the same exponential mapping but with a safety
    //     floor of 1.0: a release knob below 1 would produce a near-
    //     instant cutoff that sounds clicky. Clamping at 1 keeps the
    //     fastest release musically usable.
    // ES: El release usa el mismo mapeo exponencial pero con un piso de
    //     seguridad de 1.0: un knob de release por debajo de 1
    //     produciría un corte casi instantáneo que suena clicky.
    //     Limitarlo en 1 mantiene el release más rápido musicalmente
    //     usable.
    float envRelease = envReleaseParam->get();
    float r = juce::jmax(1.0f, envRelease);
    synth.envRelease = std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * r));


    // ------------------------------------------------------------------------
    //  Oscillator mix and noise / Mezcla de osciladores y ruido
    // ------------------------------------------------------------------------

    synth.oscMix = oscMixParam->get() / 100.0f;

    // EN: Quadratic curve on the noise knob gives finer control at low
    //     levels (where most of the musically useful range lives).
    //     The 0.06 final scale is a perceptual headroom factor: the
    //     synth would clip aggressively if noise reached unity.
    // ES: Curva cuadrática en el knob de ruido da control más fino en
    //     niveles bajos (donde vive la mayor parte del rango
    //     musicalmente útil). El factor 0.06 final es un margen
    //     perceptual: el synth saturaría agresivamente si el ruido
    //     llegara a la unidad.
    float noiseMix = noiseParam->get() / 100.0f;
    noiseMix *= noiseMix;
    synth.noiseMix = noiseMix * 0.06f;

    // EN: Auto-volume trim. As more oscillator mix and more noise
    //     enter the signal, the perceived loudness rises; this trim
    //     compensates by reducing the master gain. Tuned empirically.
    // ES: Trim automático de volumen. A medida que entra más mezcla
    //     de osciladores y más ruido, la sonoridad percibida sube;
    //     este trim compensa reduciendo la ganancia maestra. Ajustado
    //     empíricamente.
    float trim = 0.30f * (3.2f - synth.oscMix - 25.0f * synth.noiseMix);
    synth.volumeTrim = juce::jlimit(0.0f, 1.2f, trim);


    // ------------------------------------------------------------------------
    //  Tuning and detune / Afinación y detune
    // ------------------------------------------------------------------------

    // EN: oscTune is in semitones, oscFine is in cents. Together they
    //     become a frequency multiplier 2^(semis/12) applied to osc2
    //     relative to osc1.
    // ES: oscTune está en semitonos, oscFine en cents. Juntos se
    //     convierten en un multiplicador de frecuencia 2^(semis/12)
    //     aplicado a osc2 respecto a osc1.
    float semi = oscTuneParam->get();
    float cent = oscFineParam->get();
    synth.detune = std::exp2((semi + 0.01f * cent) / 12.0f);

    // EN: Global tune. Octave is integer (-2..+2), tuning is in cents
    //     (-100..+100). The result is a semitone offset added to every
    //     note before the MIDI -> Hz conversion (see Synth::calcBaseFreq).
    // ES: Afinación global. Octave es entero (-2..+2), tuning está en
    //     cents (-100..+100). El resultado es un offset en semitonos
    //     que se suma a cada nota antes de la conversión MIDI -> Hz
    //     (ver Synth::calcBaseFreq).
    float octave = octaveParam->get();
    float tuning = tuningParam->get();
    synth.tune = octave * 12.0f + tuning / 100.0f;


    // ------------------------------------------------------------------------
    //  Stereo and output level / Estéreo y nivel de salida
    // ------------------------------------------------------------------------

    synth.stereoWidth = stereoWidthParam->get() / 100.0f;

    // EN: Output level converted from dB to linear gain and pushed into
    //     the smoother. The smoother itself was configured for a 20 ms
    //     ramp in Synth::allocateResources().
    // ES: Nivel de salida convertido de dB a ganancia lineal y
    //     enviado al smoother. El smoother en sí se configuró para una
    //     rampa de 20 ms en Synth::allocateResources().
    float gain = juce::Decibels::decibelsToGain(outputLevelParam->get());
    synth.outputLevelSmoother.setTargetValue(gain);


    // ------------------------------------------------------------------------
    //  Velocity / Velocity
    // ------------------------------------------------------------------------

    const float filterVelocity = filterVelocityParam->get();

    // EN: Amplitude velocity is fixed at a moderate sensitivity and is
    //     not exposed as a knob. The "Velocity" knob in the UI only
    //     affects the FILTER envelope depth, not the amplitude. This is
    //     a deliberate separation: the user can play loud whispers
    //     (high velocity, low filter) or soft brightness (low velocity,
    //     high filter) without coupling.
    // ES: La velocity de amplitud está fija en una sensibilidad
    //     moderada y no se expone como knob. El knob "Velocity" de la
    //     UI solo afecta la profundidad de la envolvente de FILTRO, no
    //     la amplitud. Es una separación deliberada: el usuario puede
    //     tocar susurros fuertes (velocity alta, filtro bajo) o brillo
    //     suave (velocity baja, filtro alto) sin acoplamiento.
    synth.velocitySensitivity = 0.75f;
    synth.ignoreVelocity = false;

    // EN: Filter velocity. A knob below -90 turns it off entirely (the
    //     UI shows "OFF"). Otherwise the absolute value scales the
    //     filter envelope depth modulation in [0, 1].
    // ES: Filter velocity. Un knob por debajo de -90 lo apaga del todo
    //     (la UI muestra "OFF"). En otro caso, el valor absoluto
    //     escala la modulación de profundidad de la envolvente de
    //     filtro en [0, 1].
    if (filterVelocity < -90.0f)
        synth.filterVelocityAmount = 0.0f;
    else
        synth.filterVelocityAmount = juce::jlimit(0.0f, 1.0f, std::abs(filterVelocity) / 100.0f);


    // ------------------------------------------------------------------------
    //  LFO and glide / LFO y glide
    // ------------------------------------------------------------------------

    // EN: LFO rate is exposed in [0, 1]. The exponential mapping
    //     exp(7x - 4) yields ~0.018 Hz at x=0 and ~24 Hz at x=1, which
    //     covers the musically useful sub-audio range with smooth
    //     perceptual spacing.
    // ES: La rate del LFO se expone en [0, 1]. El mapeo exponencial
    //     exp(7x - 4) da ~0.018 Hz en x=0 y ~24 Hz en x=1, cubriendo
    //     el rango sub-audio musicalmente útil con espaciado
    //     perceptual suave.
    const float lfoNorm = lfoRateParam->get();
    const float lfoHz = std::exp(7.0f * lfoNorm - 4.0f);
    synth.setLfoRateHz(lfoHz);

    // EN: Glide. The "inverse update rate" 32/sampleRate accounts for
    //     the fact that Synth advances the glide once per LFO_MAX
    //     samples (32), not once per sample. A knob below 2 disables
    //     glide entirely (rate = 1.0 = instant).
    // ES: Glide. La "tasa de update inversa" 32/sampleRate refleja que
    //     Synth avanza el glide una vez cada LFO_MAX muestras (32), no
    //     una vez por muestra. Un knob por debajo de 2 desactiva el
    //     glide del todo (rate = 1.0 = instantáneo).
    const float inverseUpdateRate = 32.0f / sampleRate;
    synth.glideMode = glideModeParam->getIndex();
    float gr = glideRateParam->get();
    if (gr < 2.0f)
        synth.glideRate = 1.0f;
    else
        synth.glideRate = 1.0f - std::exp(-inverseUpdateRate * std::exp(6.0f - 0.07f * gr));

    synth.glideBend = glideBendParam->get();


    // ------------------------------------------------------------------------
    //  Vibrato / PWM bipolar control / Control bipolar Vibrato / PWM
    // ------------------------------------------------------------------------

    // EN: One knob, two functions. The "Vibrato" parameter is bipolar
    //     in [-100, +100]:
    //       - Positive values activate vibrato (LFO -> pitch).
    //       - Negative values activate PWM (LFO -> osc2 pulse width).
    //     They are mutually exclusive: the inactive side is forced to 0.
    //     The quadratic curve (value * value) gives finer control at
    //     small movements around zero.
    // ES: Un knob, dos funciones. El parámetro "Vibrato" es bipolar
    //     en [-100, +100]:
    //       - Valores positivos activan vibrato (LFO -> pitch).
    //       - Valores negativos activan PWM (LFO -> ancho de pulso de osc2).
    //     Son mutuamente excluyentes: el lado inactivo se fuerza a 0.
    //     La curva cuadrática (value * value) da control más fino en
    //     movimientos pequeños alrededor del cero.
    float vibratoValue = vibratoParam->get();

    synth.lfoDepthSemis = vibratoValue * vibratoValue * 0.0002f;

    if (vibratoValue < 0.0f)
    {
        synth.lfoDepthSemis = 0.0f;
        synth.pwmDepth = std::abs(vibratoValue) * 0.01f;
    }
    else
    {
        synth.pwmDepth = 0.0f;
    }


    // ------------------------------------------------------------------------
    //  Filter cutoff and resonance / Cutoff y resonancia del filtro
    // ------------------------------------------------------------------------

    // EN: LFO -> filter depth in semitones. The quadratic curve plus
    //     the 2.5 ceiling caps the LFO modulation at 2.5 semitones
    //     even at full knob, which is enough to be musical without
    //     becoming dissonant.
    // ES: Profundidad LFO -> filtro en semitonos. La curva cuadrática
    //     más el techo de 2.5 limita la modulación del LFO a 2.5
    //     semitonos incluso a knob completo, suficiente para ser
    //     musical sin volverse disonante.
    float filterLFO = filterLFOParam->get() / 100.0f;
    synth.filterLFODepthSemis = 2.5f * filterLFO * filterLFO;

    // EN: Filter cutoff. The knob is exposed as a percentage [0, 100],
    //     but maps logarithmically to [80 Hz, 20 kHz] so equal knob
    //     movement produces equal pitch movement (perceptually linear).
    // ES: Cutoff del filtro. El knob se expone como porcentaje
    //     [0, 100], pero se mapea logarítmicamente a [80 Hz, 20 kHz]
    //     para que un mismo movimiento de knob produzca el mismo
    //     movimiento de altura (linealidad perceptual).
    const float x = juce::jlimit(0.0f, 1.0f, filterFreqParam->get() / 100.0f);
    const float minHz = 80.0f;
    const float maxHz = 20000.0f;
    synth.filterCutoff = minHz * std::exp(std::log(maxHz / minHz) * x);

    // EN: Quadratic curve on resonance gives finer control at low
    //     values, where the filter sounds "open"; high values quickly
    //     reach self-oscillation, so coarser steps there are fine.
    // ES: Curva cuadrática en la resonancia da control más fino en
    //     valores bajos, donde el filtro suena "abierto"; los valores
    //     altos llegan rápido a la auto-oscilación, así que pasos más
    //     gruesos ahí son aceptables.
    float filterResoPercent = filterResoParam->get();
    float x1 = juce::jlimit(0.0f, 1.0f, filterResoPercent / 100.0f);
    synth.filterResonance = x1 * x1;


    // ------------------------------------------------------------------------
    //  Key tracking and filter envelope / Key tracking y envolvente de filtro
    // ------------------------------------------------------------------------

    // EN: Key tracking knob in [0, 200] %. Internally the synth uses
    //     [0, 2] (0 = no tracking, 1 = full one-octave-per-octave,
    //     2 = double tracking). The keycenter is a MIDI note number,
    //     stored as float in the APVTS for uniform UI handling.
    // ES: Knob de key tracking en [0, 200] %. Internamente el synth
    //     usa [0, 2] (0 = sin tracking, 1 = una octava por octava,
    //     2 = tracking doble). El keycenter es un número de nota MIDI,
    //     guardado como float en el APVTS para manejo uniforme de UI.
    synth.filterKeytrackAmount = filterKeytrackParam->get() / 100.0f;
    synth.filterKeycenterNote = (int)filterKeycenterParam->get();

    // EN: Filter envelope amount in semitones. The knob is bipolar in
    //     [-100, +100] %, mapped to [-48, +48] semitones (4 octaves
    //     each direction), enough for both subtle filter sweeps and
    //     dramatic "wow" effects.
    // ES: Cantidad de la envolvente de filtro en semitonos. El knob
    //     es bipolar en [-100, +100] %, mapeado a [-48, +48] semitonos
    //     (4 octavas en cada dirección), suficiente tanto para
    //     barridos sutiles como para efectos "wow" dramáticos.
    const float envPct = juce::jlimit(-100.0f, 100.0f, filterEnvParam->get());
    synth.filterEnvAmountSemis = 48.0f * (envPct / 100.0f);

    // EN: Filter envelope coefficients (same exponential mapping as the
    //     amplitude envelope, with the same release safety floor).
    // ES: Coeficientes de la envolvente de filtro (mismo mapeo
    //     exponencial que la envolvente de amplitud, con el mismo
    //     piso de seguridad para release).
    synth.filterEnvAttack = std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * filterAttackParam->get()));
    synth.filterEnvDecay = std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * filterDecayParam->get()));
    synth.filterEnvSustain = filterSustainParam->get() / 100.0f;

    float fr = filterReleaseParam->get();
    float frSafe = juce::jmax(1.0f, fr);
    synth.filterEnvRelease = std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * frSafe));
}

// ----------------------------------------------------------------------------
//  splitBufferByEvents (legacy / referencia)
// ----------------------------------------------------------------------------

// EN: Earlier render driver, kept here as a teaching reference. It does
//     the same sample-accurate splitting as splitBufferByEventsOptimized
//     but operates directly on the host's juce::AudioBuffer (no
//     oversampling). The processBlock above no longer calls it; the
//     active path is the Optimized variant inside the oversampled
//     domain.
//
//     Why preserved:
//       (1) It shows the simpler topology before the oversampling
//           pipeline was introduced; useful as a starting point for
//           students learning sample-accurate event handling.
//       (2) It can be re-enabled (in processBlock) when debugging
//           without the upsample / downsample stages, helping to
//           isolate whether a bug lives in the synth or in the
//           oversampler.
//
//     Note that the osFactor of 2 is preserved here too because every
//     midi event's samplePosition is a position at the host rate, but
//     the buffer math expects positions in the rendered domain. If you
//     ever switch this function to non-oversampled use, set osFactor
//     to 1.
//
// ES: Driver de render anterior, conservado aquí como referencia
//     didáctica. Hace la misma división con precisión por muestra que
//     splitBufferByEventsOptimized pero opera directamente sobre el
//     juce::AudioBuffer del host (sin oversampling). El processBlock
//     de arriba ya no la llama; el camino activo es la variante
//     Optimized dentro del dominio sobremuestreado.
//
//     Por qué se conserva:
//       (1) Muestra la topología más simple antes de introducir el
//           pipeline de oversampling; útil como punto de partida para
//           quien aprende manejo de eventos por muestra.
//       (2) Se puede reactivar (en processBlock) al depurar sin las
//           etapas de upsample / downsample, ayudando a aislar si un
//           bug vive en el synth o en el oversampler.
//
//     Notar que el osFactor de 2 también se conserva aquí porque cada
//     samplePosition de evento MIDI es una posición a la tasa del host,
//     pero la matemática del buffer espera posiciones en el dominio
//     renderizado. Si esta función se llegara a usar sin oversampling,
//     ajustar osFactor a 1.
void AndesJXAudioProcessor::splitBufferByEvents(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    constexpr int osFactor = 2;

    int bufferOffset = 0;

    for (const auto metadata : midiMessages)
    {
        const int eventPosOS = metadata.samplePosition * osFactor;

        int samplesThisSegment = eventPosOS - bufferOffset;
        if (samplesThisSegment > 0)
        {
            render(buffer, samplesThisSegment, bufferOffset);
            bufferOffset += samplesThisSegment;
        }

        if (metadata.numBytes <= 3)
        {
            uint8_t data1 = (metadata.numBytes >= 2) ? metadata.data[1] : 0;
            uint8_t data2 = (metadata.numBytes == 3) ? metadata.data[2] : 0;
            handleMIDI(metadata.data[0], data1, data2);
        }
    }

    int samplesLastSegment = buffer.getNumSamples() - bufferOffset;
    if (samplesLastSegment > 0)
        render(buffer, samplesLastSegment, bufferOffset);

    midiMessages.clear();
}


// ============================================================================
//  7. MIDI DISPATCH AND RENDER WRAPPER
//     DESPACHO MIDI Y WRAPPER DE RENDER
// ============================================================================

// EN: Single MIDI message handler. Two roles:
//       (1) Intercept Program Change (status 0xC0) and route it to the
//           preset loader, so a MIDI controller can switch presets.
//       (2) Forward every other message to Synth::midiMessage(),
//           which handles note-on, note-off, control change, pitch
//           bend and aftertouch.
// ES: Handler de un único mensaje MIDI. Dos roles:
//       (1) Interceptar Program Change (status 0xC0) y enrutarlo al
//           cargador de presets, para que un controlador MIDI pueda
//           cambiar de preset.
//       (2) Reenviar cualquier otro mensaje a Synth::midiMessage(),
//           que maneja note-on, note-off, control change, pitch bend
//           y aftertouch.
void AndesJXAudioProcessor::handleMIDI(uint8_t data0, uint8_t data1, uint8_t data2)
{
    if ((data0 & 0xF0) == 0xC0)
    {
        if (data1 < presets.size())
            setCurrentProgram(data1);
    }

    synth.midiMessage(data0, data1, data2);
}


// EN: Thin wrapper over Synth::render() that builds the per-channel
//     pointer pair the synth expects, with `bufferOffset` letting the
//     caller render only a sub-range of the buffer (used by
//     splitBufferByEvents to render between MIDI events).
// ES: Wrapper fino sobre Synth::render() que construye el par de
//     punteros por canal que espera el synth, con `bufferOffset`
//     permitiendo a quien llama renderizar solo un sub-rango del buffer
//     (lo usa splitBufferByEvents para renderizar entre eventos MIDI).
void AndesJXAudioProcessor::render(juce::AudioBuffer<float>& buffer, int sampleCount, int bufferOffset)
{
    float* outputBuffers[2] = { nullptr, nullptr };
    outputBuffers[0] = buffer.getWritePointer(0) + bufferOffset;
    if (getTotalNumOutputChannels() > 1)
        outputBuffers[1] = buffer.getWritePointer(1) + bufferOffset;

    synth.render(outputBuffers, sampleCount);
}


// ============================================================================
//  8. EDITOR AND SESSION STATE
//     EDITOR Y PERSISTENCIA DE SESIÓN
// ============================================================================

bool AndesJXAudioProcessor::hasEditor() const
{
    return true;
}

// EN: Creates the custom editor (PluginEditor.h). Returning a
//     juce::GenericAudioProcessorEditor here would give a free,
//     auto-generated UI that exposes every parameter as a slider; this
//     was useful during early development but the project now ships
//     the custom editor.
// ES: Crea el editor custom (PluginEditor.h). Devolver un
//     juce::GenericAudioProcessorEditor aquí daría una UI
//     auto-generada gratis que expone cada parámetro como slider;
//     esto fue útil al inicio del desarrollo pero el proyecto ya
//     entrega el editor custom.
juce::AudioProcessorEditor* AndesJXAudioProcessor::createEditor()
{
    return new AndesJXAudioProcessorEditor(*this);
}


// EN: Persist the plugin state to the host. Called when the user saves
//     a project. Two extra fields (currentProgram, isCustomPreset) are
//     stashed inside the APVTS state tree before serialization, so that
//     restoring the session also restores the active preset slot and
//     the "edited" flag.
// ES: Persistir el estado del plugin para el host. Se llama cuando el
//     usuario guarda un proyecto. Dos campos extra (currentProgram,
//     isCustomPreset) se guardan dentro del árbol de estado del APVTS
//     antes de serializar, para que al restaurar la sesión se
//     restauren también el slot del preset activo y la bandera
//     "editado".
void AndesJXAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    apvts.state.setProperty("currentProgram", currentProgram, nullptr);
    apvts.state.setProperty("isCustomPreset", isCustomPreset, nullptr);

    copyXmlToBinary(*apvts.copyState().createXml(), destData);
}


// EN: Restore the plugin state from the host. Called when the user
//     loads a project. The loadingPreset flag suppresses the
//     "became custom" detection during the bulk parameter update, the
//     same protection used by setCurrentProgram().
//
//     After restoration, the currentProgram and isCustomPreset values
//     are pulled back from the state tree. A final check against
//     currentStateMatchesProgram() catches the case where the saved
//     state matches no preset at all (e.g. a session saved with a
//     custom edit): isCustomPreset is forced to true to keep the UI
//     honest.
//
// ES: Restaurar el estado del plugin desde el host. Se llama cuando el
//     usuario carga un proyecto. La bandera loadingPreset suprime la
//     detección de "se volvió custom" durante la actualización masiva
//     de parámetros, la misma protección que usa setCurrentProgram().
//
//     Tras la restauración, los valores de currentProgram e
//     isCustomPreset se recuperan del árbol de estado. Un chequeo
//     final contra currentStateMatchesProgram() captura el caso en que
//     el estado guardado no coincide con ningún preset (p. ej. una
//     sesión guardada con una edición custom): isCustomPreset se
//     fuerza a true para mantener la UI honesta.
void AndesJXAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
    {
        loadingPreset = true;
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
        loadingPreset = false;

        currentProgram = static_cast<int>(apvts.state.getProperty("currentProgram", 0));
        isCustomPreset = static_cast<bool>(apvts.state.getProperty("isCustomPreset", false));

        if (currentProgram >= 0 && currentProgram < static_cast<int>(presets.size()))
            isCustomPreset = !currentStateMatchesProgram() ? true : isCustomPreset;

        parametersChanged.store(true);
        sendChangeMessage();
    }
}

// ============================================================================
//  9. FACTORY PRESET BANK
//     BANCO DE PRESETS DE FÁBRICA
// ============================================================================

// EN: Builds the AndesJX factory preset bank. Each preset is a Preset
//     instance constructed with its 32 parameter values in canonical
//     order (see Preset.h and Constants.h::NUM_PARAMS).
//
//     Conceptual frame:
//         The bank follows the AndesJX naming convention
//             [family] - [Andean geography] [musical / orchestral term]
//         where the family prefix groups presets by sonic role and the
//         geographic term anchors the timbral identity to a real
//         landscape (volcano, lagoon, range) of the Ecuadorian Andes.
//         The musical term clarifies the playing role (Pedal, Wobble,
//         Sostenuto, Pizzicato, etc.).
//
//     Family overview (33 presets total, including Init):
//         Bass    (7) : tectonic, sustained, percussive, rhythmic
//         Pad     (6) : atmospheric, slow, harmonically rich
//         Lead    (6) : melodic, foreground, monophonic-friendly
//         Brass / Wind / Organ (6): orchestral / aerial timbres
//         Keys    (3) : tonal percussive (Rhodes, Kalimba, Steelpan)
//         Pluck   (2) : short tonal articulation
//         FX      (3) : abstract textures and effects
//
// ES: Construye el banco de presets de fábrica de AndesJX. Cada preset
//     es una instancia de Preset construida con sus 32 valores de
//     parámetros en orden canónico (ver Preset.h y
//     Constants.h::NUM_PARAMS).
//
//     Marco conceptual:
//         El banco sigue la convención de nombres de AndesJX
//             [familia] - [geografía andina] [término musical / orquestal]
//         donde el prefijo de familia agrupa los presets por rol sonoro
//         y el término geográfico ancla la identidad tímbrica a un
//         paisaje real (volcán, laguna, cordillera) de los Andes
//         ecuatorianos. El término musical aclara el rol interpretativo
//         (Pedal, Wobble, Sostenuto, Pizzicato, etc.).
//
//     Visión por familias (33 presets en total, incluido Init):
//         Bass    (7) : tectónicos, sostenidos, percusivos, rítmicos
//         Pad     (6) : atmosféricos, lentos, armónicamente ricos
//         Lead    (6) : melódicos, primer plano, idóneos para mono
//         Brass / Wind / Organ (6): tímbricas orquestales / aéreas
//         Keys    (3) : percusivos tonales (Rhodes, Kalimba, Steelpan)
//         Pluck   (2) : articulación tonal breve
//         FX      (3) : texturas abstractas y efectos
void AndesJXAudioProcessor::createPrograms()
{
    // ------------------------------------------------------------------------
    //  Init / Init
    // ------------------------------------------------------------------------
    // EN: Neutral starting point. Loaded by setCurrentProgram(0) in the
    //     constructor; intended as a blank slate for users designing
    //     their own sounds.
    // ES: Punto de partida neutro. Lo carga setCurrentProgram(0) en el
    //     constructor; pensado como hoja en blanco para que el usuario
    //     diseñe sus propios sonidos.
    presets.emplace_back("Init",
        1.00f, 1.00f, 50.00f, -12.00f, 0.00f, 0.00f, 35.00f,
        0.00f, 1.00f, 75.00f, 15.00f, 50.00f, 0.00f,
        0.00f, 100.00f, 60.00f, 0.00f, 30.00f,
        0.00f, 25.00f, 0.00f, 50.00f, 100.00f,
        30.00f, 0.81f, 0.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 1.00f, 50.00f);


    // ------------------------------------------------------------------------
    //  Bass family / Familia Bass
    // ------------------------------------------------------------------------
    // EN: Seven low-register presets, each with a unique signature so
    //     they do not overlap when triggered with the same MIDI note:
    //       - Cotacachi Ostinato : acid squelch, repeating pulse
    //       - Imbabura Pedal     : sustained dark pedal tone
    //       - Chimborazo Sub     : pure sub-oscillator (sine bordon)
    //       - Tungurahua Wobble  : LFO-modulated cutoff (wobble bass)
    //       - Sincholagua Staccato: short percussive bass
    //       - Atacazo Pick       : noise-attack picked bass
    //       - Chiles Portamento  : fretless-style with audible glide
    // ES: Siete presets de registro grave, cada uno con una firma
    //     propia para que no se pisen al tocar la misma nota MIDI:
    //       - Cotacachi Ostinato : squelch acid, pulso repetitivo
    //       - Imbabura Pedal     : nota pedal oscura sostenida
    //       - Chimborazo Sub     : sub puro (bordón sinusoidal)
    //       - Tungurahua Wobble  : cutoff modulado por LFO (wobble bass)
    //       - Sincholagua Staccato: bajo percusivo corto
    //       - Atacazo Pick       : bajo pulsado con ataque ruidoso
    //       - Chiles Portamento  : estilo fretless con glide audible

    presets.emplace_back("Bass - Cotacachi Ostinato",
        1.00f, 1.00f, 0.00f, 0.00f, 0.00f, 1.00f, 49.00f,
        1.00f, 1.00f, 55.00f, 75.00f, 38.00f, 0.00f,
        0.00f, 45.00f, 60.00f, 0.00f, 48.00f,
        0.00f, 18.00f, 0.00f, 38.00f, 70.00f,
        25.00f, 0.20f, 0.00f, 0.00f, -2.00f,
        0.00f, 0.00f, 0.00f, 15.00f);

    presets.emplace_back("Bass - Imbabura Pedal",
        1.00f, 2.00f, 45.00f, -12.00f, -10.90f, 1.00f, 19.00f,
        1.00f, 1.00f, 32.00f, 42.00f, 38.00f, 0.00f,
        -40.00f, 50.00f, 60.00f, 0.00f, 42.00f,
        0.00f, 18.00f, 0.00f, 35.00f, 70.00f,
        20.00f, 0.20f, 0.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 0.00f, 8.00f);

    presets.emplace_back("Bass - Chimborazo Sub",
        0.00f, 3.00f, 0.00f, -12.00f, 0.00f, 0.00f, 35.00f,
        0.00f, 0.00f, 26.00f, 42.00f, 8.00f, 0.00f,
        0.00f, 40.00f, 60.00f, 0.00f, 22.00f,
        0.00f, 12.00f, 0.00f, 38.00f, 25.00f,
        12.00f, 0.12f, 0.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 0.00f, 8.00f);

    presets.emplace_back("Bass - Tungurahua Wobble",
        1.00f, 4.00f, 55.00f, -12.00f, -8.80f, 0.00f, 82.00f,
        1.00f, 1.00f, 72.00f, 42.00f, -24.00f, 28.00f,
        20.00f, 45.00f, 60.00f, 10.00f, 48.00f,
        20.00f, 18.00f, 0.00f, 38.00f, 80.00f,
        20.00f, 0.42f, 0.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 0.00f, 10.00f);

    presets.emplace_back("Bass - Sincholagua Staccato",
        2.00f, 3.00f, 49.00f, -12.00f, 1.60f, 1.00f, 35.00f,
        1.00f, 1.00f, 36.00f, 15.00f, 42.00f, 8.00f,
        0.00f, 45.00f, 60.00f, 0.00f, 28.00f,
        0.00f, 18.00f, 0.00f, 25.00f, 80.00f,
        20.00f, 0.19f, 0.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 0.00f, 10.00f);

    presets.emplace_back("Bass - Atacazo Pick",
        3.00f, 2.00f, 37.00f, 0.00f, 7.80f, 0.00f, 22.00f,
        0.00f, 0.00f, 36.00f, 40.00f, 32.00f, 8.00f,
        10.00f, 45.00f, 60.00f, 0.00f, 20.00f,
        0.00f, 14.00f, 4.00f, 40.00f, 0.00f,
        18.00f, 0.15f, 10.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 0.00f, 10.00f);

    presets.emplace_back("Bass - Chiles Portamento",
        3.00f, 1.00f, 50.00f, 0.00f, -14.40f, 1.00f, 40.00f,
        0.00f, 0.00f, 48.00f, 0.00f, 12.00f, 0.00f,
        20.00f, 45.00f, 60.00f, 0.00f, 12.00f,
        0.00f, 18.00f, 15.00f, 65.00f, 20.00f,
        18.00f, 0.25f, 0.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 0.00f, 12.00f);


    // ------------------------------------------------------------------------
    //  Pad family / Familia Pad
    // ------------------------------------------------------------------------
    // EN: Six atmospheric presets distinguished by structural identity:
    //       - Cayambe 5th         : interval of a fifth between oscs
    //       - Paramo Sostenuto    : longest envelope releases of the bank
    //       - Altar Pianissimo    : noise + noise oscillators (intimate)
    //       - Pululahua Crescendo : strong velocity-to-filter response
    //       - Corazon Strings     : classic string ensemble
    //       - Iliniza Shimmer     : fast LFO on the filter (shimmer)
    // ES: Seis presets atmosféricos diferenciados por identidad
    //     estructural:
    //       - Cayambe 5th         : intervalo de quinta entre osciladores
    //       - Paramo Sostenuto    : releases más largos del banco
    //       - Altar Pianissimo    : osciladores ruido + ruido (íntimo)
    //       - Pululahua Crescendo : fuerte respuesta velocity al filtro
    //       - Corazon Strings     : ensemble de cuerdas clásico
    //       - Iliniza Shimmer     : LFO rápido sobre el filtro (shimmer)

    presets.emplace_back("Pad - Cayambe 5th",
        1.00f, 2.00f, 40.00f, -7.00f, -6.30f, 1.00f, 32.00f,
        0.00f, 0.00f, 75.00f, 25.00f, 42.00f, 10.00f,
        0.00f, 100.00f, 60.00f, 90.00f, 80.00f,
        72.00f, 80.00f, 90.00f, 80.00f, 80.00f,
        80.00f, 0.30f, 5.00f, 0.00f, 0.00f,
        0.00f, -4.00f, 1.00f, 95.00f);

    presets.emplace_back("Pad - Paramo Sostenuto",
        1.00f, 3.00f, 55.00f, 0.00f, 0.00f, 0.00f, 49.00f,
        0.00f, 0.00f, 55.00f, 28.00f, 30.00f, 8.00f,
        10.00f, 100.00f, 60.00f, 100.00f, 86.00f,
        76.00f, 70.00f, 30.00f, 80.00f, 68.00f,
        80.00f, 0.35f, 5.00f, 25.00f, 0.00f,
        0.00f, 0.00f, 1.00f, 96.00f);

    presets.emplace_back("Pad - Altar Pianissimo",
        3.00f, 3.00f, 39.00f, 0.00f, -4.90f, 2.00f, 12.00f,
        0.00f, 0.00f, 42.00f, 60.00f, 0.00f, 0.00f,
        0.00f, 60.00f, 60.00f, 20.00f, 35.00f,
        50.00f, 30.00f, 40.00f, 65.00f, 70.00f,
        35.00f, 0.25f, 0.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 1.00f, 70.00f);

    presets.emplace_back("Pad - Pululahua Crescendo",
        1.00f, 3.00f, 41.00f, 0.00f, 9.70f, 0.00f, 8.00f,
        0.00f, 0.00f, 49.00f, 12.00f, -15.00f, 0.00f,
        48.00f, 100.00f, 60.00f, 61.00f, 87.00f,
        100.00f, 93.00f, 11.00f, 48.00f, 98.00f,
        45.00f, 0.31f, 0.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 1.00f, 80.00f);

    presets.emplace_back("Pad - Corazon Strings",
        1.00f, 2.00f, 60.00f, 0.00f, -7.10f, 0.00f, 0.00f,
        0.00f, 0.00f, 48.00f, 20.00f, 35.00f, 8.00f, 18.00f,
        60.00f, 60.00f, 20.00f, 55.00f, 45.00f, 62.00f,
        65.00f, 60.00f, 100.00f, 35.00f, 0.30f, 10.00f,
        10.00f, 0.00f, 0.00f, 0.00f, 1.00f, 82.00f);

    presets.emplace_back("Pad - Iliniza Shimmer",
        0.00f, 3.00f, 45.00f, 0.00f, 0.00f, 0.00f, 49.00f,
        0.00f, 0.00f, 58.00f, 42.00f, 22.00f, 6.00f,
        15.00f, 100.00f, 60.00f, 70.00f, 55.00f,
        65.00f, 50.00f, 8.00f, 18.00f, 35.00f,
        62.00f, 0.42f, 0.00f, 0.00f, 2.00f,
        0.00f, 0.00f, 1.00f, 85.00f);


    // ------------------------------------------------------------------------
    //  Lead family / Familia Lead
    // ------------------------------------------------------------------------
    // EN: Six melodic presets, three sawtooth-based and three with
    //     distinct waveform identities to avoid overlap:
    //       - Cotopaxi Acid       : 303-style acid lead, glide always on
    //       - Pichincha Unison    : strong detune unison saw
    //       - Sangay Fortissimo   : aggressive saw + pulse, long release
    //       - Rucu Mono           : monophonic with negative keytrack
    //       - Guagua Flute        : breathy flute-like lead with vibrato
    //       - Ruminahui Square    : square-based delicate pluck
    // ES: Seis presets melódicos, tres basados en saw y tres con
    //     identidades de forma de onda distintas para evitar solapes:
    //       - Cotopaxi Acid       : lead acid estilo 303, glide siempre
    //       - Pichincha Unison    : unison saw con detune fuerte
    //       - Sangay Fortissimo   : saw + pulse agresivos, release largo
    //       - Rucu Mono           : monofónico con keytrack negativo
    //       - Guagua Flute        : lead tipo flauta con vibrato
    //       - Ruminahui Square    : pluck delicado basado en square

    presets.emplace_back("Lead - Cotopaxi Acid",
        1.00f, 2.00f, 65.00f, 7.00f, -7.10f, 2.00f, 34.00f,
        0.00f, 1.00f, 65.00f, 65.00f, 55.00f, 12.00f,
        0.00f, 0.00f, 60.00f, 60.00f, 30.00f,
        0.00f, 25.00f, 12.00f, 50.00f, 100.00f,
        12.00f, 0.55f, 0.00f, 0.00f, 1.00f,
        0.00f, 0.00f, 1.00f, 20.00f);

    presets.emplace_back("Lead - Pichincha Unison",
        1.00f, 1.00f, 60.00f, 0.00f, -17.20f, 2.00f, 41.00f,
        0.00f, 1.00f, 54.00f, 30.00f, 18.00f, 8.00f,
        12.00f, 60.00f, 60.00f, 0.00f, 9.00f,
        100.00f, 18.00f, 10.00f, 60.00f, 100.00f,
        18.00f, 0.45f, -12.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 1.00f, 55.00f);

    presets.emplace_back("Lead - Sangay Fortissimo",
        1.00f, 4.00f, 71.00f, 12.00f, 0.00f, 0.00f, 24.00f,
        0.00f, 1.00f, 60.00f, 42.00f, 38.00f, 6.00f,
        12.00f, 60.00f, 60.00f, 8.00f, 18.00f,
        55.00f, 20.00f, 35.00f, 60.00f, 100.00f,
        12.00f, 0.40f, -8.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 1.00f, 18.00f);

    presets.emplace_back("Lead - Rucu Mono",
        1.00f, 2.00f, 62.00f, -12.00f, 0.00f, 1.00f, 42.00f,
        0.00f, 1.00f, 64.00f, 39.00f, 18.00f, 8.00f,
        -50.00f, 60.00f, 60.00f, 7.00f, 52.00f,
        24.00f, 40.00f, 13.00f, 30.00f, 76.00f,
        21.00f, 0.35f, -8.00f, 0.00f, -1.00f,
        0.00f, 0.00f, 0.00f, 5.00f);

    presets.emplace_back("Lead - Guagua Flute",
        1.00f, 3.00f, 62.00f, 12.00f, -9.80f, 1.00f, 15.00f,
        0.00f, 1.00f, 48.00f, 17.00f, 32.00f, 8.00f,
        12.00f, 60.00f, 60.00f, 0.00f, 47.00f,
        19.00f, 28.00f, 0.00f, 50.00f, 20.00f,
        33.00f, 0.45f, 3.00f, 3.00f, -2.00f,
        0.00f, 0.00f, 0.00f, 12.00f);

    presets.emplace_back("Lead - Ruminahui Square",
        2.00f, 1.00f, 48.00f, 0.00f, -8.80f, 0.00f, 0.00f,
        0.00f, 0.00f, 58.00f, 30.00f, 38.00f, 8.00f,
        18.00f, 60.00f, 60.00f, 0.00f, 16.00f,
        0.00f, 10.00f, 0.00f, 42.00f, 0.00f,
        18.00f, 0.18f, 3.00f, 2.00f, 0.00f,
        0.00f, 0.00f, 1.00f, 28.00f);


    // ------------------------------------------------------------------------
    //  Brass / Wind / Organ family / Familia Brass / Wind / Organ
    // ------------------------------------------------------------------------
    // EN: Six orchestral / aerial presets covering the registers a
    //     wind ensemble would occupy:
    //       - Carihuairazo Horn   : soft solo horn
    //       - Condor Tutti        : wide brass ensemble
    //       - Antisana Fanfare    : detuned synth-brass fanfare
    //       - Sumaco Rotary       : rotary-emulating organ
    //       - Llanganates Clarinet: dark bass clarinet register
    //       - Ilalo Whistle       : bright whistle / piccolo
    // ES: Seis presets orquestales / aéreos que cubren los registros
    //     de un ensemble de vientos:
    //       - Carihuairazo Horn   : corno solista suave
    //       - Condor Tutti        : ensemble amplio de metales
    //       - Antisana Fanfare    : fanfarria de brass sintético detuneado
    //       - Sumaco Rotary       : órgano con emulación rotary
    //       - Llanganates Clarinet: clarinete bajo, registro oscuro
    //       - Ilalo Whistle       : silbido / pícolo brillante

    presets.emplace_back("Wind - Carihuairazo Horn",
        3.00f, 2.00f, 25.00f, 12.00f, 1.90f, 0.00f, 35.00f,
        0.00f, 0.00f, 46.00f, 16.00f, -18.00f, 4.00f,
        8.00f, 60.00f, 60.00f, 4.00f, 26.00f,
        28.00f, 22.00f, 8.00f, 38.00f, 80.00f,
        20.00f, 0.18f, 0.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 1.00f, 55.00f);

    presets.emplace_back("Brass - Condor Tutti",
        1.00f, 2.00f, 45.00f, 12.00f, -7.90f, 0.00f, 28.00f,
        0.00f, 1.00f, 52.00f, 14.00f, 18.00f, 0.00f,
        0.00f, 60.00f, 60.00f, 16.00f, 14.00f,
        65.00f, 6.00f, 12.00f, 46.00f, 70.00f,
        26.00f, 0.18f, 0.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 1.00f, 72.00f);

    presets.emplace_back("Brass - Antisana Fanfare",
        1.00f, 1.00f, 55.00f, 0.00f, 14.00f, 0.00f, 31.00f,
        0.00f, 1.00f, 34.00f, 10.00f, 48.00f, 0.00f,
        12.00f, 60.00f, 60.00f, 0.00f, 42.00f,
        0.00f, 10.00f, 0.00f, 45.00f, 60.00f,
        24.00f, 0.20f, 0.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 1.00f, 55.00f);

    presets.emplace_back("Organ - Sumaco Rotary",
        2.00f, 2.00f, 0.00f, 0.00f, 0.00f, 0.00f, 13.00f,
        0.00f, 0.00f, 40.00f, 55.00f, 6.00f, 10.00f, -60.00f,
        60.00f, 60.00f, 0.00f, 40.00f, 40.00f, 30.00f,
        0.00f, 15.00f, 70.00f, 22.00f, 0.48f, -12.00f,
        0.00f, 0.00f, 0.00f, -4.00f, 1.00f, 30.00f);

    presets.emplace_back("Wind - Llanganates Clarinet",
        2.00f, 3.00f, 65.00f, 0.00f, 0.00f, 1.00f, 0.00f,
        0.00f, 0.00f, 42.00f, 12.00f, 0.00f, 6.00f,
        0.00f, 60.00f, 60.00f, 0.00f, 0.00f,
        0.00f, 26.00f, 30.00f, 50.00f, 55.00f,
        28.00f, 0.25f, 0.00f, 8.00f, 0.00f,
        0.00f, -1.00f, 1.00f, 12.00f);

    presets.emplace_back("Wind - Ilalo Whistle",
        0.00f, 3.00f, 23.00f, 0.00f, -0.70f, 0.00f, 35.00f,
        0.00f, 0.00f, 38.00f, 72.00f, 0.00f, 0.00f,
        0.00f, 45.00f, 60.00f, 0.00f, 22.00f,
        0.00f, 14.00f, 55.00f, 30.00f, 45.00f,
        22.00f, 0.48f, 10.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 1.00f, 28.00f);


    // ------------------------------------------------------------------------
    //  Keys / Pluck / FX family / Familia Keys / Pluck / FX
    // ------------------------------------------------------------------------
    // EN: Eight final presets grouped into three subfamilies:
    //       Keys (3): tonal sustained percussives
    //         - Mojanda Rhodes    : Rhodes-style electric piano
    //         - Yanahurco Kalimba : dry, short kalimba
    //         - Quilindana Steelpan: metallic steelpan
    //       Pluck (2): short tonal articulation
    //         - Pasochoa Pizzicato: orchestral pizzicato
    //         - Cuicocha Bubble   : LFO-modulated quirky pluck
    //       FX (3): abstract textures
    //         - Cajas PWM         : wide PWM pad-texture
    //         - Quilotoa Aqua     : velocity-sensitive watery texture
    //         - Reventador Ghost  : breathy spectral texture
    // ES: Ocho presets finales agrupados en tres subfamilias:
    //       Keys (3): percusivos tonales sostenibles
    //         - Mojanda Rhodes    : piano eléctrico estilo Rhodes
    //         - Yanahurco Kalimba : kalimba seca y corta
    //         - Quilindana Steelpan: steelpan metálica
    //       Pluck (2): articulación tonal breve
    //         - Pasochoa Pizzicato: pizzicato orquestal
    //         - Cuicocha Bubble   : pluck quirky modulado por LFO
    //       FX (3): texturas abstractas
    //         - Cajas PWM         : textura amplia PWM
    //         - Quilotoa Aqua     : textura acuosa sensible a velocity
    //         - Reventador Ghost  : textura espectral con respiración

    presets.emplace_back("Keys - Mojanda Rhodes",
        3.00f, 2.00f, 35.00f, 0.00f, -0.20f, 0.00f, 35.00f,
        0.00f, 0.00f, 36.00f, 18.00f, 18.00f, 0.00f,
        12.00f, 60.00f, 60.00f, 0.00f, 16.00f,
        0.00f, 32.00f, 20.00f, 48.00f, 0.00f,
        20.00f, 0.15f, 0.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 1.00f, 35.00f);

    presets.emplace_back("Keys - Yanahurco Kalimba",
        3.00f, 0.00f, 40.00f, 12.00f, 8.00f, 0.00f, 35.00f,
        0.00f, 0.00f, 38.00f, 26.00f, 22.00f, 0.00f,
        0.00f, 60.00f, 60.00f, 0.00f, 24.00f,
        0.00f, 22.00f, 0.00f, 22.00f, 0.00f,
        15.00f, 0.15f, 0.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 1.00f, 48.00f);

    presets.emplace_back("Keys - Quilindana Steelpan",
        0.00f, 3.00f, 55.00f, 12.00f, -8.00f, 0.00f, 18.00f,
        0.00f, 0.00f, 42.00f, 32.00f, 18.00f, 6.00f,
        -8.00f, 60.00f, 60.00f, 0.00f, 24.00f,
        16.00f, 28.00f, 8.00f, 34.00f, 0.00f,
        22.00f, 0.18f, 0.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 1.00f, 40.00f);

    presets.emplace_back("Pluck - Pasochoa Pizzicato",
        2.00f, 3.00f, 0.00f, -12.00f, 0.00f, 0.00f, 35.00f,
        0.00f, 0.00f, 28.00f, 20.00f, 28.00f, 0.00f,
        0.00f, 45.00f, 60.00f, 0.00f, 18.00f,
        0.00f, 8.00f, 0.00f, 30.00f, 0.00f,
        15.00f, 0.15f, 0.00f, 0.00f, 0.00f,
        0.00f, -3.00f, 1.00f, 35.00f);

    presets.emplace_back("Pluck - Cuicocha Bubble",
        0.00f, 3.00f, 25.00f, -12.00f, 0.00f, 0.00f, 71.00f,
        0.00f, 0.00f, 28.00f, 52.00f, 38.00f, 16.00f,
        12.00f, 60.00f, 60.00f, 20.00f, 24.00f,
        28.00f, 18.00f, 0.00f, 30.00f, 22.00f,
        22.00f, 0.38f, 0.00f, 0.00f, 0.00f,
        0.00f, -3.00f, 1.00f, 45.00f);

    presets.emplace_back("FX - Cajas PWM",
        1.00f, 4.00f, 55.00f, -12.00f, -8.80f, 0.00f, 35.00f,
        0.00f, 1.00f, 68.00f, 18.00f, 38.00f, 0.00f,
        -40.00f, 60.00f, 60.00f, 18.00f, 22.00f,
        62.00f, 18.00f, 30.00f, 46.00f, 68.00f,
        26.00f, 0.28f, -8.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 1.00f, 65.00f);

    presets.emplace_back("FX - Quilotoa Aqua",
        1.00f, 3.00f, 60.00f, 0.00f, -1.40f, 0.00f, 49.00f,
        0.00f, 0.00f, 72.00f, 42.00f, 55.00f, 14.00f,
        -42.00f, 60.00f, 60.00f, 42.00f, 40.00f,
        48.00f, 55.00f, 6.00f, 50.00f, 18.00f,
        24.00f, 0.24f, 4.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 1.00f, 75.00f);

    presets.emplace_back("FX - Reventador Ghost",
        3.00f, 1.00f, 55.00f, 0.00f, -7.10f, 2.00f, 16.00f,
        0.00f, 0.00f, 38.00f, 38.00f, 30.00f, 4.00f,
        18.00f, 60.00f, 60.00f, 0.00f, 22.00f,
        24.00f, 28.00f, 18.00f, 42.00f, 52.00f,
        26.00f, 0.20f, 6.00f, 0.00f, 0.00f,
        0.00f, 0.00f, 1.00f, 65.00f);
}

// ============================================================================
//  10. APVTS PARAMETER LAYOUT
//      LAYOUT DE PARÁMETROS DEL APVTS
// ============================================================================

// EN: Defines the 32 plugin parameters exposed to the host. For each
//     parameter the layout specifies:
//       - A unique ParameterID (declared in PluginProcessor.h)
//       - A user-visible name shown in the host's UI
//       - The numeric range, step, and (where useful) a perceptual skew
//       - The default value
//       - Optional formatting attributes (label / string formatter)
//
//     The order of declaration here does NOT need to match the
//     canonical order used by Preset.h or NUM_PARAMS; the cached
//     pointers in the constructor are what enforce that contract.
//
//     Several parameters use custom string formatters (lambdas) to
//     turn the numeric value into a human-readable label (e.g. mix
//     ratio "70:30", note name "C4", or special states like "OFF" /
//     "PWM 25.0"). These are documented inline.
//
// ES: Define los 32 parámetros del plugin expuestos al host. Para cada
//     parámetro el layout especifica:
//       - Un ParameterID único (declarado en PluginProcessor.h)
//       - Un nombre visible que muestra el host en su UI
//       - El rango numérico, paso y (cuando aplica) skew perceptual
//       - El valor por defecto
//       - Atributos opcionales de formato (etiqueta / formateador de
//         string)
//
//     El orden de declaración aquí NO necesita coincidir con el orden
//     canónico usado por Preset.h o NUM_PARAMS; los punteros cacheados
//     en el constructor son los que garantizan ese contrato.
//
//     Varios parámetros usan formateadores de string custom (lambdas)
//     para convertir el valor numérico en una etiqueta legible (p. ej.
//     mix ratio "70:30", nombre de nota "C4", o estados especiales
//     como "OFF" / "PWM 25.0"). Estos se documentan en línea.
juce::AudioProcessorValueTreeState::ParameterLayout AndesJXAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;


    // ------------------------------------------------------------------------
    //  Oscillators / Osciladores
    // ------------------------------------------------------------------------

    // EN: AudioParameterChoice for the discrete waveform selector.
    //     The string array order MUST match the toWave() lambda in
    //     update(): 0=Sine, 1=Saw, 2=Square, 3=Triangle, 4=PWM.
    // ES: AudioParameterChoice para el selector discreto de onda.
    //     El orden del array de strings DEBE coincidir con la lambda
    //     toWave() en update(): 0=Sine, 1=Saw, 2=Square, 3=Triangle,
    //     4=PWM.
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParameterID::osc1Wave,
        "Osc1 Wave",
        juce::StringArray{ "Sine", "Saw", "Square", "Triangle", "PWM" },
        1));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParameterID::osc2Wave,
        "Osc2 Wave",
        juce::StringArray{ "Sine", "Saw", "Square", "Triangle", "PWM" },
        1));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::stereoWidth,
        "Stereo Width",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParameterID::polyMode,
        "Polyphony",
        juce::StringArray{ "Mono", "Poly" },
        1));

    // EN: Oscillator tune in semitones. Default of -12 anchors osc2 one
    //     octave below osc1, a standard "thick" interval used as the
    //     starting point of the Init preset.
    // ES: Tune del oscilador en semitonos. El default de -12 ancla osc2
    //     una octava por debajo de osc1, un intervalo "grueso" estándar
    //     usado como punto de partida del preset Init.
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::oscTune,
        "Osc Tune",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 1.0f),
        -12.0f,
        juce::AudioParameterFloatAttributes().withLabel("semi")));

    // EN: Oscillator fine tune in cents. The 0.3 skew gives more
    //     resolution near zero, where users typically need the
    //     finest detune control.
    // ES: Fine tune del oscilador en cents. El skew de 0.3 da más
    //     resolución cerca de cero, donde el usuario típicamente
    //     necesita el control de detune más fino.
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::oscFine,
        "Osc Fine",
        juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f, 0.3f, true),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("cent")));

    // EN: Custom string formatter that displays the oscillator mix as
    //     a ratio "osc1:osc2" instead of a single percentage. This is
    //     more intuitive for users thinking about which source
    //     dominates the blend.
    // ES: Formateador custom que muestra la mezcla de osciladores como
    //     una proporción "osc1:osc2" en vez de un único porcentaje.
    //     Es más intuitivo para el usuario pensando en qué fuente
    //     domina la mezcla.
    auto oscMixStringFromValue = [](float value, int)
        {
            const int osc2 = juce::roundToInt(value);
            const int osc1 = 100 - osc2;
            return juce::String(osc1) + ":" + juce::String(osc2);
        };

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::oscMix,
        "Osc Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f),
        50.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(oscMixStringFromValue)));


    // ------------------------------------------------------------------------
    //  Glide / Glide
    // ------------------------------------------------------------------------

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParameterID::glideMode,
        "Glide Mode",
        juce::StringArray{ "Off", "Legato", "Always" },
        0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::glideRate,
        "Glide Rate",
        juce::NormalisableRange<float>(0.0f, 100.f, 1.0f),
        35.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // EN: Glide bend in semitones. The 0.4 skew gives finer control
    //     near zero (subtle drift) while still allowing wide bends at
    //     the extremes.
    // ES: Glide bend en semitonos. El skew de 0.4 da control más fino
    //     cerca de cero (drift sutil) y sigue permitiendo bends
    //     amplios en los extremos.
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::glideBend,
        "Glide Bend",
        juce::NormalisableRange<float>(-36.0f, 36.0f, 0.01f, 0.4f, true),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("semi")));


    // ------------------------------------------------------------------------
    //  Filter / Filtro
    // ------------------------------------------------------------------------

    // EN: Filter algorithm selector. Order MUST match the toFilterType()
    //     lambda in update() and the FilterType enum in Synth.h.
    // ES: Selector de algoritmo de filtro. El orden DEBE coincidir con
    //     la lambda toFilterType() en update() y con el enum FilterType
    //     en Synth.h.
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParameterID::filterType,
        "Filter Type",
        juce::StringArray{ "SVF", "Moog" },
        1));
    // EN: Filter cutoff knob. Exposed as a percentage [0, 100] but
    //     mapped logarithmically to [80 Hz, 20 kHz] in updateParameters
    //     (equal knob movement -> equal pitch movement). The "%" label
    //     reflects the raw parameter; the actual frequency range is
    //     stated here and in the user manual.
    // ES: Knob de cutoff del filtro. Se expone como porcentaje
    //     [0, 100] pero se mapea logarítmicamente a [80 Hz, 20 kHz]
    //     en updateParameters (igual movimiento de knob -> igual
    //     movimiento de altura). La etiqueta "%" refleja el parámetro
    //     crudo; el rango real en frecuencia se documenta aquí y en
    //     el manual del usuario.
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterFreq,
        "Filter Freq",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        75.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterReso,
        "Filter Reso",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        15.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // EN: Filter envelope amount, bipolar. Positive opens the cutoff
    //     with the envelope, negative closes it.
    // ES: Cantidad de envolvente al filtro, bipolar. Positivo abre el
    //     cutoff con la envolvente, negativo lo cierra.
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterEnv,
        "Filter Env",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
        50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterLFO,
        "Filter LFO",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // EN: Filter velocity formatter. Values below -90 display as "OFF",
    //     a special state used by update() to bypass velocity tracking
    //     entirely (instead of just being a very small amount).
    // ES: Formateador de filter velocity. Los valores bajo -90 se
    //     muestran como "OFF", un estado especial usado por update()
    //     para deshabilitar el seguimiento de velocity por completo
    //     (en lugar de quedar como una cantidad muy pequeña).
    auto filterVelocityStringFromValue = [](float value, int)
        {
            if (value < -90.0f)
                return juce::String("OFF");
            else
                return juce::String(value);
        };

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterVelocity,
        "Velocity",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("%")
        .withStringFromValueFunction(filterVelocityStringFromValue)));


    // ------------------------------------------------------------------------
    //  Key tracking / Key tracking
    // ------------------------------------------------------------------------

    // EN: Key tracking amount in [0, 200] %. 100 % = one octave of
    //     cutoff shift per octave of keyboard distance from the
    //     keycenter (perfect tracking). 200 % = double tracking.
    // ES: Cantidad de key tracking en [0, 200] %. 100 % = una octava
    //     de desplazamiento del cutoff por octava de distancia de
    //     teclado desde el keycenter (tracking perfecto). 200 % =
    //     tracking doble.
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterKeytrack,
        "Filter Keytrack",
        juce::NormalisableRange<float>(0.0f, 200.0f, 1.0f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // EN: Custom formatter that converts a MIDI note number into the
    //     standard note-name format (C4, F#3, etc.). Works for the
    //     full [24, 96] range exposed by the parameter.
    // ES: Formateador custom que convierte un número de nota MIDI al
    //     formato estándar de nombres de nota (C4, F#3, etc.). Cubre
    //     todo el rango [24, 96] expuesto por el parámetro.
    auto keycenterStringFromValue = [](float value, int)
        {
            static const char* noteNames[] = {
                "C", "C#", "D", "D#", "E", "F",
                "F#", "G", "G#", "A", "A#", "B"
            };
            int note = (int)value;
            int octave = (note / 12) - 1;
            int noteName = note % 12;
            return juce::String(noteNames[noteName]) + juce::String(octave);
        };

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterKeycenter,
        "Filter Keycenter",
        juce::NormalisableRange<float>(24.0f, 96.0f, 1.0f),
        60.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("")
        .withStringFromValueFunction(keycenterStringFromValue)));


    // ------------------------------------------------------------------------
    //  Envelopes (filter and amplitude) / Envolventes (filtro y amplitud)
    // ------------------------------------------------------------------------

    // EN: Shared formatter for all envelope-stage knobs: rounds to an
    //     integer instead of showing decimals, since the underlying
    //     range is [0, 100] anyway.
    // ES: Formateador compartido para todos los knobs de etapas de
    //     envolvente: redondea a entero en lugar de mostrar decimales,
    //     dado que el rango interno es [0, 100] de todos modos.
    auto envStageStringFromValue = [](float value, int)
        {
            return juce::String(juce::roundToInt(value));
        };

    auto sustainStringFromValue = [](float value, int)
        {
            return juce::String(juce::roundToInt(value));
        };

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterAttack, "Filter Attack",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 0.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("").withStringFromValueFunction(envStageStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterDecay, "Filter Decay",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 30.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("").withStringFromValueFunction(envStageStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterSustain, "Filter Sustain",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 0.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("").withStringFromValueFunction(sustainStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterRelease, "Filter Release",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 25.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("").withStringFromValueFunction(envStageStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::envAttack, "Env Attack",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 0.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("").withStringFromValueFunction(envStageStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::envDecay, "Env Decay",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 50.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("").withStringFromValueFunction(envStageStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::envSustain, "Env Sustain",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("").withStringFromValueFunction(sustainStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::envRelease, "Env Release",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 30.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("").withStringFromValueFunction(envStageStringFromValue)));


    // ------------------------------------------------------------------------
    //  LFO and modulation / LFO y modulación
    // ------------------------------------------------------------------------

    // EN: LFO rate display formatter. Mirrors the exp(7x - 4) mapping
    //     used in update() so the value the user sees is the actual
    //     frequency in Hz, with three decimals (covers ~0.018 Hz at
    //     the low end accurately).
    // ES: Formateador de display de la rate del LFO. Replica el mapeo
    //     exp(7x - 4) usado en update() para que el valor que ve el
    //     usuario sea la frecuencia real en Hz, con tres decimales
    //     (cubre con precisión ~0.018 Hz en el extremo bajo).
    auto lfoRateStringFromValue = [](float value, int)
        {
            float lfoHz = std::exp(7.0f * value - 4.0f);
            return juce::String(lfoHz, 3);
        };

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::lfoRate,
        "LFO Rate",
        juce::NormalisableRange<float>(),
        0.81f,
        juce::AudioParameterFloatAttributes()
        .withLabel("Hz")
        .withStringFromValueFunction(lfoRateStringFromValue)));

    // EN: Vibrato formatter. The bipolar parameter has different
    //     semantics on each side (positive = vibrato depth, negative
    //     = PWM depth), so the display switches between "value" and
    //     "PWM value" to make the user-facing meaning explicit.
    // ES: Formateador de vibrato. El parámetro bipolar tiene semánticas
    //     distintas en cada lado (positivo = profundidad de vibrato,
    //     negativo = profundidad PWM), así que el display alterna
    //     entre "valor" y "PWM valor" para hacer explícito el
    //     significado al usuario.
    auto vibratoStringFromValue = [](float value, int)
        {
            if (value < 0.0f)
                return "PWM " + juce::String(-value, 1);
            else
                return juce::String(value, 1);
        };

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::vibrato,
        "Vibrato",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("%")
        .withStringFromValueFunction(vibratoStringFromValue)));


    // ------------------------------------------------------------------------
    //  Noise, tuning, output / Ruido, afinación, salida
    // ------------------------------------------------------------------------

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::noise,
        "Noise",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::octave,
        "Octave",
        juce::NormalisableRange<float>(-2.0f, 2.0f, 1.0f),
        0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::tuning,
        "Tuning",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("cent")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::outputLevel,
        "Output Level",
        juce::NormalisableRange<float>(-24.0f, 6.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    return layout;
}


//==============================================================================
// EN: Plugin factory function required by JUCE. Called by the host to
//     create new instances of this plugin.
// ES: Función factoría del plugin requerida por JUCE. La llama el host
//     para crear nuevas instancias de este plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AndesJXAudioProcessor();
}
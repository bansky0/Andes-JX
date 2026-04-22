/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Utils.h"
#include "Constants.h"

//==============================================================================
AndesJXAudioProcessor::AndesJXAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
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

    apvts.state.addListener(this);
}

AndesJXAudioProcessor::~AndesJXAudioProcessor()
{
    apvts.state.removeListener(this);
}

//==============================================================================
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

double AndesJXAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AndesJXAudioProcessor::getNumPrograms()
{
    return int(presets.size());
}

int AndesJXAudioProcessor::getCurrentProgram()
{
    return currentProgram;
}

bool AndesJXAudioProcessor::currentStateMatchesProgram() const
{
    if (currentProgram < 0 || currentProgram >= static_cast<int>(presets.size()))
        return false;

    const Preset& preset = presets[currentProgram];

    const juce::RangedAudioParameter* params[NUM_PARAMS] =
    {
        osc1WaveParam,
        osc2WaveParam,
        oscMixParam,
        oscTuneParam,
        oscFineParam,
        glideModeParam,
        glideRateParam,
        glideBendParam,
        filterTypeParam,
        filterFreqParam,
        filterResoParam,
        filterEnvParam,
        filterLFOParam,
        filterVelocityParam,
        filterKeytrackParam,
        filterKeycenterParam,
        filterAttackParam,
        filterDecayParam,
        filterSustainParam,
        filterReleaseParam,
        envAttackParam,
        envDecayParam,
        envSustainParam,
        envReleaseParam,
        lfoRateParam,
        vibratoParam,
        noiseParam,
        octaveParam,
        tuningParam,
        outputLevelParam,
        polyModeParam,
        stereoWidthParam
    };

    for (int i = 0; i < NUM_PARAMS; ++i)
    {
        const float currentValue = params[i]->convertFrom0to1(params[i]->getValue());
        const float presetValue = preset.param[i];

        // tolerancia pequeña para floats
        if (std::abs(currentValue - presetValue) > 0.001f)
            return false;
    }

    return true;
}

void AndesJXAudioProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return;

    loadingPreset = true;

    currentProgram = index;
	isCustomPreset = false;

    juce::RangedAudioParameter* params[NUM_PARAMS] =
    {
        osc1WaveParam,
        osc2WaveParam,
        oscMixParam,
        oscTuneParam,
        oscFineParam,
        glideModeParam,
        glideRateParam,
        glideBendParam,
        filterTypeParam,
        filterFreqParam,
        filterResoParam,
        filterEnvParam,
        filterLFOParam,
        filterVelocityParam,
        filterKeytrackParam,
        filterKeycenterParam,
        filterAttackParam,
        filterDecayParam,
        filterSustainParam,
        filterReleaseParam,
        envAttackParam,
        envDecayParam,
        envSustainParam,
        envReleaseParam,
        lfoRateParam,
        vibratoParam,
        noiseParam,
        octaveParam,
        tuningParam,
        outputLevelParam,
        polyModeParam,
        stereoWidthParam
    };

    const Preset& preset = presets[index];

    for (int i = 0; i < NUM_PARAMS; ++i) {
      params[i]->setValueNotifyingHost(params[i]->convertTo0to1(preset.param[i]));
    }

    loadingPreset = false;

    reset();
    sendChangeMessage();
}

const juce::String AndesJXAudioProcessor::getProgramName (int index)
{
    if (index >= 0 && index < static_cast<int>(presets.size()))
        return presets[index].name;

    return "Custom";
}

void AndesJXAudioProcessor::changeProgramName (int /*index*/, const juce::String& /*newName*/)
{
    // not implemented
}

//==============================================================================
void AndesJXAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
        spec.numChannels,
        1, // 2^2 = 4x oversampling (era 1 = 2x)
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR
    );

    oversampling->initProcessing(samplesPerBlock);

    synth.allocateResources(sampleRate*2, samplesPerBlock*2);
    parametersChanged.store(true);
    reset();
}

void AndesJXAudioProcessor::releaseResources()
{
    //synth.deallocateResources();
}

void AndesJXAudioProcessor::reset()
{
    synth.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AndesJXAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void AndesJXAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    bool expected = true;
    if (parametersChanged.compare_exchange_strong(expected, false)) {
        update();
    }

    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();

        if (msg.isController())
        {
            const int cc = msg.getControllerNumber();
            const float v = msg.getControllerValue() / 127.0f;

            switch (cc)
            {
            case 1:  ccModWheel.store(v, std::memory_order_relaxed); break;          // ModWheel
            case 11: ccExpression.store(v, std::memory_order_relaxed); break;        // Expression
            case 74: ccBrightness.store(v, std::memory_order_relaxed); break;        // Brightness
            case 71: ccResonance.store(v, std::memory_order_relaxed); break;         // Resonance
            case 73: ccAttack.store(v, std::memory_order_relaxed); break;            // Attack
            case 72: ccRelease.store(v, std::memory_order_relaxed); break;           // Release
            case 64: ccSustainDown.store(v >= 0.5f, std::memory_order_relaxed); break; // Sustain
            default: break;
            }
        }
    }
    Synth::CCState s;
    s.modWheel = ccModWheel.load(std::memory_order_relaxed);
    s.expression = ccExpression.load(std::memory_order_relaxed);
    s.brightness = ccBrightness.load(std::memory_order_relaxed);
    s.resonance = ccResonance.load(std::memory_order_relaxed);
    s.attack = ccAttack.load(std::memory_order_relaxed);
    s.release = ccRelease.load(std::memory_order_relaxed);
    s.sustain = ccSustainDown.load(std::memory_order_relaxed);

    synth.setCCState(s);
    
    // ===== OVERSAMPLING =====
    juce::dsp::AudioBlock<float> block(buffer);
    auto osBlock = oversampling->processSamplesUp(block);

    // Procesar directamente con el AudioBlock oversampleado
    splitBufferByEventsOptimized(osBlock, midiMessages);

    // Downsample
    oversampling->processSamplesDown(block);

    //splitBufferByEvents(buffer, midiMessages);
}

void AndesJXAudioProcessor::splitBufferByEventsOptimized(juce::dsp::AudioBlock<float>& block,
    juce::MidiBuffer& midiMessages)
{
    constexpr int osFactor = 2;
    int bufferOffset = 0;
    const int totalSamples = static_cast<int>(block.getNumSamples());

    for (const auto metadata : midiMessages)
    {
        const int eventPosOS = metadata.samplePosition * osFactor;

        // Renderizar audio antes del evento
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

        // Manejar evento MIDI
        if (metadata.numBytes <= 3)
        {
            uint8_t data1 = (metadata.numBytes >= 2) ? metadata.data[1] : 0;
            uint8_t data2 = (metadata.numBytes == 3) ? metadata.data[2] : 0;
            handleMIDI(metadata.data[0], data1, data2);
        }
    }

    // Renderizar después del último evento
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

void AndesJXAudioProcessor::update()
{
    const int w1 = osc1WaveParam->getIndex();
    const int w2 = osc2WaveParam->getIndex();

    // Mapeo seguro (por si cambias enum/orden)
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

    const int filterIndex = filterTypeParam->getIndex();

    auto toFilterType = [](int idx) -> Synth::FilterType
        {
            switch (idx)
            {
            case 0: return Synth::FilterType::SVF;
            case 1: return Synth::FilterType::Moog;
            default: return Synth::FilterType::SVF;
            }
        };

    synth.setFilterType(toFilterType(filterIndex));

    float sampleRate = float(getSampleRate())* 2.0f;
    float inverseSampleRate = 1.0f / sampleRate;
    //float decayTime = envDecayParam->get() / 100.0f * 5.0f;
    //float decaySamples = sampleRate * decayTime;

    synth.envAttack = std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * envAttackParam->get()));
    synth.envDecay =  std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * envDecayParam->get()));
    synth.envSustain = envSustainParam->get() / 100.0f;
    synth.numVoices = (polyModeParam->getIndex() == 0) ? 1 : MAX_VOICES;
    //synth.envDecay =  std::exp(std::log(SILENCE) / decaySamples);

    float envRelease = envReleaseParam->get();
    float r = juce::jmax(1.0f, envRelease); 
    synth.envRelease = std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * r));

    /*
    if (envRelease < 1.0f) {
        synth.envRelease = 0.75f;
    }
    else {
        synth.envRelease = std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * envRelease));
    }
    */

    synth.oscMix = oscMixParam->get() / 100.0f;

    float noiseMix = noiseParam->get() / 100.0f;
    noiseMix *= noiseMix;
    synth.noiseMix = noiseMix * 0.06f;
    
    float trim = 0.30f * (3.2f - synth.oscMix - 25.0f * synth.noiseMix);
    synth.volumeTrim = juce::jlimit(0.0f, 1.2f, trim);

    float semi = oscTuneParam->get();
    float cent = oscFineParam->get();
    synth.detune = std::exp2((semi + 0.01f * cent) / 12.0f);
    
    float octave = octaveParam->get();
    float tuning = tuningParam->get();
    synth.tune = octave * 12.0f + tuning / 100.0f;

    synth.stereoWidth = stereoWidthParam->get() / 100.0f;

    float gain = juce::Decibels::decibelsToGain(outputLevelParam->get());
    synth.outputLevelSmoother.setTargetValue(gain);
    //synth.outputLevel = juce::Decibels::decibelsToGain(outputLevelParam->get());
       
    const float filterVelocity = filterVelocityParam->get();

    // amplitud: independiente del knob de filtro
    synth.velocitySensitivity = 0.75f;
    synth.ignoreVelocity = false;

    // filtro: sí depende del knob filterVelocity
    if (filterVelocity < -90.0f)
        synth.filterVelocityAmount = 0.0f;
    else
        synth.filterVelocityAmount = juce::jlimit(0.0f, 1.0f, std::abs(filterVelocity) / 100.0f);

    //Actualizar LFO
    const float lfoNorm = lfoRateParam->get();          // 0..1
    const float lfoHz = std::exp(7.0f * lfoNorm - 4.0f);
    synth.setLfoRateHz(lfoHz);

    
    
    //GLIDE / PORTAMENTO

    const float inverseUpdateRate = 32.0f / sampleRate;
    synth.glideMode = glideModeParam->getIndex();
    float gr = glideRateParam->get();
    if (gr < 2.0f) {
        synth.glideRate = 1.0f; // no glide
    } else {
    synth.glideRate = 1.0f - std::exp(-inverseUpdateRate * std::exp(6.0f - 0.07f * gr));
    }
    synth.glideBend = glideBendParam->get();
      
        
    
    // Vibrato/PWM
    float vibratoValue = vibratoParam->get(); // Asume que va de -100 a +100

    // Calcular valores al cuadrado para vibrato en semitonos
    synth.lfoDepthSemis = vibratoValue * vibratoValue * 0.0002f; // vibrato
    //synth.pwmDepth = 0.0f;
    //synth.pwmDepth = std::abs(vibratoValue) * 0.01f;
    //synth.pwmDepth = vibratoValue * vibratoValue * 0.01f;      // PWM
    // Si el parámetro es negativo, usar PWM en lugar de vibrato
    if (vibratoValue < 0.0f) {
        synth.lfoDepthSemis = 0.0f;  // Apagar vibrato
        synth.pwmDepth = std::abs(vibratoValue) * 0.01f;
    }
    else
    {
        synth.pwmDepth = 0;
    }
    //synth.lfoDepthSemis = 0.02f * lfoDepthParam->get(); // 0..2 semitonos
    //synth.lfoDepthSemis = 2.0f; // medio semitono, solo para test

    // FILTRO - Convertir de % a valores reales
    //float filterFreqPercent = filterFreqParam->get();  // 0-100%
    float filterResoPercent = filterResoParam->get();  // 0-100%

    float filterLFO = filterLFOParam->get() / 100.0f;          // 0..1
    synth.filterLFODepthSemis = 2.5f * filterLFO * filterLFO;        // 0..2.5 (curva parabólica)

    // Mapeo exponencial para frecuencia (más control en graves)
    //synth.filterCutoff = 80.0f * std::exp(6.0f * filterFreqPercent / 100.0f);  // 80Hz - 32kHz
    //synth.filterCutoff = juce::jlimit(80.0f, 20000.0f, synth.filterCutoff);
    const float x = juce::jlimit(0.0f, 1.0f, filterFreqParam->get() / 100.0f);
    const float minHz = 80.0f;
    const float maxHz = 20000.0f;
    synth.filterCutoff = minHz * std::exp(std::log(maxHz / minHz) * x);

    // Mapeo lineal para resonancia
    //synth.filterResonance = 0.5f + 4.5f * (filterResoPercent / 100.0f);  // 0.5 -
    float x1 = juce::jlimit(0.0f, 1.0f, filterResoPercent / 100.0f);
    synth.filterResonance = x1 * x1;

    // KEY TRACKING
    synth.filterKeytrackAmount = filterKeytrackParam->get() / 100.0f;  // Convertir % a 0-1.0
    synth.filterKeycenterNote = (int)filterKeycenterParam->get();

    const float envPct = juce::jlimit(-100.0f, 100.0f, filterEnvParam->get());
    synth.filterEnvAmountSemis = 48.0f * (envPct / 100.0f);

    synth.filterEnvAttack = std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * filterAttackParam->get()));
    synth.filterEnvDecay = std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * filterDecayParam->get()));
    synth.filterEnvSustain = filterSustainParam->get() / 100.0f;
    float fr = filterReleaseParam->get();
    float frSafe = juce::jmax(1.0f, fr);
    synth.filterEnvRelease = std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * frSafe));
    /*
    float fr = filterReleaseParam->get();
    if (fr < 1.0f) synth.filterEnvRelease = 0.75f;
    else          synth.filterEnvRelease = std::exp(-inverseSampleRate * std::exp(5.5f - 0.075f * fr));
    */
    // Actualizar tipo de filtro
    //const int filterTypeIndex = filterTypeParam->getIndex();
    //synth.setFilterType(static_cast<Synth::FilterType>(filterTypeIndex));
}

void AndesJXAudioProcessor::splitBufferByEvents(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    constexpr int osFactor = 2; // 4x oversampling

    int bufferOffset = 0;

    // Loop through the MIDI messages, which are sorted by samplePosition.
    for (const auto metadata : midiMessages) {

        const int eventPosOS = metadata.samplePosition * osFactor;

        // Render the audio that happens before this event (if any).
        int samplesThisSegment = eventPosOS - bufferOffset;
        if (samplesThisSegment > 0) {
            render(buffer, samplesThisSegment, bufferOffset);
            bufferOffset += samplesThisSegment;
        }

        // Handle the event. Ignore MIDI messages such as sysex.
        if (metadata.numBytes <= 3) {
            uint8_t data1 = (metadata.numBytes >= 2) ? metadata.data[1] : 0;
            uint8_t data2 = (metadata.numBytes == 3) ? metadata.data[2] : 0;
            handleMIDI(metadata.data[0], data1, data2);
        }
    }

    // Render the audio after the last MIDI event. If there were no
    // MIDI events at all, this renders the entire buffer.
    int samplesLastSegment = buffer.getNumSamples() - bufferOffset;
    if (samplesLastSegment > 0) {
        render(buffer, samplesLastSegment, bufferOffset);
    }

    midiMessages.clear();
}

void AndesJXAudioProcessor::handleMIDI(uint8_t data0, uint8_t data1, uint8_t data2)
{
    // Program Change
    if ((data0 & 0xF0) == 0xC0) {
        if (data1 < presets.size()) {
            setCurrentProgram(data1);
        }
    }

    synth.midiMessage(data0, data1, data2);
}

void AndesJXAudioProcessor::render(juce::AudioBuffer<float>& buffer, int sampleCount, int bufferOffset)
{
    float* outputBuffers[2] = { nullptr, nullptr };
    outputBuffers[0] = buffer.getWritePointer(0) + bufferOffset;
    if (getTotalNumOutputChannels() > 1) {
        outputBuffers[1] = buffer.getWritePointer(1) + bufferOffset;
    }

    synth.render(outputBuffers, sampleCount);
}

//==============================================================================
bool AndesJXAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AndesJXAudioProcessor::createEditor()
{
    /*
    auto editor = new juce::GenericAudioProcessorEditor(*this);
    editor->setSize(500, 1050);
    return editor;
    */
    return new AndesJXAudioProcessorEditor(*this);
}

//==============================================================================
void AndesJXAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    apvts.state.setProperty("currentProgram", currentProgram, nullptr);
    apvts.state.setProperty("isCustomPreset", isCustomPreset, nullptr);

    copyXmlToBinary(*apvts.copyState().createXml(), destData);
}

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

//==============================================================================
void AndesJXAudioProcessor::createPrograms()
{
    //INIT
    presets.emplace_back("Init",
        1.00f, 1.00f, 0.00f, -12.00f, 0.00f, 0.00f, 35.00f,
        0.00f, 1.00f, 100.00f, 15.00f, 50.00f, 0.00f,
        0.00f, 100.00f, 60.00f, 0.00f, 30.00f,
        0.00f, 25.00f, 0.00f, 50.00f, 100.00f,
        30.00f, 0.81f, 0.00f, 0.00f, 0.00f,
        0.00f, -3.00f, 1.00f, 0.00f);

    //BASSES
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
	
    // PADS
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

    // LEADS
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

    // BRASS / WINDS / ORGAN
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

    // KEYS / PLUCK / FX
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

juce::AudioProcessorValueTreeState::ParameterLayout AndesJXAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParameterID::osc1Wave,
        "Osc1 Wave",
        juce::StringArray{ "Sine", "Saw", "Square", "Triangle", "PWM" },
        1 // default Saw (índice 1)
    ));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParameterID::osc2Wave,
        "Osc2 Wave",
        juce::StringArray{ "Sine", "Saw", "Square", "Triangle", "PWM" },
        1
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::stereoWidth,
        "Stereo Width",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        50.0f, // Valor por defecto (totalmente)
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParameterID::polyMode,
        "Polyphony",
        juce::StringArray { "Mono", "Poly" },
        1));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::oscTune,
        "Osc Tune",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 1.0f),
        -12.0f,
        juce::AudioParameterFloatAttributes().withLabel("semi")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::oscFine,
        "Osc Fine",
        juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f, 0.3f, true),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("cent")));

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
        0.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(oscMixStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParameterID::glideMode,
        "Glide Mode",
        juce::StringArray { "Off", "Legato", "Always" },
        0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::glideRate,
        "Glide Rate",
        juce::NormalisableRange<float>(0.0f, 100.f, 1.0f),
        35.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::glideBend,
        "Glide Bend",
        juce::NormalisableRange<float>(-36.0f, 36.0f, 0.01f, 0.4f, true),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("semi")));
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ParameterID::filterType,
        "Filter Type",
        juce::StringArray{ "SVF", "Moog" },  // Opciones
        0));  // Default: SVF (índice 0)

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterFreq,
        "Filter Freq",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterReso,
        "Filter Reso",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        15.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

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

    // KEY TRACKING AMOUNT (0-200%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterKeytrack,
        "Filter Keytrack",
        juce::NormalisableRange<float>(0.0f, 200.0f, 1.0f),
        100.0f,  // 100% por defecto (tracking completo)
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // KEY TRACKING CENTER NOTE (C0 - C8)
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
        juce::NormalisableRange<float>(24.0f, 96.0f, 1.0f),  // C1 a C7
        60.0f,  // C4 (middle C) por defecto
        juce::AudioParameterFloatAttributes()
        .withLabel("")
        .withStringFromValueFunction(keycenterStringFromValue)));

    auto envStageStringFromValue = [](float value, int)
        {
            return juce::String(juce::roundToInt(value));
        };

    auto sustainStringFromValue = [](float value, int)
        {
            return juce::String(juce::roundToInt(value));
        };

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterAttack,
        "Filter Attack",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("")
        .withStringFromValueFunction(envStageStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterDecay,
        "Filter Decay",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        30.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("")
        .withStringFromValueFunction(envStageStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterSustain,
        "Filter Sustain",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("")
        .withStringFromValueFunction(sustainStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::filterRelease,
        "Filter Release",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        25.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("")
        .withStringFromValueFunction(envStageStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::envAttack,
        "Env Attack",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("")
        .withStringFromValueFunction(envStageStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::envDecay,
        "Env Decay",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        50.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("")
        .withStringFromValueFunction(envStageStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::envSustain,
        "Env Sustain",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        100.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("")
        .withStringFromValueFunction(sustainStringFromValue)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterID::envRelease,
        "Env Release",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        30.0f,
        juce::AudioParameterFloatAttributes()
        .withLabel("")
        .withStringFromValueFunction(envStageStringFromValue)));

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
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AndesJXAudioProcessor();
}
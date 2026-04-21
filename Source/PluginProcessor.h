/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Synth.h"
#include "Preset.h"
#include <juce_dsp/juce_dsp.h>

#define PARAMETER_ID(str) inline const juce::ParameterID str { #str, 1 };
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
/**
*/
class AndesJXAudioProcessor : public juce::AudioProcessor,
                              private juce::ValueTree::Listener,
                              public juce::ChangeBroadcaster
{
public:
    //==============================================================================
    AndesJXAudioProcessor();
    ~AndesJXAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    bool isCustomPresetActive() const noexcept { return isCustomPreset; }
    bool isLoadingPreset() const noexcept { return loadingPreset; }

    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters", createParameterLayout() };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) override
    {
        static const juce::Identifier currentProgramID{ "currentProgram" };
        static const juce::Identifier isCustomPresetID{ "isCustomPreset" };

        parametersChanged.store(true);

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

    std::atomic<bool> parametersChanged { false };

    void update();
    void createPrograms();

    void splitBufferByEvents(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void handleMIDI(uint8_t data0, uint8_t data1, uint8_t data2);
    void render(juce::AudioBuffer<float>& buffer, int sampleCount, int bufferOffset);
    void splitBufferByEventsOptimized(juce::dsp::AudioBlock<float>& block, juce::MidiBuffer& midiMessages);

    std::vector<Preset> presets;
    int currentProgram = 0;
    bool isCustomPreset = false;
    bool loadingPreset = false;
    bool currentStateMatchesProgram() const;

    Synth synth;

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

    std::atomic<float> ccModWheel{ 0.0f };     // CC1   [0..1]
    std::atomic<float> ccExpression{ 1.0f };   // CC11  [0..1]  (1 = unity)
    std::atomic<float> ccBrightness{ 0.0f };   // CC74  [0..1]
    std::atomic<float> ccResonance{ 0.0f };    // CC71  [0..1]
    std::atomic<float> ccAttack{ 0.0f };       // CC73  [0..1]
    std::atomic<float> ccRelease{ 0.0f };      // CC72  [0..1]
    std::atomic<bool>  ccSustainDown{ false }; // CC64

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndesJXAudioProcessor)
};
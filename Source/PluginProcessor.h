/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Synth.h"
//==============================================================================
/**
*/
class AndesJXAudioProcessor  : public juce::AudioProcessor
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

private:

    Synth synth; // crete a new object for synth to use thir properties.

    // Splits the audio buffer into smaller segments based on MIDI events
    // occurring at different times.
    void splitBufferByEvents(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    // Processes individual MIDI messages by interpreting the raw MIDI protocol bytes
    void handleMIDI(uint8_t data0, uint8_t data1, uint8_t data2);
    // Generates the synth's audio samples for a specific buffer segment
    // sampleCount = how many samples to generate
    // bufferOffset = position in the buffer where to start writing
    void render(juce::AudioBuffer<float>& buffer, int sampleCount, int bufferOffset);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndesJXAudioProcessor)
};

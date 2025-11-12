/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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
}

AndesJXAudioProcessor::~AndesJXAudioProcessor()
{
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
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AndesJXAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AndesJXAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AndesJXAudioProcessor::getProgramName (int index)
{
    return {};
}

void AndesJXAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AndesJXAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.allocateResources(sampleRate, samplesPerBlock);
    reset();
}

void AndesJXAudioProcessor::releaseResources()
{
    synth.deallocateResources();
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
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    splitBufferByEvents(buffer, midiMessages);
}

//This function (splitBufferByEvents) ensures sample-accurate timing by rendering audio in segments, 
//stopping precisely at each MIDI event to update the synth's state before continuing.
void AndesJXAudioProcessor::splitBufferByEvents(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    int bufferOffset = 0;
    for (const auto metadata : midiMessages) {
        // Render the audio that happens before this event (if any).
        int samplesThisSegment = metadata.samplePosition - bufferOffset;
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
    //To print the MIDI message.
    //It work better using standalone app in debug mode.
    //char s[16];
    //snprintf(s, 16, "%02hhX %02hhX %02hhX", data0, data1, data2);
    //DBG(s);
    synth.midiMessage(data0, data1, data2);
}

void AndesJXAudioProcessor::render(
    juce::AudioBuffer<float>& buffer, int sampleCount, int bufferOffset)
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
    return new AndesJXAudioProcessorEditor (*this);
}

//==============================================================================
void AndesJXAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AndesJXAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AndesJXAudioProcessor();
}

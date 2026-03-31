/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LookAndFeel/ComboBoxLookAndFeel.h"
#include "LookAndFeel/ToggleLookAndFeel.h"

//==============================================================================
/**
*/
class AndesJXAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                     private juce::AudioProcessorValueTreeState::Listener
{
public:
    AndesJXAudioProcessorEditor (AndesJXAudioProcessor&);
    ~AndesJXAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AndesJXAudioProcessor& audioProcessor;

    juce::Image backgroundAndesJX;

    // Combos
    juce::ComboBox oscWaveSelector;
    juce::ComboBox osc2WaveSelector;
    juce::ComboBox filterKeycenterSelector; // new Filter Keycenter combo
    juce::ComboBox presetSelector;

    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<ComboBoxAttachment> oscWaveAttachment;
    std::unique_ptr<ComboBoxAttachment> osc2WaveAttachment;
    std::unique_ptr<ComboBoxAttachment> filterKeycenterAttachment; // attachment for new combo

    std::unique_ptr<ComboBoxLookAndFeel> comboBoxLookAndFeel;

    // Poly toggle + lookandfeel
    juce::ToggleButton polyToggle;
    std::unique_ptr<ToggleLookAndFeel> toggleLookAndFeel;

    // AudioProcessorValueTreeState listener
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndesJXAudioProcessorEditor)
};

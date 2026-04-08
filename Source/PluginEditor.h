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
#include "SegmentedControl.h"
#include "LookAndFeel/SegmentedButtonLookAndFeel.h"
#include "LookAndFeel/KnobPrincipalLookAndFeel.h"
#include "LookAndFeel/SecondaryKnobLookAndFeel.h"
#include "LookAndFeel/FaderLookAndFeel.h"

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
    juce::ComboBox presetSelector;
	juce::ComboBox glideModeSelector;

    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<ComboBoxAttachment> oscWaveAttachment;
    std::unique_ptr<ComboBoxAttachment> osc2WaveAttachment;
    std::unique_ptr<ComboBoxAttachment> glideModeAttachment;

    std::unique_ptr<ComboBoxLookAndFeel> comboBoxLookAndFeel;
    
    juce::Slider mixSlider;
    juce::Slider resonanceSlider;
    juce::Slider cutoffSlider;
    juce::Slider outputSlider;

    juce::Slider oscTuneSlider;
    juce::Slider stereoWidthSlider;
    juce::Slider noiseSlider;
    juce::Slider oscFineSlider;
    juce::Slider octaveSlider;
    juce::Slider tuningSlider;
    juce::Slider glideBendSlider;
    juce::Slider glideRateSlider;
    juce::Slider vibratoSlider;
	juce::Slider filterVelocitySlider;
    juce::Slider filterEnvSlider;
	juce::Slider filterLFOSlider;
    juce::Slider filterKeycenterSlider;
	juce::Slider filterKeytrackSlider;
    juce::Slider lfoRateSlider;

    juce::Slider ampAttackSlider;
    juce::Slider ampDecaySlider;
    juce::Slider ampSustainSlider;
    juce::Slider ampReleaseSlider;

    juce::Slider filterAttackSlider;
    juce::Slider filterDecaySlider;
    juce::Slider filterSustainSlider;
    juce::Slider filterReleaseSlider;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<SliderAttachment> resonanceAttachment;
    std::unique_ptr<SliderAttachment> cutoffAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;

    std::unique_ptr<SliderAttachment> oscTuneAttachment;
    std::unique_ptr<SliderAttachment> stereoWidthAttachment;
    std::unique_ptr<SliderAttachment> noiseAttachment;
    std::unique_ptr<SliderAttachment> oscFineAttachment;
    std::unique_ptr<SliderAttachment> octaveAttachment;
    std::unique_ptr<SliderAttachment> tuningAttachment;
    std::unique_ptr<SliderAttachment> glideRateAttachment;
    std::unique_ptr<SliderAttachment> glideBendAttachment;
    std::unique_ptr<SliderAttachment> vibratoAttachment;
	std::unique_ptr<SliderAttachment> filterVelocityAttachment;
    std::unique_ptr<SliderAttachment> filterEnvAttachment;
	std::unique_ptr<SliderAttachment> filterLFOAttachment;
    std::unique_ptr<SliderAttachment> filterKeycenterAttachment;
	std::unique_ptr<SliderAttachment> filterKeytrackAttachment;
    std::unique_ptr<SliderAttachment> lfoRateAttachment;

    std::unique_ptr<SliderAttachment> ampAttackAttachment;
    std::unique_ptr<SliderAttachment> ampDecayAttachment;
    std::unique_ptr<SliderAttachment> ampSustainAttachment;
    std::unique_ptr<SliderAttachment> ampReleaseAttachment;

    std::unique_ptr<SliderAttachment> filterAttackAttachment;
    std::unique_ptr<SliderAttachment> filterDecayAttachment;
    std::unique_ptr<SliderAttachment> filterSustainAttachment;
    std::unique_ptr<SliderAttachment> filterReleaseAttachment;

    std::unique_ptr<KnobPrincipalLookAndFeel> knobPrincipalLookAndFeel;

    std::unique_ptr<SecondaryKnobLookAndFeel> secondaryKnobLookAndFeel;

    std::unique_ptr<FaderLookAndFeel> faderLookAndFeel;


    // Poly toggle + lookandfeel
    juce::ToggleButton polyToggle;
    std::unique_ptr<ToggleLookAndFeel> toggleLookAndFeel;

    //SegmentedControl filterTypeControl;
    std::unique_ptr<SegmentedButtonLookAndFeel> segmentedButtonLookAndFeel;  
    SegmentedControl filterTypeControl;
    
    // AudioProcessorValueTreeState listener
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
    void initialiseBackground();
    void initialiseLookAndFeels();

    void setupFader(juce::Slider& slider);
    void initialiseAmpEnvelopeControls();
    void initialiseFilterEnvelopeADSRControls();

    void setupCombo(juce::ComboBox& combo);
    void setupKnob(juce::Slider& slider);
    void setupSecondaryKnob(juce::Slider& slider);

    void initialiseOscWaveSelectors();
    void initialiseKnobs();
    void initialiseOscTuneControl();
    void initialiseStereoWidthControl();
    void initialiseNoiseControl();
    void initialiseOscFineControl();
    void initialiseOctaveControl();
    void initialiseTuningControl();
    void initialiseGlideRateControl();
    void initialiseGlideBendControl();
    void initialiseVibratoControl();
	void initialiseFilterVelocityControl();
    void initialiseFilterEnvControl();
	void initialiseFilterLFOControl();
    void initialiseFilterKeycenterControl();
	void initialiseFilterKeytrackControl();
    void initialiseLFORateControl();
    void initialisePresetSelector();
    void initialiseGlideModeSelector();
    void initialisePolyToggle();
    void initialiseFilterTypeControl();
    void initialiseAttachments();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndesJXAudioProcessorEditor)
};

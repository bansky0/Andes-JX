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
                                     private juce::AudioProcessorValueTreeState::Listener,
                                     private juce::ChangeListener
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
    juce::Label oscTuneValueLabel;
    juce::Slider stereoWidthSlider;
    juce::Label stereoWidthValueLabel;
    juce::Slider noiseSlider;
    juce::Label noiseValueLabel;
    juce::Slider oscFineSlider;
    juce::Label oscFineValueLabel;
    juce::Slider octaveSlider;
    juce::Label octaveValueLabel;
    juce::Slider tuningSlider;
    juce::Label tuningValueLabel;
    juce::Slider glideBendSlider;
    juce::Label glideBendValueLabel;
    juce::Slider glideRateSlider;
    juce::Label glideRateValueLabel;
    juce::Slider vibratoSlider;
    juce::Label vibratoValueLabel;
	juce::Slider filterVelocitySlider;
    juce::Label filterVelocityValueLabel;
    juce::Slider filterEnvSlider;
    juce::Label filterEnvValueLabel;
	juce::Slider filterLFOSlider;
    juce::Label filterLFOValueLabel;
    juce::Slider filterKeycenterSlider;
    juce::Label filterKeycenterValueLabel;
	juce::Slider filterKeytrackSlider;
    juce::Label filterKeytrackValueLabel;
    juce::Slider lfoRateSlider;
    juce::Label lfoRateValueLabel;

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
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

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

    juce::String formatLFORateValue(double value) const;
    void updateLFORateValueLabel();

    juce::String formatFilterLFOValue(double value) const;
    void updateFilterLFOValueLabel();
    juce::String formatFilterEnvValue(double value) const;
    void updateFilterEnvValueLabel();
    juce::String formatFilterKeycenterValue(double value) const;
    void updateFilterKeycenterValueLabel();
    juce::String formatFilterKeytrackValue(double value) const;
    void updateFilterKeytrackValueLabel();

    juce::String formatFilterVelocityValue(double value) const;
    void updateFilterVelocityValueLabel();
    juce::String formatVibratoValue(double value) const;
    void updateVibratoValueLabel();

    juce::String formatGlideBendValue(double value) const;
    void updateGlideBendValueLabel();
    juce::String formatGlideRateValue(double value) const;
    void updateGlideRateValueLabel();

    juce::String formatTuningValue(double value) const;
    void updateTuningValueLabel();
    juce::String formatOctaveValue(double value) const;
    void updateOctaveValueLabel();
    juce::String formatNoiseValue(double value) const;
    void updateNoiseValueLabel();
    juce::String formatStereoWidthValue(double value) const;
    void updateStereoWidthValueLabel();
    juce::String formatOscTuneValue(double value) const;
    void updateOscTuneValueLabel();
    juce::String formatOscFineValue(double value) const;
    void updateOscFineValueLabel();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndesJXAudioProcessorEditor)
};

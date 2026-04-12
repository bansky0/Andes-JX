/*
  ==============================================================================
 
    This file contains the basic framework code for a JUCE plugin editor.
 
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr float kTitleTracking = 0.08f;
    constexpr float kLabelTracking = 0.06f;
    constexpr float kTinyLabelTracking = 0.04f;

    void drawTextLabel(juce::Graphics& g,
        juce::Rectangle<int> area,
        const juce::String& text,
        float fontHeight,
        float alpha,
        float kerning,
        juce::Justification justification)
    {
        juce::Font font(juce::FontOptions{ fontHeight });
        font.setTypefaceName("Helvetica, Arial, sans-serif");
        font.setBold(false);
        font = font.withExtraKerningFactor(kerning);

        g.setFont(font);
        g.setColour(AndesTheme::Colours::text.withAlpha(alpha));
        g.drawFittedText(text, area, justification, 1);
    }

    void drawSectionTitle(juce::Graphics& g,
        juce::Rectangle<int> area,
        const juce::String& title)
    {
        drawTextLabel(g, area, title, 11.0f, 0.6f, kTitleTracking,
            juce::Justification::centredLeft);
    }

    void drawControlLabel(juce::Graphics& g,
        juce::Rectangle<int> area,
        const juce::String& text)
    {
        drawTextLabel(g, area, text, 7.5f, 0.8f, kLabelTracking,
            juce::Justification::centred);
    }

    void drawTinyControlLabel(juce::Graphics& g,
        juce::Rectangle<int> area,
        const juce::String& text)
    {
        drawTextLabel(g, area, text, 7.0f, 0.8f, kTinyLabelTracking,
            juce::Justification::centred);
    }
}

//==============================================================================
AndesJXAudioProcessorEditor::AndesJXAudioProcessorEditor(AndesJXAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    initialiseBackground();
    initialiseLookAndFeels();

    initialiseOscWaveSelectors();
    initialiseKnobs();
    initialiseAmpEnvelopeControls();
    initialiseFilterEnvelopeADSRControls();
    initialiseOscTuneControl();
    initialiseStereoWidthControl();
    initialiseNoiseControl();
    initialiseOscFineControl();
    initialiseOctaveControl();
    initialiseTuningControl();
    initialiseGlideRateControl();
    initialiseGlideBendControl();
    initialiseVibratoControl();
    initialiseFilterVelocityControl();
    initialiseFilterEnvControl();
    initialiseFilterLFOControl();
    initialiseFilterKeytrackControl();
    initialiseLFORateControl();
    initialiseFilterKeycenterControl();
    initialisePresetSelector();
    initialiseGlideModeSelector();
    initialisePolyToggle();
    initialiseFilterTypeControl();
    initialiseAttachments();

    updateLFORateValueLabel();
    updateFilterLFOValueLabel();

    updateFilterEnvValueLabel();
    updateFilterKeycenterValueLabel();
    updateFilterKeytrackValueLabel();

    updateFilterVelocityValueLabel();
    updateVibratoValueLabel();
    updateGlideBendValueLabel();
    updateGlideRateValueLabel();

    updateOscTuneValueLabel();
    updateStereoWidthValueLabel();
    updateNoiseValueLabel();
    updateOscFineValueLabel();
    updateOctaveValueLabel();
    updateTuningValueLabel();
}

void AndesJXAudioProcessorEditor::initialiseBackground()
{
    backgroundAndesJX = juce::ImageCache::getFromMemory(BinaryData::backgroundAndesJX_png, BinaryData::backgroundAndesJX_pngSize);

    if (backgroundAndesJX.isValid())
    {
        backgroundAndesJX = backgroundAndesJX.rescaled(
            backgroundAndesJX.getWidth() / 4,
            backgroundAndesJX.getHeight() / 4,
            juce::Graphics::highResamplingQuality
        );

        setSize(backgroundAndesJX.getWidth(), backgroundAndesJX.getHeight());
    }
    else
    {
        setSize(500, 430);
    }
}

void AndesJXAudioProcessorEditor::initialiseLookAndFeels()
{
    comboBoxLookAndFeel = std::make_unique<ComboBoxLookAndFeel>();

    knobPrincipalLookAndFeel = std::make_unique<KnobPrincipalLookAndFeel>();
    knobPrincipalLookAndFeel->setMainValueFontHeight(10.0f);
    knobPrincipalLookAndFeel->setUnitFontHeight(7.5f);
    knobPrincipalLookAndFeel->setSingleLineFontHeight(8.5f);

    secondaryKnobLookAndFeel = std::make_unique<SecondaryKnobLookAndFeel>();
    secondaryKnobLookAndFeel->setTextFontHeight(7.5f);
    secondaryKnobLookAndFeel->setShowValueText(false);
    secondaryKnobLookAndFeel->setTextInset(3);

    faderLookAndFeel = std::make_unique<FaderLookAndFeel>();

    toggleLookAndFeel = std::make_unique<ToggleLookAndFeel>();
    toggleLookAndFeel->setColour(ToggleLookAndFeel::backgroundOnColourId, AndesTheme::Colours::panel);
    toggleLookAndFeel->setColour(ToggleLookAndFeel::backgroundOffColourId, AndesTheme::Colours::panelDark);
    toggleLookAndFeel->setColour(ToggleLookAndFeel::textColourId, AndesTheme::Colours::text);
    toggleLookAndFeel->setColour(ToggleLookAndFeel::outlineColourId, AndesTheme::Colours::outline);
    toggleLookAndFeel->setToggleFontHeight(10.0f);

    segmentedButtonLookAndFeel = std::make_unique<SegmentedButtonLookAndFeel>();
    segmentedButtonLookAndFeel->setDefaultBackground(AndesTheme::Colours::panel);
    segmentedButtonLookAndFeel->setDefaultText(AndesTheme::Colours::text);
    segmentedButtonLookAndFeel->setFontHeight(10.0f);
    segmentedButtonLookAndFeel->setCornerRadius(2.0f);
}


void AndesJXAudioProcessorEditor::setupCombo(juce::ComboBox& combo)
{
    combo.setLookAndFeel(comboBoxLookAndFeel.get());
    combo.setColour(juce::ComboBox::backgroundColourId, AndesTheme::Colours::panel);
    combo.setColour(juce::ComboBox::textColourId, AndesTheme::Colours::text);
    addAndMakeVisible(combo);
}


void AndesJXAudioProcessorEditor::setupKnob(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setLookAndFeel(knobPrincipalLookAndFeel.get());
    addAndMakeVisible(slider);
}

juce::String AndesJXAudioProcessorEditor::formatOscTuneValue(double value) const
{
    const int semitones = juce::roundToInt(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (semitones == 0)
        return nbsp + nbsp + "0 st";

    if (semitones > 0)
    {
        if (semitones < 10)
            return nbsp + "+" + juce::String(semitones) + " st";

        return "+" + juce::String(semitones) + " st";
    }

    if (std::abs(semitones) < 10)
        return nbsp + juce::String(semitones) + " st";

    return juce::String(semitones) + " st";
}

void AndesJXAudioProcessorEditor::updateOscTuneValueLabel()
{
    oscTuneValueLabel.setText(formatOscTuneValue(oscTuneSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseOscTuneControl()
{
    setupSecondaryKnob(oscTuneSlider);
    oscTuneSlider.setComponentID("oscTune");

    oscTuneValueLabel.setJustificationType(juce::Justification::centred);
    oscTuneValueLabel.setColour(juce::Label::textColourId, AndesTheme::Colours::text);
    oscTuneValueLabel.setFont(juce::Font(juce::FontOptions(7.5f)));
    oscTuneValueLabel.setInterceptsMouseClicks(false, false);

    updateOscTuneValueLabel();

    oscTuneSlider.onValueChange = [this]()
        {
            updateOscTuneValueLabel();
        };

    addAndMakeVisible(oscTuneValueLabel);
}

juce::String AndesJXAudioProcessorEditor::formatStereoWidthValue(double value) const
{
    const int width = juce::roundToInt(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (width == 0)
        return nbsp + nbsp + "0 %";

    if (width < 10)
        return nbsp + nbsp + juce::String(width) + " %";

    if (width < 100)
        return nbsp + juce::String(width) + " %";

    return juce::String(width) + " %";
}

void AndesJXAudioProcessorEditor::updateStereoWidthValueLabel()
{
    stereoWidthValueLabel.setText(formatStereoWidthValue(stereoWidthSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseStereoWidthControl()
{
    setupSecondaryKnob(stereoWidthSlider);
    stereoWidthSlider.setComponentID("stereoWidth");

    stereoWidthValueLabel.setJustificationType(juce::Justification::centred);
    stereoWidthValueLabel.setColour(juce::Label::textColourId, AndesTheme::Colours::text);
    stereoWidthValueLabel.setFont(juce::Font(juce::FontOptions(7.5f)));
    stereoWidthValueLabel.setInterceptsMouseClicks(false, false);

    updateStereoWidthValueLabel();

    stereoWidthSlider.onValueChange = [this]()
        {
            updateStereoWidthValueLabel();
        };

    addAndMakeVisible(stereoWidthValueLabel);
}

juce::String AndesJXAudioProcessorEditor::formatNoiseValue(double value) const
{
    const int noise = juce::roundToInt(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (noise == 0)
        return nbsp + nbsp + "0 %";

    if (noise < 10)
        return nbsp + nbsp + juce::String(noise) + " %";

    if (noise < 100)
        return nbsp + juce::String(noise) + " %";

    return juce::String(noise) + " %";
}

void AndesJXAudioProcessorEditor::updateNoiseValueLabel()
{
    noiseValueLabel.setText(formatNoiseValue(noiseSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseNoiseControl()
{
    setupSecondaryKnob(noiseSlider);
    noiseSlider.setComponentID("noise");

    noiseValueLabel.setJustificationType(juce::Justification::centred);
    noiseValueLabel.setColour(juce::Label::textColourId, AndesTheme::Colours::text);
    noiseValueLabel.setFont(juce::Font(juce::FontOptions(7.5f)));
    noiseValueLabel.setInterceptsMouseClicks(false, false);

    updateNoiseValueLabel();

    noiseSlider.onValueChange = [this]()
        {
            updateNoiseValueLabel();
        };

    addAndMakeVisible(noiseValueLabel);
}

juce::String AndesJXAudioProcessorEditor::formatOscFineValue(double value) const
{
    const float cents = static_cast<float>(value);

    const juce::String nbsp = juce::String::charToString(0xA0);

    if (std::abs(cents) < 0.05f)
        return nbsp + nbsp + nbsp + "0.0 c";

    if (cents > 0.0f)
    {
        if (cents < 10.0f)
            return nbsp + nbsp + "+" + juce::String(cents, 1) + " c";

        return "+" + juce::String(cents, 1) + " c";
    }

    if (std::abs(cents) < 10.0f)
        return nbsp + nbsp + juce::String(cents, 1) + " c";

    return juce::String(cents, 1) + " c";
}

void AndesJXAudioProcessorEditor::updateOscFineValueLabel()
{
    oscFineValueLabel.setText(formatOscFineValue(oscFineSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseOscFineControl()
{
    setupSecondaryKnob(oscFineSlider);
    oscFineSlider.setComponentID("oscFine");

    oscFineValueLabel.setJustificationType(juce::Justification::centred);
    oscFineValueLabel.setColour(juce::Label::textColourId, AndesTheme::Colours::text);
    oscFineValueLabel.setFont(juce::Font(juce::FontOptions(7.5f)));
    oscFineValueLabel.setInterceptsMouseClicks(false, false);

    updateOscFineValueLabel();

    oscFineSlider.onValueChange = [this]()
        {
            updateOscFineValueLabel();
        };

    addAndMakeVisible(oscFineValueLabel);
}

juce::String AndesJXAudioProcessorEditor::formatOctaveValue(double value) const
{
    const int oct = juce::roundToInt(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (oct == 0)
        return nbsp + nbsp + "0 oct";

    if (oct > 0)
        return nbsp + "+" + juce::String(oct) + " oct";

    return nbsp + juce::String(oct) + " oct";
}

void AndesJXAudioProcessorEditor::updateOctaveValueLabel()
{
    octaveValueLabel.setText(formatOctaveValue(octaveSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseOctaveControl()
{
    setupSecondaryKnob(octaveSlider);
    octaveSlider.setComponentID("octave");

    octaveValueLabel.setJustificationType(juce::Justification::centred);
    octaveValueLabel.setColour(juce::Label::textColourId, AndesTheme::Colours::text);
    octaveValueLabel.setFont(juce::Font(juce::FontOptions(7.5f)));
    octaveValueLabel.setInterceptsMouseClicks(false, false);

    updateOctaveValueLabel();

    octaveSlider.onValueChange = [this]()
        {
            updateOctaveValueLabel();
        };

    addAndMakeVisible(octaveValueLabel);
}

juce::String AndesJXAudioProcessorEditor::formatTuningValue(double value) const
{
    const float cents = static_cast<float>(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (std::abs(cents) < 0.05f)
        return nbsp + nbsp + nbsp + "0.0 c";

    if (cents > 0.0f)
    {
        if (cents < 10.0f)
            return nbsp + "+" + juce::String(cents, 1) + " c";

        return "+" + juce::String(cents, 1) + " c";
    }

    if (std::abs(cents) < 10.0f)
        return nbsp + juce::String(cents, 1) + " c";

    return juce::String(cents, 1) + " c";
}

void AndesJXAudioProcessorEditor::updateTuningValueLabel()
{
    tuningValueLabel.setText(formatTuningValue(tuningSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseTuningControl()
{
    setupSecondaryKnob(tuningSlider);
    tuningSlider.setComponentID("tuning");

    tuningValueLabel.setJustificationType(juce::Justification::centred);
    tuningValueLabel.setColour(juce::Label::textColourId, AndesTheme::Colours::text);
    tuningValueLabel.setFont(juce::Font(juce::FontOptions(7.5f)));
    tuningValueLabel.setInterceptsMouseClicks(false, false);

    updateTuningValueLabel();

    tuningSlider.onValueChange = [this]()
        {
            updateTuningValueLabel();
        };

    addAndMakeVisible(tuningValueLabel);
}

juce::String AndesJXAudioProcessorEditor::formatGlideRateValue(double value) const
{
    const int rate = juce::roundToInt(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (rate == 0)
        return nbsp + nbsp + "0 %";

    if (rate < 10)
        return nbsp + nbsp + juce::String(rate) + " %";

    if (rate < 100)
        return nbsp + juce::String(rate) + " %";

    return juce::String(rate) + " %";
}

void AndesJXAudioProcessorEditor::updateGlideRateValueLabel()
{
    glideRateValueLabel.setText(formatGlideRateValue(glideRateSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseGlideRateControl()
{
    setupSecondaryKnob(glideRateSlider);
    glideRateSlider.setComponentID("glideRate");

    glideRateValueLabel.setJustificationType(juce::Justification::centred);
    glideRateValueLabel.setColour(juce::Label::textColourId, AndesTheme::Colours::text);
    glideRateValueLabel.setFont(juce::Font(juce::FontOptions(7.5f)));
    glideRateValueLabel.setInterceptsMouseClicks(false, false);

    updateGlideRateValueLabel();

    glideRateSlider.onValueChange = [this]()
        {
            updateGlideRateValueLabel();
        };

    addAndMakeVisible(glideRateValueLabel);
}

juce::String AndesJXAudioProcessorEditor::formatGlideBendValue(double value) const
{
    const float semis = static_cast<float>(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (std::abs(semis) < 0.005f)
        return nbsp + nbsp + nbsp + "0.0 st";

    if (semis > 0.0f)
    {
        if (semis < 10.0f)
            return nbsp + "+" + juce::String(semis, 1) + " st";

        return "+" + juce::String(semis, 1) + " st";
    }

    if (std::abs(semis) < 10.0f)
        return nbsp + juce::String(semis, 1) + " st";

    return juce::String(semis, 1) + " st";
}

void AndesJXAudioProcessorEditor::updateGlideBendValueLabel()
{
    glideBendValueLabel.setText(formatGlideBendValue(glideBendSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseGlideBendControl()
{
    setupSecondaryKnob(glideBendSlider);
    glideBendSlider.setComponentID("glideBend");

    glideBendValueLabel.setJustificationType(juce::Justification::centred);
    glideBendValueLabel.setColour(juce::Label::textColourId, AndesTheme::Colours::text);
    glideBendValueLabel.setFont(juce::Font(juce::FontOptions(7.5f)));
    glideBendValueLabel.setInterceptsMouseClicks(false, false);

    updateGlideBendValueLabel();

    glideBendSlider.onValueChange = [this]()
        {
            updateGlideBendValueLabel();
        };

    addAndMakeVisible(glideBendValueLabel);
}

juce::String AndesJXAudioProcessorEditor::formatVibratoValue(double value) const
{
    const float v = static_cast<float>(value);

    if (std::abs(v) < 0.05f)
        return "OFF";

    if (v < 0.0f)
        return "PWM " + juce::String(-v, 1);

    return "VIB " + juce::String(v, 1);
}

void AndesJXAudioProcessorEditor::updateVibratoValueLabel()
{
    vibratoValueLabel.setText(formatVibratoValue(vibratoSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseVibratoControl()
{
    setupSecondaryKnob(vibratoSlider);
    vibratoSlider.setComponentID("vibrato");

    vibratoValueLabel.setJustificationType(juce::Justification::centred);
    vibratoValueLabel.setColour(juce::Label::textColourId, AndesTheme::Colours::text);
    vibratoValueLabel.setFont(juce::Font(juce::FontOptions(7.5f)));
    vibratoValueLabel.setInterceptsMouseClicks(false, false);

    updateVibratoValueLabel();

    vibratoSlider.onValueChange = [this]()
        {
            updateVibratoValueLabel();
        };

    addAndMakeVisible(vibratoValueLabel);
}

juce::String AndesJXAudioProcessorEditor::formatFilterVelocityValue(double value) const
{
    const float v = static_cast<float>(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (v < -90.0f)
        return "OFF";

    if (std::abs(v) < 0.5f)
        return nbsp + nbsp + "0 %";

    if (v > 0.0f)
    {
        const int iv = juce::roundToInt(v);

        if (iv < 10)
            return nbsp + "+" + juce::String(iv) + " %";

        return "+" + juce::String(iv) + " %";
    }

    const int iv = juce::roundToInt(v);

    if (std::abs(iv) < 10)
        return nbsp + juce::String(iv) + " %";

    return juce::String(iv) + " %";
}

void AndesJXAudioProcessorEditor::updateFilterVelocityValueLabel()
{
    filterVelocityValueLabel.setText(formatFilterVelocityValue(filterVelocitySlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseFilterVelocityControl()
{
    setupSecondaryKnob(filterVelocitySlider);
    filterVelocitySlider.setComponentID("filterVelocity");

    filterVelocityValueLabel.setJustificationType(juce::Justification::centred);
    filterVelocityValueLabel.setColour(juce::Label::textColourId, AndesTheme::Colours::text);
    filterVelocityValueLabel.setFont(juce::Font(juce::FontOptions(7.5f)));
    filterVelocityValueLabel.setInterceptsMouseClicks(false, false);

    updateFilterVelocityValueLabel();

    filterVelocitySlider.onValueChange = [this]()
        {
            updateFilterVelocityValueLabel();
        };

    addAndMakeVisible(filterVelocityValueLabel);
}

juce::String AndesJXAudioProcessorEditor::formatFilterEnvValue(double value) const
{
    const float v = static_cast<float>(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (std::abs(v) < 0.05f)
        return nbsp + nbsp + "0 %";

    if (v > 0.0f)
    {
        const int iv = juce::roundToInt(v);

        if (iv < 10)
            return nbsp + "+" + juce::String(iv) + " %";

        return "+" + juce::String(iv) + " %";
    }

    const int iv = juce::roundToInt(v);

    if (std::abs(iv) < 10)
        return nbsp + juce::String(iv) + " %";

    return juce::String(iv) + " %";
}

void AndesJXAudioProcessorEditor::updateFilterEnvValueLabel()
{
    filterEnvValueLabel.setText(formatFilterEnvValue(filterEnvSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseFilterEnvControl()
{
    setupSecondaryKnob(filterEnvSlider);
    filterEnvSlider.setComponentID("filterEnv");

    filterEnvValueLabel.setJustificationType(juce::Justification::centred);
    filterEnvValueLabel.setColour(juce::Label::textColourId, AndesTheme::Colours::text);
    filterEnvValueLabel.setFont(juce::Font(juce::FontOptions(7.5f)));
    filterEnvValueLabel.setInterceptsMouseClicks(false, false);

    updateFilterEnvValueLabel();

    filterEnvSlider.onValueChange = [this]()
        {
            updateFilterEnvValueLabel();
        };

    addAndMakeVisible(filterEnvValueLabel);
}

juce::String AndesJXAudioProcessorEditor::formatFilterLFOValue(double value) const
{
    const int lfo = juce::roundToInt(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (lfo == 0)
        return nbsp + nbsp + "0 %";

    if (lfo < 10)
        return nbsp + nbsp + juce::String(lfo) + " %";

    if (lfo < 100)
        return nbsp + juce::String(lfo) + " %";

    return juce::String(lfo) + " %";
}

void AndesJXAudioProcessorEditor::updateFilterLFOValueLabel()
{
    filterLFOValueLabel.setText(formatFilterLFOValue(filterLFOSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseFilterLFOControl()
{
    setupSecondaryKnob(filterLFOSlider);
    filterLFOSlider.setComponentID("filterLFO");

    filterLFOValueLabel.setJustificationType(juce::Justification::centred);
    filterLFOValueLabel.setColour(juce::Label::textColourId, AndesTheme::Colours::text);
    filterLFOValueLabel.setFont(juce::Font(juce::FontOptions(7.5f)));
    filterLFOValueLabel.setInterceptsMouseClicks(false, false);

    updateFilterLFOValueLabel();

    filterLFOSlider.onValueChange = [this]()
        {
            updateFilterLFOValueLabel();
        };

    addAndMakeVisible(filterLFOValueLabel);
}

juce::String AndesJXAudioProcessorEditor::formatFilterKeytrackValue(double value) const
{
    const int keytrack = juce::roundToInt(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (keytrack == 0)
        return nbsp + nbsp + "0 %";

    if (keytrack < 10)
        return nbsp + nbsp + juce::String(keytrack) + " %";

    if (keytrack < 100)
        return nbsp + juce::String(keytrack) + " %";

    return juce::String(keytrack) + " %";
}

void AndesJXAudioProcessorEditor::updateFilterKeytrackValueLabel()
{
    filterKeytrackValueLabel.setText(formatFilterKeytrackValue(filterKeytrackSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseFilterKeytrackControl()
{
    setupSecondaryKnob(filterKeytrackSlider);
    filterKeytrackSlider.setComponentID("filterKeytrack");

    filterKeytrackValueLabel.setJustificationType(juce::Justification::centred);
    filterKeytrackValueLabel.setColour(juce::Label::textColourId, AndesTheme::Colours::text);
    filterKeytrackValueLabel.setFont(juce::Font(juce::FontOptions(7.5f)));
    filterKeytrackValueLabel.setInterceptsMouseClicks(false, false);

    updateFilterKeytrackValueLabel();

    filterKeytrackSlider.onValueChange = [this]()
        {
            updateFilterKeytrackValueLabel();
        };

    addAndMakeVisible(filterKeytrackValueLabel);
}

juce::String AndesJXAudioProcessorEditor::formatLFORateValue(double value) const
{
    const float normalised = static_cast<float>(value);
    const float lfoHz = std::exp(7.0f * normalised - 4.0f);

    return juce::String(lfoHz, 3) + " Hz";
}

void AndesJXAudioProcessorEditor::updateLFORateValueLabel()
{
    lfoRateValueLabel.setText(formatLFORateValue(lfoRateSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseLFORateControl()
{
    setupSecondaryKnob(lfoRateSlider);
    lfoRateSlider.setComponentID("lfoRate");

    lfoRateValueLabel.setJustificationType(juce::Justification::centred);
    lfoRateValueLabel.setColour(juce::Label::textColourId, AndesTheme::Colours::text);
    lfoRateValueLabel.setFont(juce::Font(juce::FontOptions(7.5f)));
    lfoRateValueLabel.setInterceptsMouseClicks(false, false);

    updateLFORateValueLabel();

    lfoRateSlider.onValueChange = [this]()
        {
            updateLFORateValueLabel();
        };

    addAndMakeVisible(lfoRateValueLabel);
}

juce::String AndesJXAudioProcessorEditor::formatFilterKeycenterValue(double value) const
{
    static const char* noteNames[] =
    {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };

    const int note = juce::roundToInt(value);
    const int octave = (note / 12) - 1;
    const int noteNameIndex = note % 12;

    return juce::String(noteNames[noteNameIndex]) + juce::String(octave);
}

void AndesJXAudioProcessorEditor::updateFilterKeycenterValueLabel()
{
    filterKeycenterValueLabel.setText(formatFilterKeycenterValue(filterKeycenterSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseFilterKeycenterControl()
{
    setupSecondaryKnob(filterKeycenterSlider);
    filterKeycenterSlider.setComponentID("filterKeycenter");

    filterKeycenterSlider.setDoubleClickReturnValue(true, 60.0);

    filterKeycenterValueLabel.setJustificationType(juce::Justification::centred);
    filterKeycenterValueLabel.setColour(juce::Label::textColourId, AndesTheme::Colours::text);
    filterKeycenterValueLabel.setFont(juce::Font(juce::FontOptions(7.5f)));
    filterKeycenterValueLabel.setInterceptsMouseClicks(false, false);

    updateFilterKeycenterValueLabel();

    filterKeycenterSlider.onValueChange = [this]()
        {
            updateFilterKeycenterValueLabel();
        };

    addAndMakeVisible(filterKeycenterValueLabel);
}

void AndesJXAudioProcessorEditor::setupFader(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setLookAndFeel(faderLookAndFeel.get());
    addAndMakeVisible(slider);
}

void AndesJXAudioProcessorEditor::initialiseAmpEnvelopeControls()
{
    setupFader(ampAttackSlider);
    setupFader(ampDecaySlider);
    setupFader(ampSustainSlider);
    setupFader(ampReleaseSlider);

    ampAttackSlider.setComponentID("envAttack");
    ampDecaySlider.setComponentID("envDecay");
    ampSustainSlider.setComponentID("envSustain");
    ampReleaseSlider.setComponentID("envRelease");
}

void AndesJXAudioProcessorEditor::initialiseFilterEnvelopeADSRControls()
{
    setupFader(filterAttackSlider);
    setupFader(filterDecaySlider);
    setupFader(filterSustainSlider);
    setupFader(filterReleaseSlider);

    filterAttackSlider.setComponentID("filterAttack");
    filterDecaySlider.setComponentID("filterDecay");
    filterSustainSlider.setComponentID("filterSustain");
    filterReleaseSlider.setComponentID("filterRelease");
}

void AndesJXAudioProcessorEditor::setupSecondaryKnob(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setLookAndFeel(secondaryKnobLookAndFeel.get());
    addAndMakeVisible(slider);
}

void AndesJXAudioProcessorEditor::initialiseOscWaveSelectors()
{
    oscWaveSelector.addItem("Sine", 1);
    oscWaveSelector.addItem("Saw", 2);
    oscWaveSelector.addItem("Square", 3);
    oscWaveSelector.addItem("Triangle", 4);
    oscWaveSelector.addItem("PWM", 5);
    setupCombo(oscWaveSelector);

    osc2WaveSelector.addItem("Sine", 1);
    osc2WaveSelector.addItem("Saw", 2);
    osc2WaveSelector.addItem("Square", 3);
    osc2WaveSelector.addItem("Triangle", 4);
    osc2WaveSelector.addItem("PWM", 5);
    setupCombo(osc2WaveSelector);
}

void AndesJXAudioProcessorEditor::initialiseKnobs()
{
    setupKnob(mixSlider);
    setupKnob(resonanceSlider);
    setupKnob(cutoffSlider);
    setupKnob(outputSlider);

    mixSlider.setComponentID("oscMix");
    resonanceSlider.setComponentID("filterReso");
    cutoffSlider.setComponentID("filterFreq");
    outputSlider.setComponentID("output");
}

void AndesJXAudioProcessorEditor::initialisePresetSelector()
{
    setupCombo(presetSelector);

    const int numPrograms = audioProcessor.getNumPrograms();
    for (int i = 0; i < numPrograms; ++i)
        presetSelector.addItem(audioProcessor.getProgramName(i), i + 1);

    presetSelector.setSelectedId(audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);

    presetSelector.onChange = [this]()
        {
            const int id = presetSelector.getSelectedId();
            if (id > 0)
                audioProcessor.setCurrentProgram(id - 1);
        };
}

void AndesJXAudioProcessorEditor::initialiseGlideModeSelector()
{
    glideModeSelector.addItem("Off", 1);
    glideModeSelector.addItem("Legato", 2);
    glideModeSelector.addItem("Always", 3);
    setupCombo(glideModeSelector);
}

void AndesJXAudioProcessorEditor::initialisePolyToggle()
{
    polyToggle.setLookAndFeel(toggleLookAndFeel.get());
    polyToggle.setButtonText("Mono");
    polyToggle.setToggleState(false, juce::dontSendNotification);
    polyToggle.setTooltip("Mono / Poly");
    addAndMakeVisible(polyToggle);

    if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.apvts.getParameter("polyMode")))
    {
        const bool isPoly = (param->getIndex() == 1);
        polyToggle.setToggleState(isPoly, juce::dontSendNotification);
        polyToggle.setButtonText(isPoly ? "Poly" : "Mono");

        polyToggle.onClick = [this, param]()
            {
                const int cur = param->getIndex();
                const int next = (cur == 0) ? 1 : 0;
                param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(next)));
                polyToggle.setButtonText(next == 1 ? "Poly" : "Mono");
            };

        audioProcessor.apvts.addParameterListener("polyMode", this);
    }
}

void AndesJXAudioProcessorEditor::initialiseFilterTypeControl()
{
    filterTypeControl.setLookAndFeelForButtons(segmentedButtonLookAndFeel.get());
    filterTypeControl.setItems({ "SVF", "Moog" }, 1002);
    addAndMakeVisible(filterTypeControl);

    if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.apvts.getParameter("filterType")))
    {
        filterTypeControl.setSelectedIndex(param->getIndex(), juce::dontSendNotification);

        filterTypeControl.onChange = [this, param](int index)
            {
                param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(index)));
            };

        audioProcessor.apvts.addParameterListener("filterType", this);
    }
}

void AndesJXAudioProcessorEditor::initialiseAttachments()
{
    oscWaveAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "osc1Wave", oscWaveSelector);
    osc2WaveAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "osc2Wave", osc2WaveSelector);
    glideModeAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "glideMode", glideModeSelector);

    mixAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "oscMix", mixSlider);
    resonanceAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterReso", resonanceSlider);
    cutoffAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterFreq", cutoffSlider);
    outputAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "outputLevel", outputSlider);

    oscTuneAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "oscTune", oscTuneSlider);
    stereoWidthAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "stereoWidth", stereoWidthSlider);
    noiseAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise", noiseSlider);
    oscFineAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "oscFine", oscFineSlider);
    octaveAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "octave", octaveSlider);
    tuningAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "tuning", tuningSlider);
    glideRateAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "glideRate", glideRateSlider);
    glideBendAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "glideBend", glideBendSlider);
    vibratoAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "vibrato", vibratoSlider);
    filterVelocityAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterVelocity", filterVelocitySlider);
    filterEnvAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterEnv", filterEnvSlider);
    filterLFOAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterLFO", filterLFOSlider);
    filterKeycenterAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterKeycenter", filterKeycenterSlider);
    filterKeytrackAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterKeytrack", filterKeytrackSlider);
    lfoRateAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "lfoRate", lfoRateSlider);

    ampAttackAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "envAttack", ampAttackSlider);
    ampDecayAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "envDecay", ampDecaySlider);
    ampSustainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "envSustain", ampSustainSlider);
    ampReleaseAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "envRelease", ampReleaseSlider);

    filterAttackAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterAttack", filterAttackSlider);
    filterDecayAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterDecay", filterDecaySlider);
    filterSustainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterSustain", filterSustainSlider);
    filterReleaseAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterRelease", filterReleaseSlider);
}

AndesJXAudioProcessorEditor::~AndesJXAudioProcessorEditor()
{
    // Reset look-and-feel on each combo BEFORE destroying the LAF
    oscWaveSelector.setLookAndFeel(nullptr);
    osc2WaveSelector.setLookAndFeel(nullptr);
    //filterKeycenterSelector.setLookAndFeel(nullptr);
    presetSelector.setLookAndFeel(nullptr);
    glideModeSelector.setLookAndFeel(nullptr);
    comboBoxLookAndFeel.reset();

    filterTypeControl.setLookAndFeelForButtons(nullptr);
    segmentedButtonLookAndFeel.reset();

    // Remove parameter listener and restore lookAndFeel pointer
    audioProcessor.apvts.removeParameterListener("polyMode", this);
    audioProcessor.apvts.removeParameterListener("filterType", this);

    polyToggle.setLookAndFeel(nullptr);
    
    toggleLookAndFeel.reset();

    mixSlider.setLookAndFeel(nullptr);
    resonanceSlider.setLookAndFeel(nullptr);
    cutoffSlider.setLookAndFeel(nullptr);
    outputSlider.setLookAndFeel(nullptr);

    knobPrincipalLookAndFeel.reset();

    oscTuneSlider.setLookAndFeel(nullptr);
    stereoWidthSlider.setLookAndFeel(nullptr);
    noiseSlider.setLookAndFeel(nullptr);
    oscFineSlider.setLookAndFeel(nullptr);
    octaveSlider.setLookAndFeel(nullptr);
    tuningSlider.setLookAndFeel(nullptr);

    glideRateSlider.setLookAndFeel(nullptr);
    glideBendSlider.setLookAndFeel(nullptr);

    vibratoSlider.setLookAndFeel(nullptr);
    filterVelocitySlider.setLookAndFeel(nullptr);
    
    filterEnvSlider.setLookAndFeel(nullptr);
	filterLFOSlider.setLookAndFeel(nullptr);
    filterKeytrackSlider.setLookAndFeel(nullptr);
    filterKeycenterSlider.setLookAndFeel(nullptr);

    lfoRateSlider.setLookAndFeel(nullptr);

	secondaryKnobLookAndFeel.reset();

    ampAttackSlider.setLookAndFeel(nullptr);
    ampDecaySlider.setLookAndFeel(nullptr);
    ampSustainSlider.setLookAndFeel(nullptr);
    ampReleaseSlider.setLookAndFeel(nullptr);

    filterAttackSlider.setLookAndFeel(nullptr);
    filterDecaySlider.setLookAndFeel(nullptr);
    filterSustainSlider.setLookAndFeel(nullptr);
    filterReleaseSlider.setLookAndFeel(nullptr);

    faderLookAndFeel.reset();
}

//==============================================================================
void AndesJXAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    if (backgroundAndesJX.isValid())
    {
        g.drawImageWithin(
            backgroundAndesJX,
            0, 0,
            getWidth(), getHeight(),
            juce::RectanglePlacement::stretchToFit
        );
    }

    // Section titles
    drawSectionTitle(g, { 20, 15, 140, 15 }, "OSCILLATORS");
    drawSectionTitle(g, { 300, 15, 120, 15 }, "PERFORMANCE");
    drawSectionTitle(g, { 20, 165, 100, 15 }, "FILTER");
    drawSectionTitle(g, { 300, 165, 120, 15 }, "ENVELOPES");
    drawSectionTitle(g, { 20, 315, 120, 15 }, "MODULATION");
    drawSectionTitle(g, { 370, 315, 100, 15 }, "MASTER");

    // OSC 1
    drawControlLabel(g, { 20, 62, 50, 12 }, "WAVE");
    drawControlLabel(g, { 158, 62, 40, 12 }, "COARSE");
    drawControlLabel(g, { 198, 62, 40, 12 }, "WIDTH");
    drawControlLabel(g, { 238, 62, 40, 12 }, "NOISE");

    // OSC 2
    drawControlLabel(g, { 20, 122, 50, 12 }, "WAVE");
    drawControlLabel(g, { 158, 122, 40, 12 }, "FINE");
    drawControlLabel(g, { 198, 122, 40, 12 }, "OCTAVE");
    drawControlLabel(g, { 238, 122, 40, 12 }, "TUNE");

    // MIX
    drawControlLabel(g, { 80, 105, 64, 12 }, "MIX");

    // PERFORMANCE
    drawControlLabel(g, { 320, 68, 50, 12 }, "GLIDE");
    drawControlLabel(g, { 408, 68, 54, 12 }, "VOICE");

    drawControlLabel(g, { 302, 122, 48, 12 }, "RATE");
    drawControlLabel(g, { 342, 122, 48, 12 }, "BEND");

    drawControlLabel(g, { 396, 122, 44, 12 }, "PWM/VIB");
    drawControlLabel(g, { 434, 122, 50, 12 }, "VEL FLTR");

    // FILTER
    drawControlLabel(g, { 47, 212, 76, 12 }, "TYPE");

    drawControlLabel(g, { 25, 280, 64, 12 }, "CUTOFF");
    drawControlLabel(g, { 90, 280, 64, 12 }, "RESO");

    drawControlLabel(g, { 194, 272, 48, 12 }, "ENV AMT");
    drawControlLabel(g, { 174, 212, 48, 12 }, "KEY TRCK");
    drawControlLabel(g, { 218, 212, 52, 12 }, "KEY CNTR");

    // MODULATION
    drawControlLabel(g, { 24, 382, 60, 12 }, "LFO RATE");
    drawControlLabel(g, { 74, 382, 60, 12 }, "VCF MOD");

    // MASTER
    drawControlLabel(g, { 395, 400, 60, 12 }, "OUTPUT");
    drawControlLabel(g, { 205, 400, 100, 12 }, "PRESET");

	// ENVELOPES
    drawControlLabel(g, { 305, 186, 70, 12 }, "AMP");
    drawTinyControlLabel(g, { 300, 272, 16, 10 }, "A");
    drawTinyControlLabel(g, { 320, 272, 16, 10 }, "D");
    drawTinyControlLabel(g, { 340, 272, 16, 10 }, "S");
    drawTinyControlLabel(g, { 360, 272, 16, 10 }, "R");

    drawControlLabel(g, { 403, 186, 70, 12 }, "FILTER");
    drawTinyControlLabel(g, { 400, 272, 16, 10 }, "A");
    drawTinyControlLabel(g, { 420, 272, 16, 10 }, "D");
    drawTinyControlLabel(g, { 440, 272, 16, 10 }, "S");
    drawTinyControlLabel(g, { 460, 272, 16, 10 }, "R");


}

void AndesJXAudioProcessorEditor::resized()
{
    oscWaveSelector.setBounds(20, 40, 50, 16);
    osc2WaveSelector.setBounds(20, 100, 50, 16);
    presetSelector.setBounds(205, 380, 100, 16);
    polyToggle.setBounds(410, 45, 50, 16);
    glideModeSelector.setBounds(320, 45, 50, 16);
    filterTypeControl.setBounds(45, 190, 80, 16);
    
    mixSlider.setBounds(80, 50, 64, 64);
    resonanceSlider.setBounds(90, 225, 64, 64);
    cutoffSlider.setBounds(25, 225, 64, 64);
    outputSlider.setBounds(385, 325, 80, 80);

    oscTuneValueLabel.setBounds(160, 22, 34, 10);
    oscTuneSlider.setBounds(160, 30, 36, 36);
    stereoWidthValueLabel.setBounds(196, 22, 48, 10);
    stereoWidthSlider.setBounds(200, 30, 36, 36);
    noiseValueLabel.setBounds(236, 22, 48, 10);
    noiseSlider.setBounds(240, 30, 36, 36);
    oscFineValueLabel.setBounds(160, 82, 34, 10);
    oscFineSlider.setBounds(160, 90, 36, 36);
    octaveValueLabel.setBounds(198, 82, 34, 10);
    octaveSlider.setBounds(200, 90, 36, 36);
    tuningValueLabel.setBounds(240, 82, 34, 10);
    tuningSlider.setBounds(240, 90, 36, 36);

    glideRateValueLabel.setBounds(302, 82, 48, 10);
    glideRateSlider.setBounds(308, 90, 36, 36);
    glideBendValueLabel.setBounds(342, 82, 48, 10);
    glideBendSlider.setBounds(348, 90, 36, 36);

    vibratoValueLabel.setBounds(392, 82, 52, 10);
    vibratoSlider.setBounds(400, 90, 36, 36);
    filterVelocityValueLabel.setBounds(432, 82, 52, 10);
    filterVelocitySlider.setBounds(440, 90, 36, 36);

    filterEnvValueLabel.setBounds(194, 232, 48, 10);
    filterEnvSlider.setBounds(200, 240, 36, 36);
    filterKeytrackValueLabel.setBounds(174, 172, 52, 10);
    filterKeytrackSlider.setBounds(180, 180, 36, 36);
    filterKeycenterValueLabel.setBounds(219, 172, 48, 10);
    filterKeycenterSlider.setBounds(225, 180, 36, 36);

    lfoRateValueLabel.setBounds(25, 340, 52, 10);
    lfoRateSlider.setBounds(35, 350, 36, 36);
    filterLFOValueLabel.setBounds(80, 342, 48, 10);
    filterLFOSlider.setBounds(85, 350, 36, 36);

    // Amp ADSR
    ampAttackSlider.setBounds(300, 190, 16, 90);
    ampDecaySlider.setBounds(320, 190, 16, 90);
    ampSustainSlider.setBounds(340, 190, 16, 90);
    ampReleaseSlider.setBounds(360, 190, 16, 90);

    // Filter ADSR
    filterAttackSlider.setBounds(400, 190, 16, 90);
    filterDecaySlider.setBounds(420, 190, 16, 90);
    filterSustainSlider.setBounds(440, 190, 16, 90);
    filterReleaseSlider.setBounds(460, 190, 16, 90);
}

void AndesJXAudioProcessorEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "polyMode")
    {
        // newValue viene en 0..1: convertimos a índice seguro
        if (auto* p = audioProcessor.apvts.getParameter(parameterID))
        {
            const int index = juce::roundToInt(p->convertFrom0to1(newValue));
            const bool isPoly = (index == 1);

            // parameterChanged se llama desde hilo de audio -> usar callAsync para actualizar UI
            juce::MessageManager::callAsync([this, isPoly]()
            {
                polyToggle.setToggleState(isPoly, juce::dontSendNotification);
                polyToggle.setButtonText(isPoly ? "Poly" : "Mono");
            });
        }
    }
    else if (parameterID == "filterType")
    {
        if (auto* p = audioProcessor.apvts.getParameter(parameterID))
        {
            const int index = juce::roundToInt(p->convertFrom0to1(newValue));

            juce::MessageManager::callAsync([this, index]()
                {
                    filterTypeControl.setSelectedIndex(index, juce::dontSendNotification);
                });
        }
    }
}

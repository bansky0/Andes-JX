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
    secondaryKnobLookAndFeel->setShowValueText(true);
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

void AndesJXAudioProcessorEditor::initialiseOscTuneControl()
{
    setupSecondaryKnob(oscTuneSlider);
    oscTuneSlider.setComponentID("oscTune");

    oscTuneSlider.textFromValueFunction = [](double value)
        {
            const int semitones = juce::roundToInt(value);

            if (semitones > 0)
                return "+" + juce::String(semitones);

            return juce::String(semitones);
        };
}


void AndesJXAudioProcessorEditor::initialiseStereoWidthControl()
{
    setupSecondaryKnob(stereoWidthSlider);
    stereoWidthSlider.setComponentID("stereoWidth");

    stereoWidthSlider.textFromValueFunction = [](double value)
        {
            return juce::String(juce::roundToInt(value));
        };
}

void AndesJXAudioProcessorEditor::initialiseNoiseControl()
{
    setupSecondaryKnob(noiseSlider);
    noiseSlider.setComponentID("noise");

    noiseSlider.textFromValueFunction = [](double value)
        {
            return juce::String(juce::roundToInt(value));
        };
}

void AndesJXAudioProcessorEditor::initialiseOscFineControl()
{
    setupSecondaryKnob(oscFineSlider);
    oscFineSlider.setComponentID("oscFine");

    oscFineSlider.textFromValueFunction = [](double value)
        {
            const float cents = static_cast<float>(value);

            if (std::abs(cents) < 0.05f)
                return juce::String("0.0 c");

            if (cents > 0.0f)
                return "+" + juce::String(cents, 1) + " c";

            return juce::String(cents, 1) + " c";
        };
}

void AndesJXAudioProcessorEditor::initialiseOctaveControl()
{
    setupSecondaryKnob(octaveSlider);
    octaveSlider.setComponentID("octave");

    octaveSlider.textFromValueFunction = [](double value)
        {
            const int oct = juce::roundToInt(value);

            if (oct > 0)
                return "+" + juce::String(oct);

            return juce::String(oct);
        };
}

void AndesJXAudioProcessorEditor::initialiseTuningControl()
{
    setupSecondaryKnob(tuningSlider);
    tuningSlider.setComponentID("tuning");

    tuningSlider.textFromValueFunction = [](double value)
        {
            const float cents = static_cast<float>(value);

            if (std::abs(cents) < 0.05f)
                return juce::String("0");

            if (cents > 0.0f)
                return "+" + juce::String(cents, 1);

            return juce::String(cents, 1);
        };
}

void AndesJXAudioProcessorEditor::initialiseGlideRateControl()
{
    setupSecondaryKnob(glideRateSlider);
    glideRateSlider.setComponentID("glideRate");

    glideRateSlider.textFromValueFunction = [](double value)
        {
            return juce::String(juce::roundToInt(value));
        };
}

void AndesJXAudioProcessorEditor::initialiseGlideBendControl()
{
    setupSecondaryKnob(glideBendSlider);
    glideBendSlider.setComponentID("glideBend");

    glideBendSlider.textFromValueFunction = [](double value)
        {
            const float semis = static_cast<float>(value);

            if (std::abs(semis) < 0.005f)
                return juce::String("0");

            if (semis > 0.0f)
                return "+" + juce::String(semis, 1);

            return juce::String(semis, 1);
        };
}

void AndesJXAudioProcessorEditor::initialiseVibratoControl()
{
    setupSecondaryKnob(vibratoSlider);
    vibratoSlider.setComponentID("vibrato");

    vibratoSlider.textFromValueFunction = [](double value)
        {
            const float v = static_cast<float>(value);

            if (std::abs(v) < 0.05f)
                return juce::String("0.0");

            if (v < 0.0f)
                return "PWM " + juce::String(-v, 1);

            return juce::String(v, 1);
        };
}

void AndesJXAudioProcessorEditor::initialiseFilterVelocityControl()
{
    setupSecondaryKnob(filterVelocitySlider);
    filterVelocitySlider.setComponentID("filterVelocity");

    filterVelocitySlider.textFromValueFunction = [](double value)
        {
            const float v = static_cast<float>(value);

            if (v < -90.0f)
                return juce::String("OFF");

            if (std::abs(v) < 0.5f)
                return juce::String("0");

            if (v > 0.0f)
                return "+" + juce::String(juce::roundToInt(v));

            return juce::String(juce::roundToInt(v));
        };
}

void AndesJXAudioProcessorEditor::initialiseFilterEnvControl()
{
    setupSecondaryKnob(filterEnvSlider);
    filterEnvSlider.setComponentID("filterEnv");

    filterEnvSlider.textFromValueFunction = [](double value)
        {
            const float v = static_cast<float>(value);

            if (std::abs(v) < 0.05f)
                return juce::String("0");

            if (v > 0.0f)
                return "+" + juce::String(juce::roundToInt(v));

            return juce::String(juce::roundToInt(v));
        };
}

void AndesJXAudioProcessorEditor::initialiseFilterLFOControl()
{
    setupSecondaryKnob(filterLFOSlider);
    filterLFOSlider.setComponentID("filterLFO");

    filterLFOSlider.textFromValueFunction = [](double value)
        {
            return juce::String(juce::roundToInt(value));
        };
}

void AndesJXAudioProcessorEditor::initialiseFilterKeytrackControl()
{
    setupSecondaryKnob(filterKeytrackSlider);
    filterKeytrackSlider.setComponentID("filterKeytrack");

    filterKeytrackSlider.textFromValueFunction = [](double value)
        {
            return juce::String(juce::roundToInt(value));
        };
}

void AndesJXAudioProcessorEditor::initialiseLFORateControl()
{
    setupSecondaryKnob(lfoRateSlider);
    lfoRateSlider.setComponentID("lfoRate");

    lfoRateSlider.textFromValueFunction = [](double value)
        {
            const float normalised = static_cast<float>(value);
            const float lfoHz = std::exp(7.0f * normalised - 4.0f);
            return juce::String(lfoHz, 3);
        };
}

void AndesJXAudioProcessorEditor::initialiseFilterKeycenterControl()
{
    setupSecondaryKnob(filterKeycenterSlider);
    filterKeycenterSlider.setComponentID("filterKeycenter");

    filterKeycenterSlider.setDoubleClickReturnValue(true, 60.0);

    filterKeycenterSlider.textFromValueFunction = [](double value)
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
        };
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

    oscTuneSlider.setBounds(160, 30, 36, 36);
    stereoWidthSlider.setBounds(200, 30, 36, 36);
    noiseSlider.setBounds(240, 30, 36, 36);
    oscFineSlider.setBounds(160, 90, 36, 36);
    octaveSlider.setBounds(200, 90, 36, 36);
    tuningSlider.setBounds(240, 90, 36, 36);

    glideRateSlider.setBounds(308, 90, 36, 36);
    glideBendSlider.setBounds(348, 90, 36, 36);

    vibratoSlider.setBounds(400, 90, 36, 36);
    filterVelocitySlider.setBounds(440, 90, 36, 36);

    filterEnvSlider.setBounds(200, 240, 36, 36);
    //filterLFOSlider.setBounds(220, 190, 36, 36);
    filterKeytrackSlider.setBounds(180, 180, 36, 36);
    filterKeycenterSlider.setBounds(225, 180, 36, 36);

    lfoRateSlider.setBounds(35, 350, 36, 36);
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

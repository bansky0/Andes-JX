/*
  ==============================================================================
 
    This file contains the basic framework code for a JUCE plugin editor.
 
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AndesJXAudioProcessorEditor::AndesJXAudioProcessorEditor(AndesJXAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    initialiseBackground();
    initialiseLookAndFeels();

    initialiseOscWaveSelectors();
    initialiseKnobs();
    initialiseOscTuneControl();
    initialiseFilterKeycenterSelector();
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
    secondaryKnobLookAndFeel->setShowValueText(false);
    secondaryKnobLookAndFeel->setTextInset(3);
    

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

void AndesJXAudioProcessorEditor::initialiseFilterKeycenterSelector()
{
    setupCombo(filterKeycenterSelector);

    const char* noteNames[] =
    {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };

    for (int note = 24; note <= 96; ++note)
    {
        const int octave = (note / 12) - 1;
        const int noteNameIndex = note % 12;
        const juce::String label = juce::String(noteNames[noteNameIndex]) + juce::String(octave);
        filterKeycenterSelector.addItem(label, note);
    }

    if (auto* raw = audioProcessor.apvts.getRawParameterValue("filterKeycenter"))
    {
        const int currentNote = juce::jlimit(24, 96, static_cast<int>(std::round(*raw)));
        filterKeycenterSelector.setSelectedId(currentNote, juce::dontSendNotification);
    }

    filterKeycenterSelector.onChange = [this]()
        {
            const int selectedId = filterKeycenterSelector.getSelectedId();

            if (selectedId <= 0)
                return;

            if (auto* param = audioProcessor.apvts.getParameter("filterKeycenter"))
                param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(selectedId)));
        };
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
}

AndesJXAudioProcessorEditor::~AndesJXAudioProcessorEditor()
{
    // Reset look-and-feel on each combo BEFORE destroying the LAF
    oscWaveSelector.setLookAndFeel(nullptr);
    osc2WaveSelector.setLookAndFeel(nullptr);
    filterKeycenterSelector.setLookAndFeel(nullptr);
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
    oscTuneSlider.setLookAndFeel(nullptr);
}

//==============================================================================
void AndesJXAudioProcessorEditor::paint (juce::Graphics& g)
{
    // clear background first
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
}

void AndesJXAudioProcessorEditor::resized()
{
    oscWaveSelector.setBounds(20, 40, 50, 16);
    osc2WaveSelector.setBounds(20, 100, 50, 16);
    filterKeycenterSelector.setBounds(220, 250, 35, 16);
    presetSelector.setBounds(205, 390, 100, 16);
    polyToggle.setBounds(410, 50, 50, 16);
    glideModeSelector.setBounds(320, 50, 50, 16);
    filterTypeControl.setBounds(50, 200, 80, 16);
    
    mixSlider.setBounds(90, 50, 64, 64);
    resonanceSlider.setBounds(90, 225, 64, 64);
    cutoffSlider.setBounds(30, 225, 64, 64);
    outputSlider.setBounds(390, 330, 80, 80);

    oscTuneSlider.setBounds(160, 50, 48, 48);
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

/*
  ==============================================================================
 
    This file contains the basic framework code for a JUCE plugin editor.
 
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AndesJXAudioProcessorEditor::AndesJXAudioProcessorEditor (AndesJXAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
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

    //==========================================================================
        // Shared ComboBox LookAndFeel
    comboBoxLookAndFeel = std::make_unique<ComboBoxLookAndFeel>();
    comboBoxLookAndFeel->setDefaultComboBackground(juce::Colour::fromRGB(0x4F, 0x6B, 0x72));
    comboBoxLookAndFeel->setDefaultComboText(juce::Colour::fromRGB(0xD9, 0xD9, 0xD9));

    //==========================================================================
    // OSC1 Combo
    oscWaveSelector.setLookAndFeel(comboBoxLookAndFeel.get());
    oscWaveSelector.addItem("Sine", 1);
    oscWaveSelector.addItem("Saw", 2);
    oscWaveSelector.addItem("Square", 3);
    oscWaveSelector.addItem("Triangle", 4);
    oscWaveSelector.addItem("PWM", 5);
    oscWaveSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(0x4F, 0x6B, 0x72));
    oscWaveSelector.setColour(juce::ComboBox::textColourId, juce::Colour::fromRGB(0xD9, 0xD9, 0xD9));
    addAndMakeVisible(oscWaveSelector);

    // OSC2 Combo
    osc2WaveSelector.setLookAndFeel(comboBoxLookAndFeel.get());
    osc2WaveSelector.addItem("Sine", 1);
    osc2WaveSelector.addItem("Saw", 2);
    osc2WaveSelector.addItem("Square", 3);
    osc2WaveSelector.addItem("Triangle", 4);
    osc2WaveSelector.addItem("PWM", 5);
    osc2WaveSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(0x4F, 0x6B, 0x72));
    osc2WaveSelector.setColour(juce::ComboBox::textColourId, juce::Colour::fromRGB(0xD9, 0xD9, 0xD9));
    addAndMakeVisible(osc2WaveSelector);

    //==========================================================================
	// Sliders LookAndFeel
    knobPrincipalLookAndFeel = std::make_unique<KnobPrincipalLookAndFeel>();

    auto configureKnob = [this](juce::Slider& s)
        {
            s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            s.setLookAndFeel(knobPrincipalLookAndFeel.get());
            addAndMakeVisible(s);
        };

    configureKnob(mixSlider);
    configureKnob(resonanceSlider);
    configureKnob(cutoffSlider);
    configureKnob(outputSlider);
    
	mixSlider.setComponentID("oscMix");
	resonanceSlider.setComponentID("filterReso");
	cutoffSlider.setComponentID("filterFreq");
    outputSlider.setComponentID("output");
    /*
    mixSlider.setRange(0.0, 100.0, 1.0);
    mixSlider.setValue(50.0);

    mixSlider.textFromValueFunction = [](double value)
        {
            return juce::String(juce::roundToInt(value)) + " %";
        };
    
    resonanceSlider.setRange(0.0, 100.0, 1.0);
    resonanceSlider.setValue(0.0);

    resonanceSlider.textFromValueFunction = [](double value)
        {
            return juce::String(juce::roundToInt(value)) + " %";
        };

	cutoffSlider.setRange(0.0, 100.0, 1.0);
	cutoffSlider.setValue(50.0);

    cutoffSlider.textFromValueFunction = [](double value)
        {
            return juce::String(juce::roundToInt(value)) + " %";
        };
        */
    //==========================================================================
    // Filter Keycenter Combo
    filterKeycenterSelector.setLookAndFeel(comboBoxLookAndFeel.get());

    const int noteIds[] = { 24, 36, 48, 60, 72, 84, 96 };
    const char* noteNames[] = { "C1", "C2", "C3", "C4", "C5", "C6", "C7" };

    for (int i = 0; i < 7; ++i)
        filterKeycenterSelector.addItem(noteNames[i], noteIds[i]);

    filterKeycenterSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(0x4F, 0x6B, 0x72));
    filterKeycenterSelector.setColour(juce::ComboBox::textColourId, juce::Colour::fromRGB(0xD9, 0xD9, 0xD9));

    addAndMakeVisible(filterKeycenterSelector);

    if (auto* raw = audioProcessor.apvts.getRawParameterValue("filterKeycenter"))
    {
        const int currentNote = juce::jlimit(0, 127, static_cast<int>(std::round(*raw)));
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

    //==========================================================================
    // Presets Combo
    presetSelector.setLookAndFeel(comboBoxLookAndFeel.get());

    const int numPrograms = audioProcessor.getNumPrograms();
    for (int i = 0; i < numPrograms; ++i)
        presetSelector.addItem(audioProcessor.getProgramName(i), i + 1);

    presetSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(0x4F, 0x6B, 0x72));
    presetSelector.setColour(juce::ComboBox::textColourId, juce::Colour::fromRGB(0xD9, 0xD9, 0xD9));

    addAndMakeVisible(presetSelector);

    presetSelector.setSelectedId(audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);

    presetSelector.onChange = [this]()
        {
            const int id = presetSelector.getSelectedId();
            if (id > 0)
                audioProcessor.setCurrentProgram(id - 1);
        };

    //==========================================================================
    // Glide Mode Combo
    glideModeSelector.setLookAndFeel(comboBoxLookAndFeel.get());
    glideModeSelector.addItem("OFF", 1);
    glideModeSelector.addItem("LEGATO", 2);
    glideModeSelector.addItem("ALWAYS", 3);

    glideModeSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(0x4F, 0x6B, 0x72));
    glideModeSelector.setColour(juce::ComboBox::textColourId, juce::Colour::fromRGB(0xD9, 0xD9, 0xD9));

    addAndMakeVisible(glideModeSelector);

    //==========================================================================
    // Toggle LookAndFeel for Mono/Poly
    toggleLookAndFeel = std::make_unique<ToggleLookAndFeel>();

    toggleLookAndFeel->setColour(ToggleLookAndFeel::backgroundOnColourId,
        juce::Colour::fromRGB(0x4F, 0x6B, 0x72));
    toggleLookAndFeel->setColour(ToggleLookAndFeel::backgroundOffColourId,
        juce::Colour::fromRGB(0x3F, 0x55, 0x5B));
    toggleLookAndFeel->setColour(ToggleLookAndFeel::textColourId,
        juce::Colour::fromRGB(0xD9, 0xD9, 0xD9));
    toggleLookAndFeel->setColour(ToggleLookAndFeel::outlineColourId,
        juce::Colour::fromRGB(0x4F, 0x6B, 0x72).darker(0.35f));

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
                param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(next)
                    ));
                polyToggle.setButtonText(next == 1 ? "Poly" : "Mono");
            };

        audioProcessor.apvts.addParameterListener("polyMode", this);
    }

    //==========================================================================
    // Filter Type Segmented Control
    segmentedButtonLookAndFeel = std::make_unique<SegmentedButtonLookAndFeel>();
    segmentedButtonLookAndFeel->setDefaultBackground(juce::Colour::fromRGB(0x4F, 0x6B, 0x72));
    segmentedButtonLookAndFeel->setDefaultText(juce::Colour::fromRGB(0xD9, 0xD9, 0xD9));
    segmentedButtonLookAndFeel->setFontHeight(11.0f);

    filterTypeControl.setLookAndFeelForButtons(segmentedButtonLookAndFeel.get());
    filterTypeControl.setItems({ "SVF", "MOOG" }, 1002);
    addAndMakeVisible(filterTypeControl);

    if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.apvts.getParameter("filterType")))
    {
        // Inicializar UI desde el parámetro
        filterTypeControl.setSelectedIndex(param->getIndex(), juce::dontSendNotification);

        // UI -> parámetro
        filterTypeControl.onChange = [this, param](int index)
            {
                param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(index)));
            };

        // Parámetro -> UI
        audioProcessor.apvts.addParameterListener("filterType", this);
    }

    //==========================================================================
    // Attachments
    oscWaveAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "osc1Wave", oscWaveSelector);
    osc2WaveAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "osc2Wave", osc2WaveSelector);
    filterKeycenterAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filterKeycenter", filterKeycenterSelector);
    glideModeAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "glideMode", glideModeSelector);
    
    mixAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "oscMix", mixSlider);
	resonanceAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterReso", resonanceSlider);
	cutoffAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterFreq", cutoffSlider);
    outputAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "outputLevel", outputSlider);
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

    //g.setColour(juce::Colours::white);
    //g.setFont(14.0f);
    //g.drawText("Poly Mode:", 10, 10, 100, 20, juce::Justification::left);
}

void AndesJXAudioProcessorEditor::resized()
{
    oscWaveSelector.setBounds(20, 40, 50, 16);
    osc2WaveSelector.setBounds(20, 100, 50, 16);
    filterKeycenterSelector.setBounds(230, 250, 25, 16);
    presetSelector.setBounds(205, 390, 100, 16);
    polyToggle.setBounds(410, 50, 50, 16);
    glideModeSelector.setBounds(320, 50, 50, 16);
    filterTypeControl.setBounds(50, 200, 80, 16);
    
    mixSlider.setBounds(90, 50, 64, 64);
    resonanceSlider.setBounds(90, 225, 64, 64);
    cutoffSlider.setBounds(30, 225, 64, 64);
    outputSlider.setBounds(390, 330, 80, 80);
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

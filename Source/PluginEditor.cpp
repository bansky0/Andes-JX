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

    // Create single shared LookAndFeel instance and set defaults
    comboBoxLookAndFeel = std::make_unique<ComboBoxLookAndFeel>();
    comboBoxLookAndFeel->setDefaultComboBackground(juce::Colour::fromRGB(0x4F, 0x6B, 0x72)); // #4F6B72
    comboBoxLookAndFeel->setDefaultComboText(juce::Colour::fromRGB(0xD9, 0xD9, 0xD9));       // #D9D9D9

    // OSC1 Combo
    oscWaveSelector.setLookAndFeel(comboBoxLookAndFeel.get());
    oscWaveSelector.addItem("Sine", 1);
    oscWaveSelector.addItem("Saw", 2);
    oscWaveSelector.addItem("Square", 3);
    oscWaveSelector.addItem("Triangle", 4);
    oscWaveSelector.addItem("PWM", 5);
    oscWaveSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(0x4F,0x6B,0x72));
    oscWaveSelector.setColour(juce::ComboBox::textColourId,       juce::Colour::fromRGB(0xD9,0xD9,0xD9));
    addAndMakeVisible(oscWaveSelector);

    // OSC2 Combo
    osc2WaveSelector.setLookAndFeel(comboBoxLookAndFeel.get());
    osc2WaveSelector.addItem("Sine", 1);
    osc2WaveSelector.addItem("Saw", 2);
    osc2WaveSelector.addItem("Square", 3);
    osc2WaveSelector.addItem("Triangle", 4);
    osc2WaveSelector.addItem("PWM", 5);
    osc2WaveSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(0x4F,0x6B,0x72));
    osc2WaveSelector.setColour(juce::ComboBox::textColourId,       juce::Colour::fromRGB(0xD9,0xD9,0xD9));
    addAndMakeVisible(osc2WaveSelector);

    // Filter Keycenter Combo (reuses the same LAF)
    filterKeycenterSelector.setLookAndFeel(comboBoxLookAndFeel.get());

    // populate C1..C7 (MIDI notes: 24,36,48,60,72,84,96)
    const int noteIds[] = { 24, 36, 48, 60, 72, 84, 96 };
    const char* noteNames[] = { "C1", "C2", "C3", "C4", "C5", "C6", "C7" };

    for (int i = 0; i < 7; ++i)
        filterKeycenterSelector.addItem(noteNames[i], noteIds[i]);

    // Colours and look
    filterKeycenterSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(0x4F,0x6B,0x72));
    filterKeycenterSelector.setColour(juce::ComboBox::textColourId,       juce::Colour::fromRGB(0xD9,0xD9,0xD9));

    addAndMakeVisible(filterKeycenterSelector);

    // Initialize selection from APVTS raw value (if present)
    if (auto* raw = audioProcessor.apvts.getRawParameterValue ("filterKeycenter"))
    {
        const int currentNote = juce::jlimit(0, 127, (int) std::round(*raw));
        // setSelectedId expects the item id we added (the MIDI note)
        filterKeycenterSelector.setSelectedId (currentNote, juce::dontSendNotification);
    }

    // When user changes the combo, update the parameter
    filterKeycenterSelector.onChange = [this]()
    {
        const int selectedId = filterKeycenterSelector.getSelectedId();
        if (selectedId <= 0)
            return;

        if (auto* param = audioProcessor.apvts.getParameter("filterKeycenter"))
        {
            param->setValueNotifyingHost(param->convertTo0to1((float) selectedId));
        }
    };

    // Attachments (ensure the parameter IDs exist in your APVTS)
    oscWaveAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "osc1Wave", oscWaveSelector);
    osc2WaveAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "osc2Wave", osc2WaveSelector);
    // Replace "filterKeycenter" with your actual parameter ID for the filter keycenter
    filterKeycenterAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filterKeycenter", filterKeycenterSelector);

    // --- Presets Combo (reutiliza el mismo LookAndFeel) ---
    presetSelector.setLookAndFeel(comboBoxLookAndFeel.get());

    // populate from processor programs
    const int numPrograms = audioProcessor.getNumPrograms();
    for (int i = 0; i < numPrograms; ++i)
        presetSelector.addItem(audioProcessor.getProgramName(i), i + 1);

    // colors / LAF
    presetSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(0x4F,0x6B,0x72));
    presetSelector.setColour(juce::ComboBox::textColourId,       juce::Colour::fromRGB(0xD9,0xD9,0xD9));

    addAndMakeVisible(presetSelector);

    // select current program
    presetSelector.setSelectedId(audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);

    // when user changes preset, tell the processor to change program
    presetSelector.onChange = [this]()
    {
        const int id = presetSelector.getSelectedId();
        if (id > 0)
            audioProcessor.setCurrentProgram(id - 1);
    };

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

    // Initialize toggle from parameter (safe guard if parameter exists)
    if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.apvts.getParameter("polyMode")))
    {
        const bool isPoly = (param->getIndex() == 1); // layout uses { "Mono", "Poly" }
        polyToggle.setToggleState(isPoly, juce::dontSendNotification);
        polyToggle.setButtonText(isPoly ? "Poly" : "Mono");

        // Clicking updates parameter (notify host)
        polyToggle.onClick = [this, param]()
        {
            const int cur = param->getIndex();
            const int next = (cur == 0) ? 1 : 0;
            param->setValueNotifyingHost(param->convertTo0to1(next));
            polyToggle.setButtonText(next == 1 ? "Poly" : "Mono");
        };

        // Register for parameter change callbacks to reflect external changes
        audioProcessor.apvts.addParameterListener("polyMode", this);
    }

    // NOTA: no uso Timer aquí; uso listener para actualizaciones en tiempo real.
}

AndesJXAudioProcessorEditor::~AndesJXAudioProcessorEditor()
{
    // Reset look-and-feel on each combo BEFORE destroying the LAF
    oscWaveSelector.setLookAndFeel(nullptr);
    osc2WaveSelector.setLookAndFeel(nullptr);
    filterKeycenterSelector.setLookAndFeel(nullptr);
    presetSelector.setLookAndFeel(nullptr);

    comboBoxLookAndFeel.reset();
    // Remove parameter listener and restore lookAndFeel pointer
    audioProcessor.apvts.removeParameterListener("polyMode", this);
    polyToggle.setLookAndFeel(nullptr);
    toggleLookAndFeel.reset();
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

    // Asegúrate de conservar tus posiciones existentes para los demás controles
}

void AndesJXAudioProcessorEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "polyMode")
    {
        // newValue viene en 0..1: convertimos a índice seguro
        if (auto* p = audioProcessor.apvts.getParameter(parameterID))
        {
            const int index = p->convertFrom0to1(newValue);
            const bool isPoly = (index == 1);

            // parameterChanged se llama desde hilo de audio -> usar callAsync para actualizar UI
            juce::MessageManager::callAsync([this, isPoly]()
            {
                polyToggle.setToggleState(isPoly, juce::dontSendNotification);
                polyToggle.setButtonText(isPoly ? "Poly" : "Mono");
            });
        }
    }
}

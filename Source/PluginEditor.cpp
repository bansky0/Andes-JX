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

        oscWaveSelector.addItem("Sine", 1);
        oscWaveSelector.addItem("Saw", 2);
        oscWaveSelector.addItem("Square", 3);
        oscWaveSelector.addItem("Triangle", 4);
        oscWaveSelector.addItem("PWM", 5);

        addAndMakeVisible(oscWaveSelector);

        oscWaveAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "osc1Wave", oscWaveSelector);

    }

AndesJXAudioProcessorEditor::~AndesJXAudioProcessorEditor()
{
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
}

void AndesJXAudioProcessorEditor::resized()
{
    oscWaveSelector.setBounds(60, 80, 120, 24);

}


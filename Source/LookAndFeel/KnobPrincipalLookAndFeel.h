/*
  ==============================================================================

    KnobPrincipalLookAndFeel.h
    Created: 1 Apr 2026 4:09:44pm
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class KnobPrincipalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    KnobPrincipalLookAndFeel()
    {
        knobImage = juce::ImageCache::getFromMemory(
            BinaryData::AndesKnobPrincipal2_png,
            BinaryData::AndesKnobPrincipal2_pngSize);
    }

    void drawRotarySlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float /*rotaryStartAngle*/,
        float /*rotaryEndAngle*/,
        juce::Slider& slider) override
    {
        if (!knobImage.isValid())
            return;

        const int frameSize = knobImage.getWidth();
        const int numFrames = knobImage.getHeight() / frameSize;

        const int frameIndex = juce::jlimit(
            0, numFrames - 1,
            juce::roundToInt(sliderPosProportional * static_cast<float>(numFrames - 1))
        );

        const int drawSize = juce::jmin(width, height);
        const int drawX = x + (width - drawSize) / 2;
        const int drawY = y + (height - drawSize) / 2;

        g.drawImage(knobImage,
            drawX, drawY, drawSize, drawSize,
            0, frameIndex * frameSize, frameSize, frameSize);

        // Valor en el centro del knob
        const juce::String valueText = slider.getTextFromValue(slider.getValue());

        g.setColour(juce::Colour::fromRGB(0xD9, 0xD9, 0xD9));
        g.setFont(juce::Font(juce::FontOptions(8.5f)));

        auto textBounds = juce::Rectangle<int>(drawX, drawY, drawSize, drawSize)
            .reduced(2, 2);

        g.drawFittedText(valueText,
            textBounds,
            juce::Justification::centred,
            1);
    }

private:
    juce::Image knobImage;
};
/*
  ==============================================================================

    SecondaryKnobLookAndFeel.h
    Created: 2 Apr 2026 3:36:06pm
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "AndesBaseLookAndFeel.h"

class SecondaryKnobLookAndFeel : public AndesBaseLookAndFeel
{
public:
    SecondaryKnobLookAndFeel()
    {
        knobImage = juce::ImageCache::getFromMemory(
            BinaryData::AndesKnobSecondary_png,
            BinaryData::AndesKnobSecondary_pngSize);
    }

    ~SecondaryKnobLookAndFeel() override = default;

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

        if (showValueText)
            drawKnobValueText(g, slider, drawX, drawY, drawSize);
    }

    void setTextFontHeight(float newHeight) noexcept { textFontHeight = newHeight; }
    void setShowValueText(bool shouldShow) noexcept { showValueText = shouldShow; }
    void setTextInset(int newInset) noexcept { textInset = newInset; }

private:
    void drawKnobValueText(juce::Graphics& g,
        juce::Slider& slider,
        int drawX, int drawY, int drawSize)
    {
        g.setColour(textColour());

        const auto valueText = slider.getTextFromValue(slider.getValue());

        auto textBounds = juce::Rectangle<int>(drawX, drawY, drawSize, drawSize)
            .reduced(textInset, textInset);

        g.setFont(juce::Font(juce::FontOptions(textFontHeight)));
        g.drawFittedText(valueText,
            textBounds,
            juce::Justification::centred,
            1);
    }

private:
    juce::Image knobImage;

    float textFontHeight{ 7.5f };
    bool showValueText{ true };
    int textInset{ 3 };
};
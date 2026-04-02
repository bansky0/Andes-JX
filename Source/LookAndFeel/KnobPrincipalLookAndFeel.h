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

        g.setColour(juce::Colour::fromRGB(0xD9, 0xD9, 0xD9));

        const auto id = slider.getComponentID();

        juce::String valueStr;
        juce::String unitStr;
        bool useTwoLineText = false;
        bool useOneLineCustom = false;

        if (id == "output")
        {
            const float value = slider.getValue();

            if (std::abs(value) < 0.05f)
                valueStr = "0";
            else if (value > 0.0f)
                valueStr = "+" + juce::String(value, 1);
            else
                valueStr = juce::String(value, 1);

            unitStr = "dB";
            useTwoLineText = true;
        }
        else if (id == "oscMix")
        {
            const float value = slider.getValue();

            const int osc1 = juce::roundToInt(100.0f - 0.5f * value);
            const int osc2 = juce::roundToInt(0.5f * value);

            valueStr = juce::String(osc1) + ":" + juce::String(osc2);

            useOneLineCustom = true;
        }
        else if (id == "filterReso" || id == "filterFreq")
        {
            valueStr = juce::String(juce::roundToInt(slider.getValue()));
            unitStr = "%";
            useTwoLineText = true;
        }

        if (useTwoLineText)
        {
            auto bounds = juce::Rectangle<float>(drawX, drawY, drawSize, drawSize).reduced(2.0f);

            auto centerY = bounds.getCentreY();
            float spacing = 3.0f;

            juce::Rectangle<float> valueArea(bounds.getX(), centerY - 10.0f, bounds.getWidth(), 12.0f);
            juce::Rectangle<float> unitArea(bounds.getX(), centerY + spacing, bounds.getWidth(), 8.0f);

            g.setFont(juce::Font(10.0f));
            g.drawText(valueStr, valueArea, juce::Justification::centred);

            g.setFont(juce::Font(7.5f));
            g.drawText(unitStr, unitArea, juce::Justification::centred);
        }
        else
        {
            const juce::String valueText = slider.getTextFromValue(slider.getValue());

            auto textBounds = juce::Rectangle<int>(drawX, drawY, drawSize, drawSize)
                .reduced(2, 2);

            g.setFont(juce::Font(juce::FontOptions(8.5f)));
            g.drawFittedText(valueText,
                textBounds,
                juce::Justification::centred,
                1);
        }
    }

private:
    juce::Image knobImage;
};
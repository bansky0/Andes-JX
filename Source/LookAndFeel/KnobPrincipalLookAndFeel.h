/*
  ==============================================================================

    KnobPrincipalLookAndFeel.h
    Created: 1 Apr 2026 4:09:44pm
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "AndesBaseLookAndFeel.h"

class KnobPrincipalLookAndFeel : public AndesBaseLookAndFeel
{
public:
    KnobPrincipalLookAndFeel()
    {
        knobImage = juce::ImageCache::getFromMemory(
            BinaryData::AndesKnobPrincipal2_png,
            BinaryData::AndesKnobPrincipal2_pngSize);
    }

    ~KnobPrincipalLookAndFeel() override = default;

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

        drawKnobValueText(g, slider, drawX, drawY, drawSize);
    }

    void setMainValueFontHeight(float newHeight) noexcept { mainValueFontHeight = newHeight; }
    void setUnitFontHeight(float newHeight) noexcept { unitFontHeight = newHeight; }
    void setSingleLineFontHeight(float newHeight) noexcept { singleLineFontHeight = newHeight; }

private:
    enum class TextLayout
    {
        singleLine,
        twoLine
    };

    void drawKnobValueText(juce::Graphics& g,
        juce::Slider& slider,
        int drawX, int drawY, int drawSize)
    {
        g.setColour(textColour());

        const auto layout = resolveTextLayout(slider);
        const auto valueText = resolvePrimaryText(slider);
        const auto unitText = resolveSecondaryText(slider);

        auto bounds = juce::Rectangle<float>(static_cast<float>(drawX),
            static_cast<float>(drawY),
            static_cast<float>(drawSize),
            static_cast<float>(drawSize)).reduced(2.0f);

        if (layout == TextLayout::twoLine)
        {
            const float centerY = bounds.getCentreY();
            const float spacing = 3.0f;

            juce::Rectangle<float> valueArea(bounds.getX(),
                centerY - 10.0f,
                bounds.getWidth(),
                12.0f);

            juce::Rectangle<float> unitArea(bounds.getX(),
                centerY + spacing,
                bounds.getWidth(),
                8.0f);

            g.setFont(juce::Font(juce::FontOptions(mainValueFontHeight)));
            g.drawText(valueText, valueArea, juce::Justification::centred);

            g.setFont(juce::Font(juce::FontOptions(unitFontHeight)));
            g.drawText(unitText, unitArea, juce::Justification::centred);
        }
        else
        {
            auto textBounds = juce::Rectangle<int>(drawX, drawY, drawSize, drawSize).reduced(2, 2);

            g.setFont(juce::Font(juce::FontOptions(singleLineFontHeight)));
            g.drawFittedText(valueText,
                textBounds,
                juce::Justification::centred,
                1);
        }
    }

    TextLayout resolveTextLayout(juce::Slider& slider) const
    {
        const auto id = slider.getComponentID();

        if (id == "output" || id == "filterReso" || id == "filterFreq")
            return TextLayout::twoLine;

        return TextLayout::singleLine;
    }

    juce::String resolvePrimaryText(juce::Slider& slider) const
    {
        const auto id = slider.getComponentID();
        const float value = static_cast<float>(slider.getValue());

        if (id == "output")
        {
            if (std::abs(value) < 0.05f)
                return "0";

            return value > 0.0f
                ? "+" + juce::String(value, 1)
                : juce::String(value, 1);
        }

        if (id == "oscMix")
        {
            const int osc2 = juce::roundToInt(value);
            const int osc1 = 100 - osc2;
            return juce::String(osc1) + ":" + juce::String(osc2);
        }

        if (id == "filterReso" || id == "filterFreq")
            return juce::String(juce::roundToInt(value));

        return slider.getTextFromValue(value);
    }

    juce::String resolveSecondaryText(juce::Slider& slider) const
    {
        const auto id = slider.getComponentID();

        if (id == "output")
            return "dB";

        if (id == "filterReso" || id == "filterFreq")
            return "%";

        return {};
    }

private:
    juce::Image knobImage;

    float mainValueFontHeight{ 10.0f };
    float unitFontHeight{ 7.5f };
    float singleLineFontHeight{ 8.5f };
};
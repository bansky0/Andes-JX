/*
  ==============================================================================

    FaderLookAndFeel.h
    Created: 8 Apr 2026 12:33:11pm
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class FaderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    FaderLookAndFeel(int frameW = 32, int frameH = 128)
        : frameWidth(frameW), frameHeight(frameH)
    {
        faderImage = juce::ImageCache::getFromMemory(
            BinaryData::AndesFaderADSR_png,
            BinaryData::AndesFaderADSR_pngSize);
    }

    void drawLinearSlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        const juce::Slider::SliderStyle style,
        juce::Slider& slider) override
    {
        juce::ignoreUnused(sliderPos, minSliderPos, maxSliderPos);

        if (!faderImage.isValid())
            return;

        if (style != juce::Slider::LinearVertical)
        {
            juce::LookAndFeel_V4::drawLinearSlider(
                g, x, y, width, height,
                sliderPos, minSliderPos, maxSliderPos, style, slider);
            return;
        }

        const int numFrames = faderImage.getHeight() / frameHeight;

        if (numFrames <= 0 || faderImage.getWidth() != frameWidth)
            return;

        const double proportion = slider.valueToProportionOfLength(slider.getValue());

        const int frameIndex = juce::jlimit(
            0,
            numFrames - 1,
            (int)std::round(proportion * (numFrames - 1))
        );

        g.drawImage(faderImage,
            x, y, width, height,
            0, frameIndex * frameHeight,
            frameWidth, frameHeight);
    }

private:
    juce::Image faderImage;
    int frameWidth = 32;
    int frameHeight = 128;
};
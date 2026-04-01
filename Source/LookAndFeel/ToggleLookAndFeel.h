/*
  ==============================================================================

    SegmentedButtonLookAndFeel.h
    Created: 31 March 2026
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class ToggleLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ToggleLookAndFeel() = default;

    enum ColourIds
    {
        backgroundOnColourId = 0x2001000,
        backgroundOffColourId = 0x2001001,
        textColourId = 0x2001002,
        outlineColourId = 0x2001003
    };

    void setDefaultToggleBackgroundOn(juce::Colour c) noexcept { bgOn = c; }
    void setDefaultToggleBackgroundOff(juce::Colour c) noexcept { bgOff = c; }
    void setDefaultToggleText(juce::Colour c) noexcept { text = c; }
    void setToggleFontHeight(float newHeight) noexcept { toggleFontHeight = newHeight; }

    void drawToggleButton(juce::Graphics&,
        juce::ToggleButton&,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;

    juce::Font getTextButtonFont(juce::TextButton&, int) override
    {
        return juce::Font(juce::FontOptions(toggleFontHeight));
    }

private:
    juce::Colour bgOn{ juce::Colour::fromRGB(0x4F, 0x6B, 0x72) };
    juce::Colour bgOff{ juce::Colour::fromRGB(0x3F, 0x55, 0x5B) };
    juce::Colour text{ juce::Colour::fromRGB(0xD9, 0xD9, 0xD9) };

    float toggleFontHeight{ 11.0f };
};
/*
  ==============================================================================

    SegmentedButtonLookAndFeel.h
    Created: 31 March 2026
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "AndesBaseLookAndFeel.h"
#include "AndesStyleHelpers.h"

class ToggleLookAndFeel : public AndesBaseLookAndFeel
{
public:
    ToggleLookAndFeel() = default;
    ~ToggleLookAndFeel() override = default;

    enum ColourIds
    {
        backgroundOnColourId = 0x2001000,
        backgroundOffColourId = 0x2001001,
        textColourId = 0x2001002,
        outlineColourId = 0x2001003
    };

    void setDefaultToggleBackgroundOn(juce::Colour c) noexcept { bgOnOverride = c; }
    void setDefaultToggleBackgroundOff(juce::Colour c) noexcept { bgOffOverride = c; }
    void setDefaultToggleText(juce::Colour c) noexcept { textOverride = c; }

    void setToggleFontHeight(float newHeight) noexcept { toggleFontHeight = newHeight; }
    void setCornerRadius(float newRadius) noexcept { cornerRadius = newRadius; }

    void drawToggleButton(juce::Graphics&,
        juce::ToggleButton&,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;

    juce::Font getTextButtonFont(juce::TextButton&, int) override
    {
        return AndesStyleHelpers::makeUIFont(toggleFontHeight);
    }

private:
    juce::Colour resolveBackgroundOn() const noexcept;
    juce::Colour resolveBackgroundOff() const noexcept;
    juce::Colour resolveTextColour() const noexcept;
    juce::Colour resolveOutlineColour() const noexcept;

private:
    std::optional<juce::Colour> bgOnOverride;
    std::optional<juce::Colour> bgOffOverride;
    std::optional<juce::Colour> textOverride;

    float toggleFontHeight{ fontMedium() };
    float cornerRadius{ smallRadius() };
};
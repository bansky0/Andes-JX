/*
  ==============================================================================

    AndesBaseLookAndFeel.h
    Created: 3 Apr 2026 12:10:14pm
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "AndesTheme.h"

class AndesBaseLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AndesBaseLookAndFeel() = default;
    virtual ~AndesBaseLookAndFeel() = default;

protected:
    juce::Colour panelColour() const noexcept { return AndesTheme::Colours::panel; }
    juce::Colour panelDarkColour() const noexcept { return AndesTheme::Colours::panelDark; }
    juce::Colour textColour() const noexcept { return AndesTheme::Colours::text; }
    juce::Colour outlineColour() const noexcept { return AndesTheme::Colours::outline; }

    float smallRadius() const noexcept { return AndesTheme::Metrics::cornerRadiusSmall; }
    float fontSmall() const noexcept { return AndesTheme::Fonts::small; }
    float fontMedium() const noexcept { return AndesTheme::Fonts::medium; }
    float fontTiny() const noexcept { return AndesTheme::Fonts::tiny; }
};
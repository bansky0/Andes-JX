/*
  ==============================================================================

    AndesTheme.h
    Created: 3 Apr 2026 12:06:51pm
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct AndesTheme
{
    struct Colours
    {
        static inline const juce::Colour panel = juce::Colour::fromRGB(0x4F, 0x6B, 0x72);
        static inline const juce::Colour panelDark = juce::Colour::fromRGB(0x3F, 0x55, 0x5B);
        static inline const juce::Colour text = juce::Colour::fromRGB(0xD9, 0xD9, 0xD9);
        static inline const juce::Colour outline = panel.darker(0.35f);
    };

    struct Metrics
    {
        static constexpr float cornerRadiusSmall = 2.0f;
        static constexpr float borderThickness = 1.0f;
        static constexpr float innerPadding = 1.5f;
        static constexpr float topGlossRatio = 0.45f;
    };

    struct Fonts
    {
        static constexpr float small = 8.5f;
        static constexpr float medium = 11.0f;
        static constexpr float tiny = 7.5f;
    };
};
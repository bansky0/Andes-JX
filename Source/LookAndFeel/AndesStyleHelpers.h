/*
  ==============================================================================

    AndesStyleHelpers.h
    Created: 3 Apr 2026 12:09:47pm
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "AndesTheme.h"

namespace AndesStyleHelpers
{
    inline juce::Font makeUIFont(float height)
    {
        juce::Font font(juce::FontOptions{ height });
        font.setTypefaceName("Arial");
        font.setBold(false);
        return font;
    }

    inline void drawPanel(juce::Graphics& g,
        juce::Rectangle<float> bounds,
        juce::Colour fill,
        float cornerRadius = AndesTheme::Metrics::cornerRadiusSmall,
        bool drawTopGloss = true)
    {
        const auto outline = AndesTheme::Colours::outline;

        g.setColour(fill);
        g.fillRoundedRectangle(bounds, cornerRadius);

        g.setColour(outline);
        g.drawRoundedRectangle(bounds, cornerRadius, AndesTheme::Metrics::borderThickness);

        if (drawTopGloss)
        {
            auto inner = bounds.reduced(AndesTheme::Metrics::innerPadding);
            auto topSection = inner.removeFromTop(bounds.getHeight() * AndesTheme::Metrics::topGlossRatio);

            g.setColour(fill.brighter(0.03f));
            g.fillRoundedRectangle(topSection, juce::jmax(1.0f, cornerRadius - 0.5f));
        }
    }

    inline juce::Colour applyInteractionState(juce::Colour base, bool isHover, bool isDown, bool isToggled = false)
    {
        auto c = base;

        if (isToggled)
            c = c.brighter(0.18f);

        if (isDown)
            c = c.darker(0.10f);
        else if (isHover)
            c = c.brighter(isToggled ? 0.03f : 0.06f);

        return c;
    }
}
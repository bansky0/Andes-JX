#pragma once
#include <JuceHeader.h>
#include "AndesBaseLookAndFeel.h"
#include "AndesStyleHelpers.h"

class SegmentedButtonLookAndFeel : public AndesBaseLookAndFeel
{
public:
    SegmentedButtonLookAndFeel() = default;
    ~SegmentedButtonLookAndFeel() override = default;

    void setDefaultBackground(juce::Colour c) noexcept { backgroundOverride = c; }
    void setDefaultText(juce::Colour c) noexcept { textOverride = c; }

    void setFontHeight(float newHeight) noexcept { fontHeight = newHeight; }
    void setCornerRadius(float newRadius) noexcept { cornerRadius = newRadius; }

    void drawButtonBackground(juce::Graphics& g,
        juce::Button& button,
        const juce::Colour& /*backgroundColour*/,
        bool isMouseOverButton,
        bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        const bool isToggled = button.getToggleState();

        auto fill = resolveBackgroundColour();
        fill = AndesStyleHelpers::applyInteractionState(fill,
            isMouseOverButton,
            isButtonDown,
            isToggled);

        AndesStyleHelpers::drawPanel(g, bounds, fill, cornerRadius, true);
    }

    void drawButtonText(juce::Graphics& g,
        juce::TextButton& button,
        bool /*isMouseOverButton*/,
        bool /*isButtonDown*/) override
    {
        auto area = button.getLocalBounds().reduced(6, 0);

        auto colour = resolveTextColour();

        if (!button.isEnabled())
            colour = colour.withAlpha(0.4f);
        else
            colour = colour.withAlpha(button.getToggleState() ? 1.0f : 0.85f);

        g.setColour(colour);
        g.setFont(AndesStyleHelpers::makeUIFont(fontHeight));
        g.drawFittedText(button.getButtonText(),
            area,
            juce::Justification::centred,
            1);
    }

private:
    juce::Colour resolveBackgroundColour() const noexcept
    {
        if (backgroundOverride.has_value())
            return *backgroundOverride;

        return panelColour();
    }

    juce::Colour resolveTextColour() const noexcept
    {
        if (textOverride.has_value())
            return *textOverride;

        return textColour();
    }

private:
    std::optional<juce::Colour> backgroundOverride;
    std::optional<juce::Colour> textOverride;

    float fontHeight{ fontMedium() };
    float cornerRadius{ smallRadius() };
};
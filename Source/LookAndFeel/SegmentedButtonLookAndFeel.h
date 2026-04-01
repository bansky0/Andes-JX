#pragma once
#include <JuceHeader.h>

class SegmentedButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SegmentedButtonLookAndFeel() = default;

    void setDefaultBackground(juce::Colour c) noexcept { background = c; }
    void setDefaultText(juce::Colour c) noexcept { text = c; }
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

        auto fill = background;
        const auto outline = background.darker(0.35f);

        if (isToggled)
            fill = background.brighter(0.18f);

        if (isButtonDown)
            fill = fill.darker(0.10f);
        else if (isMouseOverButton)
            fill = fill.brighter(isToggled ? 0.03f : 0.06f);

        // fondo exterior
        g.setColour(fill);
        g.fillRoundedRectangle(bounds, cornerRadius);

        // borde
        g.setColour(outline);
        g.drawRoundedRectangle(bounds, cornerRadius, 1.0f);

        // panel interno sutil para profundidad
        auto inner = bounds.reduced(1.5f);
        auto topSection = inner.removeFromTop(bounds.getHeight() * 0.45f);

        g.setColour(fill.brighter(isToggled ? 0.08f : 0.03f));
        g.fillRoundedRectangle(topSection, juce::jmax(1.0f, cornerRadius - 0.5f));
    }

    void drawButtonText(juce::Graphics& g,
        juce::TextButton& button,
        bool /*isMouseOverButton*/,
        bool /*isButtonDown*/) override
    {
        auto area = button.getLocalBounds().reduced(6, 0);

        g.setFont(juce::Font(juce::FontOptions(fontHeight)));

        auto textColour = text;

        if (!button.isEnabled())
            textColour = textColour.withAlpha(0.4f);
        else
            textColour = textColour.withAlpha(button.getToggleState() ? 1.0f : 0.85f);

        g.setColour(textColour);
        g.drawFittedText(button.getButtonText(),
            area,
            juce::Justification::centred,
            1);
    }

private:
    juce::Colour background{ juce::Colour::fromRGB(0x4F, 0x6B, 0x72) };
    juce::Colour text{ juce::Colour::fromRGB(0xD9, 0xD9, 0xD9) };

    float fontHeight{ 11.0f };
    float cornerRadius{ 2.0f };
};
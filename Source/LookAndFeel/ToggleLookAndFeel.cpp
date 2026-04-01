/*
  ==============================================================================

    SegmentedButtonLookAndFeel.h
    Created: 31 March 2026
    Author:  Jhonatan

  ==============================================================================
*/


#include "ToggleLookAndFeel.h"

void ToggleLookAndFeel::drawToggleButton(juce::Graphics& g,
    juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);

    const auto bgOnColour = isColourSpecified(backgroundOnColourId)
        ? findColour(backgroundOnColourId)
        : bgOn;

    const auto bgOffColour = isColourSpecified(backgroundOffColourId)
        ? findColour(backgroundOffColourId)
        : bgOff;

    const auto textColourResolved = isColourSpecified(textColourId)
        ? findColour(textColourId)
        : text;

    const auto outlineColour = isColourSpecified(outlineColourId)
        ? findColour(outlineColourId)
        : bgOn.darker(0.35f);

    auto fill = button.getToggleState() ? bgOnColour : bgOffColour;

    if (shouldDrawButtonAsDown)
        fill = fill.darker(0.10f);
    else if (shouldDrawButtonAsHighlighted)
        fill = fill.brighter(0.06f);

    // fondo exterior
    g.setColour(fill);
    g.fillRoundedRectangle(bounds, 2.0f);

    // borde
    g.setColour(outlineColour);
    g.drawRoundedRectangle(bounds, 2.0f, 1.0f);

    // panel interno sutil para dar profundidad
    auto inner = bounds.reduced(1.5f);

    g.setColour(fill.brighter(button.getToggleState() ? 0.08f : 0.03f));
    g.fillRoundedRectangle(inner.removeFromTop(bounds.getHeight() * 0.45f), 3.0f);

    // texto
    g.setColour(textColourResolved.withAlpha(button.getToggleState() ? 1.0f : 0.85f));
    g.setFont(juce::Font(juce::FontOptions(toggleFontHeight)));

    g.drawFittedText(button.getButtonText(),
        button.getLocalBounds().reduced(6, 0),
        juce::Justification::centred,
        1);
}
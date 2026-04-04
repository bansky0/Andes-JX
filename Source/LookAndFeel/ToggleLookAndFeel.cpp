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

    const bool isToggled = button.getToggleState();

    auto fill = isToggled ? resolveBackgroundOn()
        : resolveBackgroundOff();

    fill = AndesStyleHelpers::applyInteractionState(fill,
        shouldDrawButtonAsHighlighted,
        shouldDrawButtonAsDown,
        isToggled);

    AndesStyleHelpers::drawPanel(g, bounds, fill, cornerRadius, true);

    auto textCol = resolveTextColour();

    if (!button.isEnabled())
        textCol = textCol.withAlpha(0.4f);
    else
        textCol = textCol.withAlpha(isToggled ? 1.0f : 0.85f);

    g.setColour(textCol);
    g.setFont(juce::Font(juce::FontOptions(toggleFontHeight)));

    g.drawFittedText(button.getButtonText(),
        button.getLocalBounds().reduced(6, 0),
        juce::Justification::centred,
        1);
}

juce::Colour ToggleLookAndFeel::resolveBackgroundOn() const noexcept
{
    if (isColourSpecified(backgroundOnColourId))
        return findColour(backgroundOnColourId);

    if (bgOnOverride.has_value())
        return *bgOnOverride;

    return panelColour();
}

juce::Colour ToggleLookAndFeel::resolveBackgroundOff() const noexcept
{
    if (isColourSpecified(backgroundOffColourId))
        return findColour(backgroundOffColourId);

    if (bgOffOverride.has_value())
        return *bgOffOverride;

    return panelDarkColour();
}

juce::Colour ToggleLookAndFeel::resolveTextColour() const noexcept
{
    if (isColourSpecified(textColourId))
        return findColour(textColourId);

    if (textOverride.has_value())
        return *textOverride;

    return textColour();
}

juce::Colour ToggleLookAndFeel::resolveOutlineColour() const noexcept
{
    if (isColourSpecified(outlineColourId))
        return findColour(outlineColourId);

    return outlineColour();
}
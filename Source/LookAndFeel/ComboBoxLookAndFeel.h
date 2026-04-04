/*
  ==============================================================================
 
    ComboBoxlookAndFeel.h
    Created: 29 Mar 2026 12:19:14pm
    Author:  Jhonatan
 
  ==============================================================================
*/
 
#pragma once
#include <JuceHeader.h>
#include "AndesBaseLookAndFeel.h"
#include "AndesStyleHelpers.h"

class ComboBoxLookAndFeel : public AndesBaseLookAndFeel
{
public:
    ComboBoxLookAndFeel() = default;
    ~ComboBoxLookAndFeel() override = default;

    // Optional local overrides
    void setDefaultComboBackground(juce::Colour c) noexcept { comboBgOverride = c; }
    void setDefaultComboText(juce::Colour c) noexcept { comboTextOverride = c; }

    void setComboBoxFontHeight(float newHeight) noexcept { comboBoxFontHeight = newHeight; }
    void setPopupMenuFontHeight(float newHeight) noexcept { popupMenuFontHeight = newHeight; }
    void setCornerRadius(float newRadius) noexcept { cornerRadius = newRadius; }

    void drawComboBox(juce::Graphics& g,
        int width,
        int height,
        bool isButtonDown,
        int /*buttonX*/,
        int /*buttonY*/,
        int /*buttonW*/,
        int /*buttonH*/,
        juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<float>(0.0f, 0.0f,
            static_cast<float>(width),
            static_cast<float>(height)).reduced(0.5f);

        auto bg = resolveComboBackground(box);
        bg = AndesStyleHelpers::applyInteractionState(bg, box.isMouseOver(), isButtonDown, false);

        AndesStyleHelpers::drawPanel(g, bounds, bg, cornerRadius, true);
    }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        // Deja espacio a la derecha por si luego customizas flecha/arrow zone
        constexpr int leftPadding = 1;
        constexpr int rightPadding = 1;
        constexpr int topBottomPad = 1;

        label.setBounds(leftPadding,
            topBottomPad,
            box.getWidth() - leftPadding - rightPadding,
            box.getHeight() - (topBottomPad * 2));

        label.setFont(getComboBoxFont(box));
        label.setJustificationType(juce::Justification::centredLeft);
        label.setMinimumHorizontalScale(1.0f);

        const auto textCol = box.findColour(juce::ComboBox::textColourId, true);
        label.setColour(juce::Label::textColourId, textCol.isTransparent() ? resolveComboText(box) : textCol);
        label.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        label.setEditable(false, false, false);
    }

    juce::Font getComboBoxFont(juce::ComboBox& /*box*/) override
    {
        return juce::Font(juce::FontOptions(comboBoxFontHeight));
    }

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        auto area = juce::Rectangle<float>(0.0f, 0.0f,
            static_cast<float>(width),
            static_cast<float>(height));

        g.fillAll(resolvePopupBackground());
        g.setColour(outlineColour());
        g.drawRect(area.toNearestInt(), 1);
    }

    void drawPopupMenuItem(juce::Graphics& g,
        const juce::Rectangle<int>& area,
        bool isSeparator,
        bool isActive,
        bool isHighlighted,
        bool /*isTicked*/,
        bool /*hasSubMenu*/,
        const juce::String& text,
        const juce::String& /*shortcutKeyText*/,
        const juce::Drawable* /*icon*/,
        const juce::Colour* textColourToUse) override
    {
        if (isSeparator)
        {
            auto line = area.reduced(6, 0).withHeight(1).withY(area.getCentreY());
            g.setColour(outlineColour().withAlpha(0.7f));
            g.fillRect(line);
            return;
        }

        auto bg = resolvePopupBackground();
        auto fg = (textColourToUse != nullptr) ? *textColourToUse : resolvePopupText();

        if (!isActive)
            fg = fg.withAlpha(0.4f);

        g.setColour(bg);
        g.fillRect(area);

        if (isHighlighted)
        {
            g.setColour(resolvePopupText().withAlpha(0.12f));
            g.fillRect(area);
        }

        auto itemArea = area;
        /*
        if (isTicked)
        {
            auto tickArea = itemArea.removeFromLeft(14);
            g.setColour(fg);
            g.setFont(juce::Font(juce::FontOptions(popupMenuFontHeight)));
            g.drawText("•", tickArea, juce::Justification::centred);
        }
        */
        g.setColour(fg);
        g.setFont(juce::Font(juce::FontOptions(popupMenuFontHeight)));
        g.drawFittedText(text, area.reduced(8, 0), juce::Justification::centredLeft, 1);
    }

private:
    juce::Colour resolveComboBackground(juce::ComboBox& box) const noexcept
    {
        const auto boxBg = box.findColour(juce::ComboBox::backgroundColourId, true);

        if (!boxBg.isTransparent())
            return boxBg;

        if (comboBgOverride.has_value())
            return *comboBgOverride;

        return panelColour();
    }

    juce::Colour resolveComboText(juce::ComboBox& box) const noexcept
    {
        const auto boxText = box.findColour(juce::ComboBox::textColourId, true);

        if (!boxText.isTransparent())
            return boxText;

        if (comboTextOverride.has_value())
            return *comboTextOverride;

        return textColour();
    }

    juce::Colour resolvePopupBackground() const noexcept
    {
        return comboTextOverride.has_value() ? *comboTextOverride : textColour();
    }

    juce::Colour resolvePopupText() const noexcept
    {
        return comboBgOverride.has_value() ? *comboBgOverride : panelColour();
    }

private:
    std::optional<juce::Colour> comboBgOverride;
    std::optional<juce::Colour> comboTextOverride;

    float comboBoxFontHeight{ fontMedium() };
    float popupMenuFontHeight{ fontMedium() };
    float cornerRadius{ smallRadius() };
};
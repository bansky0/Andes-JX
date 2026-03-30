/*
  ==============================================================================
 
    ComboBoxlookAndFeel.h
    Created: 29 Mar 2026 12:19:14pm
    Author:  Jhonatan
 
  ==============================================================================
*/
 
#pragma once
#include <JuceHeader.h>
 
class ComboBoxLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ComboBoxLookAndFeel() = default;

    // allow editor to override defaults
    void setDefaultComboBackground(juce::Colour c) noexcept { comboBg = c; }
    void setDefaultComboText(juce::Colour c) noexcept       { comboText = c; }

    void setComboBoxFontHeight(float newHeight) noexcept    { comboBoxFontHeight = newHeight; }
    void setPopupMenuFontHeight(float newHeight) noexcept   { popupMenuFontHeight = newHeight; }

    // Popup uses inverted colors by default (can be overridden via setColour on the ComboBox)
    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        (void) width; (void) height;
        g.fillAll(popupBg());
    }

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool /*isActive*/, bool isHighlighted, bool /*isTicked*/,
                           bool /*hasSubMenu*/, const juce::String& text,
                           const juce::String& /*shortcutKeyText*/,
                           const juce::Drawable* /*icon*/,
                           const juce::Colour* textColourToUse) override
    {
        if (isSeparator)
        {
            juce::Rectangle<int> r(area.reduced(5, 0));
            g.setColour(juce::Colours::grey);
            g.fillRect(r.removeFromTop(1));
            return;
        }

        // background for the item (popupBg is already inverted)
        g.setColour(popupBg());
        g.fillRect(area);

        if (isHighlighted)
        {
            g.setColour(popupText().withAlpha(0.12f));
            g.fillRect(area);
        }

        const float availableHeight = (float) juce::jmax(1, area.getHeight() - 2);
        const float fontHeight = juce::jmin(popupMenuFontHeight, availableHeight);
        g.setFont(fontHeight);

        juce::Colour drawTextColour = (textColourToUse != nullptr) ? *textColourToUse : popupText();
        g.setColour(drawTextColour);
        g.drawFittedText(text, area.reduced(10, 0), juce::Justification::centredLeft, 1);
    }

    // Draw only background/outline. Do not draw the text.
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                      juce::ComboBox& box) override
    {
        juce::Rectangle<int> area(0, 0, width, height);

        const juce::Colour bg = box.findColour(juce::ComboBox::backgroundColourId, true).isTransparent()
                                ? comboBg : box.findColour(juce::ComboBox::backgroundColourId);
        g.setColour(bg);
        g.fillRect(area);

        g.setColour(bg.darker(0.35f));
        g.drawRect(area);

        if (isButtonDown)
        {
            g.setColour(bg.darker(0.08f));
            g.fillRect(area.reduced(1));
        }

        // Do NOT draw text here; the ComboBox Label will paint it (configured in positionComboBoxText).
    }

    // Configure internal Label: font, colours, padding.
    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds (1, 1,
                         box.getWidth() - 2,      // usar todo el ancho
                         box.getHeight() - 2);

        label.setFont (getComboBoxFont (box));

        // Asegurarse de que el Label use el color de texto del ComboBox (o el fallback del LAF)
        const juce::Colour textCol = box.findColour(juce::ComboBox::textColourId);
        label.setColour(juce::Label::textColourId, textCol);

        // transparencia en el fondo del label
        label.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

        // no editable por defecto
        label.setEditable(false, false);
    }

    juce::Font getComboBoxFont(juce::ComboBox& /*box*/) override
    {
        return juce::Font(comboBoxFontHeight);
    }

private:
    juce::Colour popupBg()  const noexcept { return comboText; } // inverted: popup bg = combo text
    juce::Colour popupText()const noexcept { return comboBg; }   // inverted: popup text = combo bg

    juce::Colour comboBg  { juce::Colour::fromRGB(0x4F, 0x6B, 0x72) }; // fallback #4F6B72
    juce::Colour comboText{ juce::Colour::fromRGB(0xD9, 0xD9, 0xD9) }; // fallback #D9D9D9

    float comboBoxFontHeight { 11.0f };
    float popupMenuFontHeight { 11.0f };
};
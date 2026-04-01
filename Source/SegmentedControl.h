#pragma once
#include <JuceHeader.h>

class SegmentedControl : public juce::Component
{
public:
    SegmentedControl()
    {
        addAndMakeVisible(buttonsContainer);
    }

    void setLookAndFeelForButtons(juce::LookAndFeel* laf)
    {
        buttonLookAndFeel = laf;

        for (auto* b : buttons)
            b->setLookAndFeel(buttonLookAndFeel);
    }

    void setItems(const juce::StringArray& items, int radioGroupId)
    {
        buttons.clear();
        ownedButtons.clear();
        buttonsContainer.removeAllChildren();

        for (int i = 0; i < items.size(); ++i)
        {
            auto button = std::make_unique<juce::TextButton>(items[i]);
            button->setClickingTogglesState(true);
            button->setRadioGroupId(radioGroupId);
            button->setLookAndFeel(buttonLookAndFeel);

            auto* raw = button.get();
            raw->onClick = [this, i]()
                {
                    selectedIndex = i;

                    if (onChange != nullptr)
                        onChange(selectedIndex);

                    repaint();
                };

            buttonsContainer.addAndMakeVisible(raw);
            buttons.push_back(raw);
            ownedButtons.push_back(std::move(button));
        }

        if (!buttons.empty())
        {
            buttons.front()->setToggleState(true, juce::dontSendNotification);
            selectedIndex = 0;
        }
        else
        {
            selectedIndex = -1;
        }

        resized();
        repaint();
    }

    void setSelectedIndex(int newIndex, juce::NotificationType notification = juce::sendNotification)
    {
        if (!juce::isPositiveAndBelow(newIndex, static_cast<int>(buttons.size())))
            return;

        for (int i = 0; i < static_cast<int>(buttons.size()); ++i)
            buttons[i]->setToggleState(i == newIndex, juce::dontSendNotification);

        selectedIndex = newIndex;

        if (notification == juce::sendNotification && onChange != nullptr)
            onChange(selectedIndex);

        repaint();
    }

    int getSelectedIndex() const noexcept
    {
        return selectedIndex;
    }

    juce::String getSelectedText() const
    {
        if (!juce::isPositiveAndBelow(selectedIndex, static_cast<int>(buttons.size())))
            return {};

        return buttons[static_cast<size_t>(selectedIndex)]->getButtonText();
    }

    std::function<void(int)> onChange;

    void resized() override
    {
        buttonsContainer.setBounds(getLocalBounds());

        auto area = buttonsContainer.getLocalBounds();
        const int numButtons = static_cast<int>(buttons.size());

        if (numButtons <= 0)
            return;

        const int segmentWidth = area.getWidth() / numButtons;
        int x = 0;

        for (int i = 0; i < numButtons; ++i)
        {
            const int w = (i == numButtons - 1) ? (area.getWidth() - x) : segmentWidth;
            buttons[i]->setBounds(x, 0, w, area.getHeight());
            x += w;
        }
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(0.5f);

        const auto bg = juce::Colour::fromRGB(0x4F, 0x6B, 0x72);
        const auto outline = bg.darker(0.35f);
        constexpr float cornerRadius = 2.0f;

        // fondo exterior redondeado
        g.setColour(bg);
        g.fillRoundedRectangle(bounds, cornerRadius);

        // borde exterior redondeado
        g.setColour(outline);
        g.drawRoundedRectangle(bounds, cornerRadius, 1.0f);

        // separadores internos
        for (int i = 1; i < static_cast<int>(buttons.size()); ++i)
        {
            const float x = static_cast<float>(buttons[i]->getX());
            g.drawVerticalLine(static_cast<int>(x), bounds.getY() + 1.0f, bounds.getBottom() - 1.0f);
        }
    }

private:
    juce::Component buttonsContainer;

    std::vector<std::unique_ptr<juce::TextButton>> ownedButtons;
    std::vector<juce::TextButton*> buttons;

    juce::LookAndFeel* buttonLookAndFeel = nullptr;
    int selectedIndex = -1;
};
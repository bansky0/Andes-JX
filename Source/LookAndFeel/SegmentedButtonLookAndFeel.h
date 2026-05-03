/*
  ==============================================================================

    SegmentedButtonLookAndFeel.h
    Created: 1 April 2026 11:52:58am
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: SegmentedButtonLookAndFeel
    Purpose:
        EN: Custom JUCE LookAndFeel for the SegmentedControl widget
            (see SegmentedControl.h). Renders each segment of the
            SVF / Moog filter selector as an individual rounded panel
            (drawn through AndesStyleHelpers) whose tone reacts to
            hover, press and toggled state through the standard
            interaction model.
        ES: LookAndFeel custom de JUCE para el widget SegmentedControl
            (ver SegmentedControl.h). Renderiza cada segmento del
            selector de filtro SVF / Moog como un panel redondeado
            individual (dibujado mediante AndesStyleHelpers) cuyo tono
            reacciona a hover, press y estado toggled mediante el
            modelo estándar de interacción.

    Architectural role:
        EN: Follows the canonical AndesJX LookAndFeel structure
            established by ToggleLookAndFeel, with two intentional
            simplifications:
              - No ColourIds enum / setColour API. The segmented
                control is internal to AndesJX and is not configured
                by external hosts, so the typed setDefaultX setters
                alone are enough. This keeps the surface area smaller
                and the file header-only.
              - Single background color. Unlike ToggleLookAndFeel
                (which keeps a separate "ON" and "OFF" fill), the
                segmented control distinguishes the active segment
                purely through the toggled-state brightness boost
                applied by applyInteractionState. Same base color +
                different brightness modulation = clear active vs
                inactive without doubling the palette.
        ES: Sigue la estructura canónica de LookAndFeel de AndesJX
            establecida por ToggleLookAndFeel, con dos simplificaciones
            intencionales:
              - Sin enum ColourIds / API setColour. El control
                segmentado es interno de AndesJX y no lo configuran
                hosts externos, así que basta con los setters tipados
                setDefaultX. Mantiene la superficie más chica y el
                archivo header-only.
              - Un solo color de fondo. A diferencia de
                ToggleLookAndFeel (que conserva un fill "ON" y otro
                "OFF" separados), el control segmentado distingue al
                segmento activo puramente mediante el boost de brillo
                del estado toggled que aplica applyInteractionState.
                Mismo color base + distinta modulación de brillo =
                activo vs inactivo claro sin duplicar la paleta.

    Notes:
        EN:
          - The override of drawButtonBackground and drawButtonText
            is split (instead of a single drawToggleButton like in the
            toggle) because SegmentedControl uses juce::TextButton
            internally, and JUCE routes background and text through
            separate methods for TextButton.
          - The visible rounded "frame" of the whole segmented control
            (the outer border + the inter-segment vertical separators)
            is drawn by SegmentedControl.cpp itself in its paint()
            method. This LookAndFeel only paints the per-segment fill
            and text; the unifying frame on top is the SegmentedControl
            component's job.
        ES:
          - El override de drawButtonBackground y drawButtonText va
            separado (en lugar de un único drawToggleButton como en el
            toggle) porque SegmentedControl usa juce::TextButton
            internamente, y JUCE enruta background y texto por métodos
            distintos para TextButton.
          - El "marco" redondeado visible de todo el control segmentado
            (el borde exterior + los separadores verticales entre
            segmentos) lo dibuja SegmentedControl.cpp por su cuenta en
            su método paint(). Este LookAndFeel solo pinta el relleno
            y el texto por segmento; el marco unificador encima es
            trabajo del componente SegmentedControl.
*/

#pragma once
#include <JuceHeader.h>
#include "AndesBaseLookAndFeel.h"
#include "AndesStyleHelpers.h"


class SegmentedButtonLookAndFeel : public AndesBaseLookAndFeel
{
public:
    SegmentedButtonLookAndFeel() = default;
    ~SegmentedButtonLookAndFeel() override = default;


    // ------------------------------------------------------------------------
    //  Default-color setters (typed alternative to setColour)
    //  Setters de color por defecto (alternativa tipada a setColour)
    // ------------------------------------------------------------------------

    // EN: The resolveX helpers below consult these overrides first
    //     and fall back to the theme defaults if not provided.
    // ES: Los helpers resolveX de abajo consultan estos overrides
    //     primero y caen al default del tema si no se proveen.
    void setDefaultBackground(juce::Colour c) noexcept { backgroundOverride = c; }
    void setDefaultText(juce::Colour c) noexcept { textOverride = c; }

    // EN: Geometry / typography setters.
    // ES: Setters de geometría / tipografía.
    void setFontHeight(float newHeight) noexcept { fontHeight = newHeight; }
    void setCornerRadius(float newRadius) noexcept { cornerRadius = newRadius; }


    // ------------------------------------------------------------------------
    //  Background drawing override / Override de dibujado de fondo
    // ------------------------------------------------------------------------

    // EN: JUCE calls this for each segment's background. The active
    //     segment is identified by button.getToggleState(); the
    //     resulting brightness boost (via applyInteractionState's
    //     toggled branch) is what tells the user which option is
    //     currently selected.
    //     The `backgroundColour` argument from JUCE is intentionally
    //     ignored: AndesJX always sources its colors from the theme,
    //     not from JUCE's per-button color injection.
    // ES: JUCE llama esto para el fondo de cada segmento. El segmento
    //     activo se identifica por button.getToggleState(); el boost
    //     de brillo resultante (vía la rama toggled de
    //     applyInteractionState) es lo que le dice al usuario qué
    //     opción está seleccionada actualmente.
    //     El argumento `backgroundColour` de JUCE se ignora a
    //     propósito: AndesJX siempre saca los colores del tema, no
    //     de la inyección de color por botón de JUCE.
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


    // ------------------------------------------------------------------------
    //  Text drawing override / Override de dibujado de texto
    // ------------------------------------------------------------------------

    // EN: Centered text with the same alpha-by-state grading used by
    //     the toggle: 40 % when disabled, 100 % when toggled,
    //     85 % otherwise. The 6-px horizontal padding keeps the text
    //     from touching the segment edges. Mouse hover/press flags
    //     are ignored here (they are already absorbed by the
    //     background's interaction modulation).
    // ES: Texto centrado con la misma gradación de alpha por estado
    //     que usa el toggle: 40 % cuando está deshabilitado, 100 %
    //     cuando está toggled, 85 % en otro caso. El padding
    //     horizontal de 6 px evita que el texto toque los bordes del
    //     segmento. Las flags de hover/press del mouse se ignoran
    //     aquí (ya las absorbió la modulación de interacción del
    //     fondo).
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
    // ------------------------------------------------------------------------
    //  Color resolvers / Resolvedores de color
    // ------------------------------------------------------------------------

    // EN: Two-tier priority (no setColour layer here, see notes
    //     above): setDefaultX override > theme default.
    // ES: Prioridad de dos niveles (sin capa setColour aquí, ver
    //     notas arriba): override setDefaultX > default del tema.
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
    // ------------------------------------------------------------------------
    //  Default-color overrides + geometric configuration
    //  Overrides de color por defecto + configuración geométrica
    // ------------------------------------------------------------------------

    std::optional<juce::Colour> backgroundOverride;
    std::optional<juce::Colour> textOverride;

    float fontHeight{ fontMedium() };
    float cornerRadius{ smallRadius() };
};
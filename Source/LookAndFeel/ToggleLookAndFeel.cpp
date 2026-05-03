/*
  ==============================================================================

    ToggleLookAndFeel.cpp
    Created: 31 March 2026
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: ToggleLookAndFeel (implementation)
    Purpose:
        EN: Implements the drawing override and color resolvers
            declared in ToggleLookAndFeel.h. Two halves:
              1. drawToggleButton: combines the active fill color
                 (state-aware), the AndesStyleHelpers panel primitive
                 and a centered text label.
              2. resolveX helpers: implement the three-tier color
                 priority (setColour > setDefault > theme).
        ES: Implementa el override de dibujo y los resolvedores de
            color declarados en ToggleLookAndFeel.h. Dos mitades:
              1. drawToggleButton: combina el color de relleno activo
                 (consciente del estado), la primitiva de panel de
                 AndesStyleHelpers y un label de texto centrado.
              2. Helpers resolveX: implementan la prioridad de color
                 en tres niveles (setColour > setDefault > tema).
*/


#include "ToggleLookAndFeel.h"


// ============================================================================
//  DRAW TOGGLE BUTTON / DIBUJADO DEL TOGGLE
// ============================================================================

// EN: JUCE calls this every time the toggle needs repainting. Five
//     steps:
//       1. Compute drawing bounds with a half-pixel inset so the
//          1-pixel outline drawn by drawPanel stays inside the
//          component's clip rectangle (otherwise the outline gets
//          clipped at the edges).
//       2. Pick the base fill color according to ON / OFF state.
//       3. Apply the standard interaction modulation
//          (hover/press/toggled) via AndesStyleHelpers.
//       4. Render the panel (rounded background + outline + top gloss).
//       5. Draw the centered button text. Disabled buttons fade to
//          40 % alpha; enabled buttons go to full alpha when toggled
//          and to 85 % when not, giving a visible-but-dimmer "OFF"
//          state.
// ES: JUCE llama esta función cada vez que el toggle necesita
//     repintarse. Cinco pasos:
//       1. Calcular los bounds de dibujo con un inset de medio píxel
//          para que el contorno de 1 píxel que dibuja drawPanel
//          quede dentro del rectángulo de clip del componente
//          (sino el contorno se recorta en los bordes).
//       2. Elegir el color base de relleno según el estado ON / OFF.
//       3. Aplicar la modulación estándar de interacción
//          (hover/press/toggled) vía AndesStyleHelpers.
//       4. Renderizar el panel (fondo redondeado + contorno + gloss
//          superior).
//       5. Dibujar el texto del botón centrado. Los botones
//          deshabilitados se atenúan al 40 % de alpha; los habilitados
//          van al alpha completo cuando están toggled y al 85 %
//          cuando no, dando un estado "OFF" visible-pero-más-tenue.
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
    g.setFont(AndesStyleHelpers::makeUIFont(toggleFontHeight));

    // EN: 6 px horizontal padding so the text never kisses the
    //     outline.
    // ES: Padding horizontal de 6 px para que el texto nunca toque
    //     el contorno.
    g.drawFittedText(button.getButtonText(),
        button.getLocalBounds().reduced(6, 0),
        juce::Justification::centred,
        1);
}


// ============================================================================
//  COLOR RESOLVERS / RESOLVEDORES DE COLOR
// ============================================================================

// EN: Each resolver implements the same three-tier priority:
//
//       1. setColour() (JUCE-idiomatic, by ColourId): if the editor
//          called setColour(backgroundOnColourId, ...), use that.
//       2. setDefaultX() override (typed setter): if the editor called
//          the typed setter, use the std::optional value.
//       3. Theme default: fall back to the AndesTheme color via the
//          base-class accessor.
//
//     This priority order means the host or runtime configuration
//     always wins (setColour), the editor's per-instance choice wins
//     next (setDefault), and the theme is the final safety net.
//
// ES: Cada resolver implementa la misma prioridad de tres niveles:
//
//       1. setColour() (idiomático de JUCE, por ColourId): si el
//          editor llamó setColour(backgroundOnColourId, ...), usar
//          ese.
//       2. Override setDefaultX() (setter tipado): si el editor llamó
//          al setter tipado, usar el valor del std::optional.
//       3. Default del tema: caer al color de AndesTheme vía el
//          accesor de la clase base.
//
//     Este orden de prioridad significa que la configuración del host
//     o en runtime siempre gana (setColour), la elección por
//     instancia del editor gana después (setDefault), y el tema es la
//     red de seguridad final.

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
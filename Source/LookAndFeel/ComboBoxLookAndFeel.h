/*
  ==============================================================================

    ComboBoxLookAndFeel.h
    Created: 29 Mar 2026 12:19:14pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: ComboBoxLookAndFeel
    Purpose:
        EN: Custom JUCE LookAndFeel for the four AndesJX combo boxes
            (oscWaveSelector, osc2WaveSelector, presetSelector,
            glideModeSelector). Renders the closed combo as a small
            rounded panel (drawn through AndesStyleHelpers) and the
            open popup menu as an INVERTED palette panel: where the
            closed combo has dark background and light text, the open
            popup has light background and dark text. This deliberate
            color flip makes the popup visually pop out from the
            interface without needing extra borders or shadows.
        ES: LookAndFeel custom de JUCE para los cuatro combo boxes de
            AndesJX (oscWaveSelector, osc2WaveSelector, presetSelector,
            glideModeSelector). Renderiza el combo cerrado como un
            pequeño panel redondeado (dibujado mediante
            AndesStyleHelpers) y el menú popup abierto como un panel
            de paleta INVERTIDA: donde el combo cerrado tiene fondo
            oscuro y texto claro, el popup abierto tiene fondo claro y
            texto oscuro. Este flip de color deliberado hace que el
            popup destaque visualmente sin necesitar bordes o sombras
            extra.

    Architectural role:
        EN: Follows the canonical AndesJX LookAndFeel structure (see
            ToggleLookAndFeel for the canonical reference). This is
            the most method-rich LookAndFeel of the project because
            JUCE splits the combo's appearance across five separate
            override points: drawComboBox (closed state),
            positionComboBoxText (text label inside the closed combo),
            getComboBoxFont (font selection), drawPopupMenuBackground
            (open menu container) and drawPopupMenuItem (each row of
            the open menu). All five must be coordinated to give a
            consistent look.
        ES: Sigue la estructura canónica de LookAndFeel de AndesJX
            (ver ToggleLookAndFeel como referencia canónica). Es el
            LookAndFeel con más métodos del proyecto porque JUCE
            divide la apariencia del combo en cinco overrides
            separados: drawComboBox (estado cerrado),
            positionComboBoxText (label de texto dentro del combo
            cerrado), getComboBoxFont (selección de fuente),
            drawPopupMenuBackground (contenedor del menú abierto) y
            drawPopupMenuItem (cada fila del menú abierto). Los cinco
            deben coordinarse para dar un look consistente.

    Notes:
        EN:
          - The popup palette is intentionally INVERTED with respect
            to the closed combo. resolvePopupBackground returns the
            CLOSED combo's TEXT color, and resolvePopupText returns
            the CLOSED combo's BACKGROUND color. This swap is what
            makes the popup feel like a separate, brighter surface
            without drawing additional decoration.
          - Color resolution uses the JUCE-idiomatic findColour with
            inheritance enabled (the second `true` argument), so a
            parent container can set a color that propagates to
            every combo child.
          - Three-tier color priority for the closed combo: the
            box's own setColour > setDefault override > theme.
            For the popup, only the setDefault override > theme path
            is consulted, because the popup is drawn outside the
            combo's component tree.
        ES:
          - La paleta del popup está intencionadamente INVERTIDA
            respecto al combo cerrado. resolvePopupBackground devuelve
            el color de TEXTO del combo CERRADO, y resolvePopupText
            devuelve el color de FONDO del combo CERRADO. Este swap
            es lo que hace que el popup se sienta como una superficie
            separada y más brillante sin dibujar decoración adicional.
          - La resolución de color usa el findColour idiomático de
            JUCE con herencia activada (el segundo argumento `true`),
            así un container padre puede asignar un color que se
            propaga a cada combo hijo.
          - Prioridad de color en tres niveles para el combo cerrado:
            el setColour del propio box > override setDefault > tema.
            Para el popup, solo se consulta el camino override
            setDefault > tema, porque el popup se dibuja fuera del
            árbol de componentes del combo.
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


    // ------------------------------------------------------------------------
    //  Default-color setters / Setters de color por defecto
    // ------------------------------------------------------------------------

    void setDefaultComboBackground(juce::Colour c) noexcept { comboBgOverride = c; }
    void setDefaultComboText(juce::Colour c) noexcept { comboTextOverride = c; }


    // ------------------------------------------------------------------------
    //  Geometry / typography setters
    //  Setters de geometría / tipografía
    // ------------------------------------------------------------------------

    void setComboBoxFontHeight(float newHeight) noexcept { comboBoxFontHeight = newHeight; }
    void setPopupMenuFontHeight(float newHeight) noexcept { popupMenuFontHeight = newHeight; }
    void setCornerRadius(float newRadius) noexcept { cornerRadius = newRadius; }


    // ------------------------------------------------------------------------
    //  drawComboBox / drawComboBox (closed state)
    // ------------------------------------------------------------------------

    // EN: Renders the closed combo as a small rounded panel. The
    //     button-position arguments (buttonX, buttonY, buttonW,
    //     buttonH) are ignored because AndesJX combos do not draw a
    //     separate dropdown arrow zone — the whole combo body is the
    //     clickable area.
    // ES: Renderiza el combo cerrado como un pequeño panel redondeado.
    //     Los argumentos de posición del botón (buttonX, buttonY,
    //     buttonW, buttonH) se ignoran porque los combos de AndesJX
    //     no dibujan una zona de flecha desplegable separada — todo
    //     el cuerpo del combo es el área clickeable.
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


    // ------------------------------------------------------------------------
    //  positionComboBoxText / Posicionamiento del label de texto
    // ------------------------------------------------------------------------

    // EN: Configures the internal Label that JUCE draws inside the
    //     closed combo: its bounds, font, justification and color.
    //     The 1-px padding on all sides leaves a small breathing
    //     space against the rounded panel border, and reserving
    //     symmetric left/right padding keeps room for a future arrow
    //     icon if AndesJX ever adds one.
    //
    //     Color resolution is dual-path: prefer an explicitly set
    //     ComboBox::textColourId if non-transparent, else fall back
    //     to resolveComboText (override > theme).
    //
    // ES: Configura el Label interno que JUCE dibuja dentro del combo
    //     cerrado: sus bounds, fuente, justificación y color. El
    //     padding de 1 px en todos los lados deja un pequeño respiro
    //     contra el borde del panel redondeado, y reservar padding
    //     simétrico izquierda/derecha mantiene espacio por si AndesJX
    //     añade una flecha en el futuro.
    //
    //     La resolución de color es por dos caminos: preferir un
    //     ComboBox::textColourId explícitamente seteado si no es
    //     transparente, sino caer a resolveComboText (override > tema).
    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        constexpr int leftPadding = 1;
        constexpr int rightPadding = 1;
        constexpr int topBottomPad = 1;

        label.setBounds(leftPadding,
            topBottomPad,
            box.getWidth() - leftPadding - rightPadding,
            box.getHeight() - (topBottomPad * 2));

        label.setFont(getComboBoxFont(box));
        label.setJustificationType(juce::Justification::centred);
        label.setMinimumHorizontalScale(1.0f);

        const auto textCol = box.findColour(juce::ComboBox::textColourId, true);
        label.setColour(juce::Label::textColourId,
            textCol.isTransparent() ? resolveComboText(box) : textCol);
        label.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        label.setEditable(false, false, false);
    }


    // ------------------------------------------------------------------------
    //  getComboBoxFont / Fuente del combo
    // ------------------------------------------------------------------------

    juce::Font getComboBoxFont(juce::ComboBox& /*box*/) override
    {
        return AndesStyleHelpers::makeUIFont(comboBoxFontHeight);
    }


    // ------------------------------------------------------------------------
    //  drawPopupMenuBackground / Fondo del menú popup
    // ------------------------------------------------------------------------

    // EN: Renders the open popup menu's container. Note the INVERTED
    //     palette: resolvePopupBackground returns what would be the
    //     closed combo's text color, producing a popup that contrasts
    //     visually with the rest of the GUI without needing a drop
    //     shadow. A 1-pixel rectangular outline (not rounded) frames
    //     the popup and reinforces the "different surface" feel.
    // ES: Renderiza el contenedor del menú popup abierto. Notar la
    //     paleta INVERTIDA: resolvePopupBackground devuelve lo que
    //     sería el color de texto del combo cerrado, produciendo un
    //     popup que contrasta visualmente con el resto de la GUI sin
    //     necesitar drop shadow. Un contorno rectangular de 1 píxel
    //     (no redondeado) enmarca el popup y refuerza la sensación
    //     de "superficie distinta".
    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        auto area = juce::Rectangle<float>(0.0f, 0.0f,
            static_cast<float>(width),
            static_cast<float>(height));

        g.fillAll(resolvePopupBackground());
        g.setColour(outlineColour());
        g.drawRect(area.toNearestInt(), 1);
    }


    // ------------------------------------------------------------------------
    //  drawPopupMenuItem / Item del menú popup
    // ------------------------------------------------------------------------

    // EN: Renders one row of the open popup menu. Three cases:
    //       - Separator: a thin centered horizontal line at 70 %
    //         outline alpha, used to break the menu into sections
    //         (e.g. between factory presets and the "Custom" entry).
    //       - Inactive item: foreground text faded to 40 % alpha to
    //         signal that it cannot be selected.
    //       - Highlighted item: an additional translucent overlay at
    //         12 % of the popup-text color shows the hover state.
    //     Text is left-aligned with 8-px horizontal padding, matching
    //     the visual rhythm of typical native popup menus.
    //
    //     The icon, shortcut and submenu arguments are unused because
    //     AndesJX popups are flat lists with no decorations.
    //
    // ES: Renderiza una fila del menú popup abierto. Tres casos:
    //       - Separator: una línea horizontal fina centrada al 70 %
    //         del alpha del outline, usada para dividir el menú en
    //         secciones (p. ej. entre los presets de fábrica y la
    //         entrada "Custom").
    //       - Item inactivo: texto en primer plano atenuado al 40 %
    //         de alpha para señalizar que no se puede seleccionar.
    //       - Item resaltado: un overlay translúcido extra al 12 %
    //         del color de texto del popup muestra el estado hover.
    //     El texto va alineado a la izquierda con padding horizontal
    //     de 8 px, que coincide con el ritmo visual de los menús
    //     popup nativos típicos.
    //
    //     Los argumentos icon, shortcut y submenu no se usan porque
    //     los popups de AndesJX son listas planas sin decoraciones.
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

        g.setColour(fg);
        g.setFont(AndesStyleHelpers::makeUIFont(popupMenuFontHeight));
        g.drawFittedText(text, area.reduced(8, 0), juce::Justification::centredLeft, 1);
    }


private:
    // ------------------------------------------------------------------------
    //  Color resolvers (closed combo) / Resolvedores de color (combo cerrado)
    // ------------------------------------------------------------------------

    // EN: Three-tier priority for the CLOSED combo:
    //       1. ComboBox::backgroundColourId set on the box itself
    //          (only honored if non-transparent).
    //       2. setDefaultComboBackground override.
    //       3. Theme's panel color.
    // ES: Prioridad de tres niveles para el combo CERRADO:
    //       1. ComboBox::backgroundColourId asignado en el box mismo
    //          (solo se honra si no es transparente).
    //       2. Override setDefaultComboBackground.
    //       3. Color panel del tema.
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


    // ------------------------------------------------------------------------
    //  Color resolvers (popup, INVERTED palette)
    //  Resolvedores de color (popup, paleta INVERTIDA)
    // ------------------------------------------------------------------------

    // EN: Deliberate inversion: the popup's BACKGROUND is the
    //     CLOSED combo's TEXT color, and the popup's TEXT is the
    //     CLOSED combo's BACKGROUND color. This is the design
    //     decision that makes the popup feel like a distinct
    //     surface (light background + dark text vs the GUI's dark
    //     panels + light text).
    //     Two-tier priority here (no setColour layer) because the
    //     popup is drawn outside the combo's component tree.
    // ES: Inversión deliberada: el FONDO del popup es el color de
    //     TEXTO del combo CERRADO, y el TEXTO del popup es el color
    //     de FONDO del combo CERRADO. Es la decisión de diseño que
    //     hace que el popup se sienta como una superficie distinta
    //     (fondo claro + texto oscuro vs los paneles oscuros + texto
    //     claro de la GUI).
    //     Prioridad de dos niveles aquí (sin capa setColour) porque
    //     el popup se dibuja fuera del árbol de componentes del
    //     combo.
    juce::Colour resolvePopupBackground() const noexcept
    {
        return comboTextOverride.has_value() ? *comboTextOverride : textColour();
    }

    juce::Colour resolvePopupText() const noexcept
    {
        return comboBgOverride.has_value() ? *comboBgOverride : panelColour();
    }


private:
    // ------------------------------------------------------------------------
    //  Default-color overrides + geometric configuration
    //  Overrides de color por defecto + configuración geométrica
    // ------------------------------------------------------------------------

    std::optional<juce::Colour> comboBgOverride;
    std::optional<juce::Colour> comboTextOverride;

    float comboBoxFontHeight{ fontMedium() };
    float popupMenuFontHeight{ fontMedium() };
    float cornerRadius{ smallRadius() };
};
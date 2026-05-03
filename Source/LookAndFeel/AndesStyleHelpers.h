/*
  ==============================================================================

    AndesStyleHelpers.h
    Created: 3 Apr 2026 12:09:47pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: AndesStyleHelpers
    Purpose:
        EN: Reusable drawing primitives shared by every AndesJX
            LookAndFeel. Sits one layer above AndesTheme: where the
            theme defines WHAT (colors, sizes, radii), this namespace
            defines HOW (paint a panel with the right gloss, build a
            UI font with the right typeface, derive interaction colors
            from a base).
        ES: Primitivas de dibujo reusables que comparten todos los
            LookAndFeel de AndesJX. Vive una capa por encima de
            AndesTheme: donde el tema define QUÉ (colores, tamaños,
            radios), este namespace define CÓMO (pintar un panel con
            el gloss correcto, construir una fuente de UI con el
            typeface correcto, derivar colores de interacción a partir
            de uno base).

    Architectural role:
        EN: Free-function namespace, all inline. Consumed by every
            LookAndFeel in this folder. By centralizing these drawing
            primitives here, all panels in the GUI share the exact
            same shape, gloss, border thickness and interaction
            response, regardless of which LookAndFeel paints them.
        ES: Namespace de funciones libres, todas inline. Las consume
            cada LookAndFeel de esta carpeta. Al centralizar estas
            primitivas de dibujo aquí, todos los paneles de la GUI
            comparten exactamente la misma forma, gloss, grosor de
            borde y respuesta a interacción, sin importar qué
            LookAndFeel los pinta.

    Notes:
        EN:
          - All functions are `inline` so the header can be included
            in many translation units without duplicate-symbol errors
            at link time (same rationale as the static inline values
            in AndesTheme.h).
          - The "gloss" effect is a soft brighter band on the upper
            half of every panel, evoking a glassy / illuminated
            surface. It is optional per call.
          - Hover and pressed states are derived numerically from a
            base color rather than stored as separate palette entries,
            so the GUI keeps responding consistently to interaction
            even after a theme tweak.
        ES:
          - Todas las funciones son `inline` para que el header pueda
            incluirse en muchas unidades de traducción sin errores de
            duplicate-symbol al linkear (misma razón que los valores
            static inline en AndesTheme.h).
          - El efecto "gloss" es una banda suave más clara en la mitad
            superior de cada panel, que evoca una superficie vidriada
            / iluminada. Es opcional por llamada.
          - Los estados hover y pressed se derivan numéricamente de un
            color base en lugar de guardarse como entradas separadas
            de paleta, así la GUI sigue respondiendo consistentemente
            a la interacción incluso tras un retoque del tema.
*/

#pragma once
#include <JuceHeader.h>
#include "AndesTheme.h"


namespace AndesStyleHelpers
{
    // ------------------------------------------------------------------------
    //  makeUIFont / makeUIFont
    // ------------------------------------------------------------------------

    // EN: Builds the standard AndesJX UI font at a given pixel height.
    //     Arial is used as the explicit typeface for cross-platform
    //     consistency: macOS, Windows and Linux all ship Arial (or a
    //     compatible substitute) so a preset rendered on one platform
    //     looks identical on the others.
    //     `setBold(false)` is explicit (not redundant) because some
    //     hosts pre-configure the default JUCE font as bold; forcing
    //     it false keeps every label uniform.
    // ES: Construye la fuente estándar de UI de AndesJX a un alto en
    //     píxeles dado. Se usa Arial como typeface explícito por
    //     consistencia entre plataformas: macOS, Windows y Linux
    //     entregan Arial (o un sustituto compatible) así que un preset
    //     renderizado en una plataforma se ve idéntico en las otras.
    //     `setBold(false)` es explícito (no redundante) porque algunos
    //     hosts pre-configuran la fuente JUCE por defecto como negrita;
    //     forzarla a false mantiene cada label uniforme.
    inline juce::Font makeUIFont(float height)
    {
        juce::Font font(juce::FontOptions{ height });
        font.setTypefaceName("Arial");
        font.setBold(false);
        return font;
    }


    // ------------------------------------------------------------------------
    //  drawPanel / drawPanel
    // ------------------------------------------------------------------------

    // EN: The signature panel-drawing primitive of AndesJX. Paints a
    //     rounded rectangle filled with `fill`, outlines it with the
    //     theme's outline color and standard border thickness, and
    //     optionally adds the AndesJX "top gloss": a slightly brighter
    //     band in the upper portion of the panel that gives every
    //     surface a soft illuminated look.
    //
    //     This single function is what makes combo boxes, toggles,
    //     segmented buttons and knobs FEEL like they belong to the
    //     same instrument: every panel is born from these few lines.
    //
    //     Parameters:
    //       fill          -> base color of the panel.
    //       cornerRadius  -> defaults to the theme's small radius;
    //                        override only when a control needs a
    //                        deliberately different shape.
    //       drawTopGloss  -> true by default; pass false for flat
    //                        surfaces (e.g. a depressed/active state
    //                        where the gloss would feel inconsistent).
    //
    //     Implementation note:
    //       The gloss rectangle is reduced by `innerPadding` so it
    //       sits visually inside the outline, and its corner radius
    //       is `cornerRadius - 0.5f` (clamped to >= 1) so the gloss
    //       follows the panel's outer curve without overshooting it.
    //
    // ES: Primitiva fundamental de dibujo de paneles de AndesJX.
    //     Pinta un rectángulo redondeado relleno con `fill`, lo
    //     contornea con el color outline del tema y el grosor de
    //     borde estándar, y opcionalmente añade el "top gloss" de
    //     AndesJX: una banda ligeramente más clara en la parte
    //     superior del panel que da a cada superficie un aspecto
    //     suavemente iluminado.
    //
    //     Esta única función es la que hace que combo boxes, toggles,
    //     botones segmentados y knobs SE SIENTAN parte del mismo
    //     instrumento: cada panel nace de estas pocas líneas.
    //
    //     Parámetros:
    //       fill          -> color base del panel.
    //       cornerRadius  -> por defecto el radio chico del tema;
    //                        sobrescribir solo cuando un control
    //                        necesite una forma deliberadamente
    //                        distinta.
    //       drawTopGloss  -> true por defecto; pasar false para
    //                        superficies planas (p. ej. un estado
    //                        hundido/activo donde el gloss se sentiría
    //                        inconsistente).
    //
    //     Nota de implementación:
    //       El rectángulo del gloss se reduce por `innerPadding` para
    //       sentarse visualmente dentro del contorno, y su radio de
    //       esquina es `cornerRadius - 0.5f` (limitado a >= 1) para
    //       que el gloss siga la curva exterior del panel sin
    //       desbordarse.
    inline void drawPanel(juce::Graphics& g,
        juce::Rectangle<float> bounds,
        juce::Colour fill,
        float cornerRadius = AndesTheme::Metrics::cornerRadiusSmall,
        bool  drawTopGloss = true)
    {
        const auto outline = AndesTheme::Colours::outline;

        // EN: Filled rounded background.
        // ES: Fondo redondeado relleno.
        g.setColour(fill);
        g.fillRoundedRectangle(bounds, cornerRadius);

        // EN: Outline. Theme's standard border thickness.
        // ES: Contorno. Grosor estándar de borde del tema.
        g.setColour(outline);
        g.drawRoundedRectangle(bounds, cornerRadius, AndesTheme::Metrics::borderThickness);

        // EN: Top gloss band. Slightly brighter than the fill, sized
        //     to the topGlossRatio fraction of the panel height.
        // ES: Banda gloss superior. Levemente más clara que el fill,
        //     dimensionada según la fracción topGlossRatio de la
        //     altura del panel.
        if (drawTopGloss)
        {
            auto inner = bounds.reduced(AndesTheme::Metrics::innerPadding);
            auto topSection = inner.removeFromTop(bounds.getHeight() * AndesTheme::Metrics::topGlossRatio);

            g.setColour(fill.brighter(0.03f));
            g.fillRoundedRectangle(topSection, juce::jmax(1.0f, cornerRadius - 0.5f));
        }
    }


    // ------------------------------------------------------------------------
    //  applyInteractionState / applyInteractionState
    // ------------------------------------------------------------------------

    // EN: Returns a color derived from `base` according to the user's
    //     interaction state (hover, pressed, toggled). The mapping is:
    //
    //       toggled = true  -> brighter(0.18f)   (visibly "ON")
    //       isDown          -> darker(0.10f)     (pressed)
    //       isHover         -> brighter(0.06f)   (or 0.03f if toggled,
    //                                              to avoid overdoing
    //                                              an already-bright
    //                                              surface)
    //
    //     Order of application matters: toggled first, then down/hover
    //     on top. A toggled+down button is "toggled then pressed"
    //     (brighter then darker), which produces a visible click
    //     feedback even on already-active controls.
    //
    //     Centralizing this here means every interactive element in
    //     the GUI reacts to mouse the same way. If we ever want
    //     stronger or softer feedback, this is a one-line edit.
    //
    // ES: Devuelve un color derivado de `base` según el estado de
    //     interacción del usuario (hover, pressed, toggled). El mapeo
    //     es:
    //
    //       toggled = true  -> brighter(0.18f)   (visiblemente "ON")
    //       isDown          -> darker(0.10f)     (pulsado)
    //       isHover         -> brighter(0.06f)   (o 0.03f si toggled,
    //                                              para no exagerar
    //                                              una superficie ya
    //                                              brillante)
    //
    //     El orden de aplicación importa: primero toggled, luego
    //     down/hover encima. Un botón toggled+down es "toggled luego
    //     pulsado" (más claro luego más oscuro), lo que produce un
    //     feedback visible de clic incluso en controles ya activos.
    //
    //     Centralizar esto aquí hace que cada elemento interactivo de
    //     la GUI reaccione al mouse de la misma manera. Si alguna vez
    //     quisiéramos feedback más fuerte o más suave, sería una
    //     edición de una sola línea.
    inline juce::Colour applyInteractionState(juce::Colour base,
        bool isHover,
        bool isDown,
        bool isToggled = false)
    {
        auto c = base;

        if (isToggled)
            c = c.brighter(0.18f);

        if (isDown)
            c = c.darker(0.10f);
        else if (isHover)
            c = c.brighter(isToggled ? 0.03f : 0.06f);

        return c;
    }
}
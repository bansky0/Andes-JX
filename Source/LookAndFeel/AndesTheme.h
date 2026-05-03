/*
  ==============================================================================

    AndesTheme.h
    Created: 3 Apr 2026 12:06:51pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: AndesTheme
    Purpose:
        EN: Single source of truth for the visual identity of AndesJX.
            Centralizes the color palette, geometric metrics, and font
            sizes used across every LookAndFeel and the PluginEditor.
            Changing the palette or the corner radii of the whole
            plugin happens here, in one place.
        ES: Única fuente de verdad para la identidad visual de AndesJX.
            Centraliza la paleta de colores, las métricas geométricas y
            los tamaños de fuente que usan todos los LookAndFeel y el
            PluginEditor. Cambiar la paleta o los radios de esquina del
            plugin entero ocurre aquí, en un solo lugar.

    Architectural role:
        EN: Pure constants header. Has no behavior, only static inline
            const data. Every other UI file in the project depends on
            it (LookAndFeels, SegmentedControl, PluginEditor),
            consuming the values as AndesTheme::Colours::xxx,
            AndesTheme::Metrics::xxx, AndesTheme::Fonts::xxx.
            Because all values are static inline, including this header
            in many translation units does NOT cause duplicate-symbol
            errors at link time.
        ES: Header de constantes puras. No tiene comportamiento, solo
            datos const static inline. Todo el resto de archivos de UI
            del proyecto dependen de él (LookAndFeels, SegmentedControl,
            PluginEditor), consumiendo los valores como
            AndesTheme::Colours::xxx, AndesTheme::Metrics::xxx,
            AndesTheme::Fonts::xxx. Como todos los valores son static
            inline, incluir este header en muchas unidades de
            traducción NO genera errores de duplicate-symbol al
            linkear.

    Notes:
        EN:
          - The palette is intentionally narrow (5 colors) to keep
            the GUI visually consistent. Adding a sixth should be a
            deliberate decision, not a quick fix; many designs are
            stronger when constrained.
          - All numeric metrics live here so a future visual revision
            (e.g. moving from 2px corners to 3px) is a one-line edit
            that reflows the entire UI uniformly.
          - Font sizes are pixel heights, not point sizes (juce::Font
            ctor convention).
        ES:
          - La paleta es intencionadamente reducida (5 colores) para
            mantener la GUI visualmente consistente. Añadir un sexto
            debería ser una decisión deliberada, no un parche rápido;
            muchos diseños se vuelven más fuertes cuando se limitan.
          - Todas las métricas numéricas viven aquí para que una
            revisión visual futura (p. ej. pasar de esquinas de 2 px
            a 3 px) sea una edición de una sola línea que se propaga
            uniformemente por toda la UI.
          - Los tamaños de fuente son altos en píxeles, no en puntos
            (convención del ctor de juce::Font).
*/

#pragma once
#include <JuceHeader.h>


// ============================================================================
//  ANDES THEME / TEMA ANDES
// ============================================================================

struct AndesTheme
{
    // ------------------------------------------------------------------------
    //  Colours / Colores
    // ------------------------------------------------------------------------

    // EN: AndesJX visual identity. The palette evokes the slate-blue
    //     greys of high-altitude Andean stone (panel) against a softly
    //     desaturated text white. Every UI element in the plugin pulls
    //     its color from here.
    // ES: Identidad visual de AndesJX. La paleta evoca los grises
    //     azulados de la piedra andina de altura (panel) contra un
    //     blanco de texto suavemente desaturado. Cada elemento de UI
    //     del plugin saca su color de aquí.
    struct Colours
    {
        // EN: Main panel background. Slate blue-grey.
        // ES: Fondo principal de paneles. Azul-gris pizarra.
        static inline const juce::Colour panel = juce::Colour::fromRGB(0x4F, 0x6B, 0x72);

        // EN: Darker panel variant for inactive states (toggle off,
        //     disabled controls).
        // ES: Variante oscura del panel para estados inactivos (toggle
        //     apagado, controles deshabilitados).
        static inline const juce::Colour panelDark = juce::Colour::fromRGB(0x3F, 0x55, 0x5B);

        // EN: Foreground text color. Soft white (not pure 0xFFFFFF) to
        //     reduce contrast fatigue against the slate panels.
        // ES: Color del texto en primer plano. Blanco suave (no 0xFFFFFF
        //     puro) para reducir fatiga de contraste contra los paneles
        //     pizarra.
        static inline const juce::Colour text = juce::Colour::fromRGB(0xD9, 0xD9, 0xD9);

        // EN: Muted text variant at 60 % opacity. Used for section
        //     titles, captions and any de-emphasized label.
        // ES: Variante atenuada del texto al 60 % de opacidad. Para
        //     títulos de sección, captions y cualquier label menos
        //     prominente.
        static inline const juce::Colour textMuted = text.withAlpha(0.6f);

        // EN: Border / outline color, derived from panel by darkening
        //     35 %. Keeping it derived (not hardcoded) means a single
        //     palette swap on `panel` propagates correctly to every
        //     border in the GUI.
        // ES: Color de borde / contorno, derivado de panel oscureciendo
        //     un 35 %. Mantenerlo derivado (no hardcodeado) hace que un
        //     único cambio de paleta sobre `panel` se propague
        //     correctamente a cada borde de la GUI.
        static inline const juce::Colour outline = panel.darker(0.35f);
    };


    // ------------------------------------------------------------------------
    //  Metrics / Métricas
    // ------------------------------------------------------------------------

    // EN: Geometric constants. Centralizing them here means a single
    //     edit can reflow the entire visual style of the plugin.
    // ES: Constantes geométricas. Centralizarlas aquí significa que una
    //     sola edición puede reorganizar todo el estilo visual del
    //     plugin.
    struct Metrics
    {
        // EN: Default rounded-corner radius for buttons, segments and
        //     panels.
        // ES: Radio por defecto de esquinas redondeadas para botones,
        //     segmentos y paneles.
        static constexpr float cornerRadiusSmall = 2.0f;

        // EN: Standard border thickness used by every outlined element.
        // ES: Grosor estándar de borde que usa cada elemento con
        //     contorno.
        static constexpr float borderThickness = 1.0f;

        // EN: Padding between an element's outer border and its inner
        //     content.
        // ES: Padding entre el borde exterior de un elemento y su
        //     contenido interno.
        static constexpr float innerPadding = 1.5f;

        // EN: Vertical fraction (0..1) of an element occupied by its
        //     top "gloss" highlight. 0.45 means the gloss covers
        //     roughly the upper 45 % of the surface, giving a soft
        //     glassy / illuminated look.
        // ES: Fracción vertical (0..1) de un elemento ocupada por su
        //     "gloss" superior. 0.45 significa que el gloss cubre
        //     aproximadamente el 45 % superior de la superficie,
        //     dando un aspecto suavemente vidriado / iluminado.
        static constexpr float topGlossRatio = 0.45f;
    };


    // ------------------------------------------------------------------------
    //  Fonts / Fuentes
    // ------------------------------------------------------------------------

    // EN: Font heights in pixels (juce::Font ctor convention). Three
    //     tiers are enough to cover every text role in the GUI:
    //       - tiny:   value labels next to secondary knobs and ADSR
    //                 letters
    //       - small:  combo box items, toggle text
    //       - medium: section titles
    //     Adding a fourth size should be resisted; if a control needs
    //     a different font weight or style, prefer a different
    //     typeface variant before introducing a new size.
    // ES: Altos de fuente en píxeles (convención del ctor de
    //     juce::Font). Tres niveles bastan para cubrir cada rol de
    //     texto en la GUI:
    //       - tiny:   labels de valor junto a knobs secundarios y
    //                 letras ADSR
    //       - small:  items de combo box, texto del toggle
    //       - medium: títulos de sección
    //     Habría que resistir añadir un cuarto tamaño; si un control
    //     necesita peso o estilo diferente, preferir una variante
    //     distinta de typeface antes que introducir un nuevo tamaño.
    struct Fonts
    {
        static constexpr float small = 8.5f;
        static constexpr float medium = 11.0f;
        static constexpr float tiny = 7.5f;
    };
};
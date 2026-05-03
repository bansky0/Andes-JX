/*
  ==============================================================================

    ToggleLookAndFeel.h
    Created: 31 March 2026
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: ToggleLookAndFeel (header)
    Purpose:
        EN: Custom JUCE LookAndFeel for the polyphony Mono / Poly toggle.
            Renders a TextButton-style toggle as a small rounded panel
            (drawn through AndesStyleHelpers) whose fill color
            distinguishes the ON / OFF state and reacts to hover and
            press through the standard interaction model.
        ES: LookAndFeel custom de JUCE para el toggle de polifonía
            Mono / Poly. Renderiza un toggle estilo TextButton como un
            pequeño panel redondeado (dibujado mediante
            AndesStyleHelpers) cuyo color de fondo distingue el estado
            ON / OFF y reacciona a hover y press mediante el modelo
            estándar de interacción.

    Architectural role:
        EN: First concrete subclass of AndesBaseLookAndFeel in this
            folder. Establishes the canonical structure that every
            other AndesJX LookAndFeel follows:
              - Inherit from AndesBaseLookAndFeel (gets theme accessors).
              - Declare a ColourIds enum with custom IDs that the
                editor can override per instance via setColour.
              - Provide setDefaultX setters as a programmatic
                alternative to setColour.
              - Provide private resolveX helpers that pick the active
                color in priority order: setColour > setDefault > theme.
              - Override the relevant juce::LookAndFeel_V4 methods.
        ES: Primera subclase concreta de AndesBaseLookAndFeel en esta
            carpeta. Establece la estructura canónica que sigue cada
            otro LookAndFeel de AndesJX:
              - Heredar de AndesBaseLookAndFeel (obtiene los accesores
                del tema).
              - Declarar un enum ColourIds con IDs custom que el editor
                puede overridear por instancia vía setColour.
              - Proveer setters setDefaultX como alternativa
                programática a setColour.
              - Proveer helpers private resolveX que eligen el color
                activo en orden de prioridad: setColour > setDefault >
                tema.
              - Overridear los métodos relevantes de
                juce::LookAndFeel_V4.

    Notes:
        EN:
          - The dual configuration system (setColour + setDefault) is
            deliberate: setColour is the JUCE-idiomatic way to
            customize a LookAndFeel from outside, while setDefault is
            kept for places in the editor that prefer a typed setter
            over an untyped color ID lookup.
          - The custom ColourIds start at 0x2001000 to avoid colliding
            with JUCE's own reserved color ID space.
          - getTextButtonFont is overridden so that even if a
            different code path renders the toggle through the
            TextButton path, the font stays consistent with what
            drawToggleButton uses.
        ES:
          - El sistema dual de configuración (setColour + setDefault)
            es deliberado: setColour es la forma idiomática de JUCE de
            personalizar un LookAndFeel desde afuera, mientras que
            setDefault se mantiene para sitios en el editor que
            prefieren un setter tipado en lugar de un lookup por ID
            de color sin tipo.
          - Los ColourIds custom empiezan en 0x2001000 para evitar
            choques con el espacio de IDs de color reservado por
            JUCE.
          - Se overridea getTextButtonFont para que aunque algún otro
            camino de código renderice el toggle por la ruta de
            TextButton, la fuente se mantenga consistente con la que
            usa drawToggleButton.
*/

#pragma once
#include <JuceHeader.h>
#include "AndesBaseLookAndFeel.h"
#include "AndesStyleHelpers.h"


class ToggleLookAndFeel : public AndesBaseLookAndFeel
{
public:
    ToggleLookAndFeel() = default;
    ~ToggleLookAndFeel() override = default;


    // ------------------------------------------------------------------------
    //  Colour IDs / IDs de color
    // ------------------------------------------------------------------------

    // EN: Custom JUCE color identifiers. The editor uses these with
    //     setColour() to override the toggle palette per instance.
    //     The 0x2001000 base is arbitrary but stays clear of JUCE's
    //     own reserved ranges.
    // ES: Identificadores de color custom de JUCE. El editor los usa
    //     con setColour() para sobrescribir la paleta del toggle por
    //     instancia. La base 0x2001000 es arbitraria pero se mantiene
    //     fuera de los rangos reservados por JUCE.
    enum ColourIds
    {
        backgroundOnColourId = 0x2001000,
        backgroundOffColourId = 0x2001001,
        textColourId = 0x2001002,
        outlineColourId = 0x2001003
    };


    // ------------------------------------------------------------------------
    //  Default-color setters (typed alternative to setColour)
    //  Setters de color por defecto (alternativa tipada a setColour)
    // ------------------------------------------------------------------------

    // EN: Programmatic alternative to setColour(). The resolveX
    //     helpers in the .cpp consult setColour first, then these
    //     overrides, and only fall back to the theme defaults if
    //     neither was provided.
    // ES: Alternativa programática a setColour(). Los helpers resolveX
    //     del .cpp consultan setColour primero, luego estos overrides,
    //     y solo caen al default del tema si ninguno fue provisto.
    void setDefaultToggleBackgroundOn(juce::Colour c) noexcept { bgOnOverride = c; }
    void setDefaultToggleBackgroundOff(juce::Colour c) noexcept { bgOffOverride = c; }
    void setDefaultToggleText(juce::Colour c) noexcept { textOverride = c; }

    // EN: Geometry / typography setters.
    // ES: Setters de geometría / tipografía.
    void setToggleFontHeight(float newHeight) noexcept { toggleFontHeight = newHeight; }
    void setCornerRadius(float newRadius) noexcept { cornerRadius = newRadius; }


    // ------------------------------------------------------------------------
    //  Drawing overrides / Overrides de dibujo
    // ------------------------------------------------------------------------

    void drawToggleButton(juce::Graphics&,
        juce::ToggleButton&,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;

    // EN: Inline override for the TextButton font. JUCE may use this
    //     path internally when drawing the button text outside of
    //     drawToggleButton; returning the same font keeps everything
    //     consistent regardless of which code path JUCE picks.
    // ES: Override inline para la fuente de TextButton. JUCE puede
    //     usar esta ruta internamente al dibujar el texto del botón
    //     fuera de drawToggleButton; devolver la misma fuente
    //     mantiene todo consistente sin importar qué ruta de código
    //     elija JUCE.
    juce::Font getTextButtonFont(juce::TextButton&, int) override
    {
        return AndesStyleHelpers::makeUIFont(toggleFontHeight);
    }


private:
    // ------------------------------------------------------------------------
    //  Color resolvers / Resolvedores de color
    // ------------------------------------------------------------------------

    // EN: Helper functions implemented in the .cpp. Each one returns
    //     the active color for one role, picking from setColour first,
    //     then the setDefault override, then the theme default.
    // ES: Funciones helper implementadas en el .cpp. Cada una devuelve
    //     el color activo para un rol, eligiendo de setColour primero,
    //     luego del override setDefault, luego del default del tema.
    juce::Colour resolveBackgroundOn()  const noexcept;
    juce::Colour resolveBackgroundOff() const noexcept;
    juce::Colour resolveTextColour()    const noexcept;
    juce::Colour resolveOutlineColour() const noexcept;


private:
    // ------------------------------------------------------------------------
    //  Default-color overrides + geometric configuration
    //  Overrides de color por defecto + configuración geométrica
    // ------------------------------------------------------------------------

    // EN: std::optional lets us distinguish "no override set" (empty)
    //     from "override set to a specific color" (engaged), without
    //     having to invent a sentinel color value.
    // ES: std::optional permite distinguir "sin override asignado"
    //     (vacío) de "override asignado a un color específico"
    //     (presente), sin tener que inventar un valor de color
    //     centinela.
    std::optional<juce::Colour> bgOnOverride;
    std::optional<juce::Colour> bgOffOverride;
    std::optional<juce::Colour> textOverride;

    // EN: Default geometry pulled from the theme. The setters above
    //     replace these per-instance.
    // ES: Geometría por defecto traída del tema. Los setters de arriba
    //     las reemplazan por instancia.
    float toggleFontHeight{ fontMedium() };
    float cornerRadius{ smallRadius() };
};
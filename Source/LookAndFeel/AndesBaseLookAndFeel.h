/*
  ==============================================================================

    AndesBaseLookAndFeel.h
    Created: 3 Apr 2026 12:10:14pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: AndesBaseLookAndFeel
    Purpose:
        EN: Common base class for every LookAndFeel in AndesJX. Inherits
            from juce::LookAndFeel_V4 (the modern JUCE rendering base)
            and exposes a small set of protected accessor methods that
            wrap AndesTheme values. Subclasses get a uniform, slightly
            more readable interface to the theme without each one
            having to spell out `AndesTheme::Colours::xxx` every time.
        ES: Clase base común para todos los LookAndFeel de AndesJX.
            Hereda de juce::LookAndFeel_V4 (la base de renderizado
            moderna de JUCE) y expone un pequeño conjunto de métodos
            accesores protected que envuelven valores de AndesTheme.
            Las subclases obtienen una interfaz uniforme y un poco más
            legible al tema sin que cada una tenga que escribir
            `AndesTheme::Colours::xxx` cada vez.

    Architectural role:
        EN: Sits between juce::LookAndFeel_V4 and the six concrete
            AndesJX LookAndFeels (ComboBox, Toggle, SegmentedButton,
            KnobPrincipal, SecondaryKnob, Fader). Provides no virtual
            overrides itself: every override lives in the concrete
            subclasses. This class exists only to centralize theme
            access and to give the type system a single anchor point
            for "every AndesJX LookAndFeel".
        ES: Vive entre juce::LookAndFeel_V4 y los seis LookAndFeel
            concretos de AndesJX (ComboBox, Toggle, SegmentedButton,
            KnobPrincipal, SecondaryKnob, Fader). No provee overrides
            virtuales por sí misma: cada override vive en las
            subclases concretas. Esta clase existe solo para
            centralizar el acceso al tema y darle al sistema de tipos
            un punto de anclaje único para "cualquier LookAndFeel de
            AndesJX".

    Notes:
        EN:
          - The accessors are `noexcept` and return by value because
            they only forward references to constexpr / static inline
            constants. Branch prediction, inlining and copy elision
            make the cost identical to accessing the constants
            directly.
          - The class deliberately exposes only a SUBSET of AndesTheme
            (panel + panelDark + text + outline + small radius +
            three font sizes). If a concrete LookAndFeel needs another
            theme value (e.g. textMuted, topGlossRatio), it should
            access it directly through AndesTheme::Colours::xxx;
            adding it here would inflate the base interface for a
            single use site.
          - Future LookAndFeel additions should inherit from this
            class, not from juce::LookAndFeel_V4 directly, so the
            theme indirection stays consistent.
        ES:
          - Los accesores son `noexcept` y retornan por valor porque
            solo reenvían referencias a constantes constexpr / static
            inline. La predicción de ramas, el inlining y el copy
            elision hacen que el costo sea idéntico a acceder a las
            constantes directamente.
          - La clase expone deliberadamente solo un SUBCONJUNTO de
            AndesTheme (panel + panelDark + text + outline + radio
            chico + tres tamaños de fuente). Si un LookAndFeel
            concreto necesita otro valor del tema (p. ej. textMuted,
            topGlossRatio), debe acceder a él directamente vía
            AndesTheme::Colours::xxx; añadirlo aquí inflaría la
            interfaz base por un único sitio de uso.
          - Los LookAndFeel futuros deben heredar de esta clase, no
            de juce::LookAndFeel_V4 directamente, para mantener la
            indirección del tema consistente.
*/

#pragma once
#include <JuceHeader.h>
#include "AndesTheme.h"


class AndesBaseLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AndesBaseLookAndFeel() = default;
    virtual ~AndesBaseLookAndFeel() = default;


protected:
    // ------------------------------------------------------------------------
    //  Theme accessors / Accesores del tema
    // ------------------------------------------------------------------------

    // EN: Color accessors. These mirror the AndesTheme::Colours fields
    //     but read more naturally inside subclass code:
    //         panelColour() instead of AndesTheme::Colours::panel
    // ES: Accesores de color. Reflejan los campos de AndesTheme::Colours
    //     pero se leen más naturales dentro del código de subclases:
    //         panelColour() en lugar de AndesTheme::Colours::panel
    juce::Colour panelColour()     const noexcept { return AndesTheme::Colours::panel; }
    juce::Colour panelDarkColour() const noexcept { return AndesTheme::Colours::panelDark; }
    juce::Colour textColour()      const noexcept { return AndesTheme::Colours::text; }
    juce::Colour outlineColour()   const noexcept { return AndesTheme::Colours::outline; }

    // EN: Geometric and typographic accessors. Same naturalness
    //     argument as the color accessors above.
    // ES: Accesores geométricos y tipográficos. Mismo argumento de
    //     naturalidad que los accesores de color de arriba.
    float smallRadius() const noexcept { return AndesTheme::Metrics::cornerRadiusSmall; }
    float fontSmall()   const noexcept { return AndesTheme::Fonts::small; }
    float fontMedium()  const noexcept { return AndesTheme::Fonts::medium; }
    float fontTiny()    const noexcept { return AndesTheme::Fonts::tiny; }
};
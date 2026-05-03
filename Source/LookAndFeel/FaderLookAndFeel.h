/*
  ==============================================================================

    FaderLookAndFeel.h
    Created: 8 Apr 2026 12:33:11pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: FaderLookAndFeel
    Purpose:
        EN: Custom JUCE LookAndFeel for the eight ADSR vertical faders
            of AndesJX (amp* and filter* attack/decay/sustain/release).
            Renders each fader as a sprite picked from a pre-baked
            sprite sheet (AndesFaderADSR.png) and overlays the live
            value text while the user is hovering or dragging.
        ES: LookAndFeel custom de JUCE para los ocho faders verticales
            de ADSR de AndesJX (amp* y filter*
            attack/decay/sustain/release). Renderiza cada fader como un
            sprite elegido de un sprite sheet pre-renderizado
            (AndesFaderADSR.png) y superpone el texto del valor en vivo
            mientras el usuario está sobre el fader o arrastrándolo.

    Architectural role:
        EN: Same sprite-sheet paradigm as SecondaryKnobLookAndFeel: a
            PNG holds every visual state of the fader cap stacked
            vertically, and the slider's normalized position picks the
            correct frame at runtime. Three differences worth noting
            compared to the canonical secondary knob:
              - Frames are RECTANGULAR (32 x 128 by default), not
                square, because the fader is a vertical slider with a
                tall track.
              - Frame dimensions are configurable via constructor
                arguments, so the same class can be reused for fader
                sprite sheets exported at different resolutions.
              - The value text is shown ONLY while the user is hovering
                or dragging, instead of permanently like the (unused)
                internal text path of SecondaryKnobLookAndFeel.
        ES: Mismo paradigma de sprite sheet que
            SecondaryKnobLookAndFeel: un PNG contiene cada estado
            visual del cap del fader apilado verticalmente, y la
            posición normalizada del slider elige el frame correcto en
            runtime. Tres diferencias notables respecto al knob
            secundario canónico:
              - Los frames son RECTANGULARES (32 x 128 por defecto),
                no cuadrados, porque el fader es un slider vertical
                con un track largo.
              - Las dimensiones del frame son configurables vía
                argumentos del constructor, así la misma clase puede
                reusarse para sprite sheets de faders exportados a
                distintas resoluciones.
              - El texto del valor solo se muestra mientras el usuario
                está sobre el fader o arrastrándolo, en lugar de
                permanentemente como el camino interno (no usado) de
                texto de SecondaryKnobLookAndFeel.

    Notes:
        EN:
          - Inherits from AndesBaseLookAndFeel like every other
            AndesJX LookAndFeel (theme accessors available as
            textColour(), fontTiny(), etc.).
          - Sprite sheet layout: the image must be exactly frameWidth
            pixels wide and (numFrames * frameHeight) pixels tall,
            with frames stacked top-to-bottom from frame 0 (slider at
            minimum) to frame N-1 (slider at maximum).
          - For non-vertical slider styles, the call falls through to
            the default JUCE drawing so the class does not break
            anything if reused for an unintended slider type.
        ES:
          - Hereda de AndesBaseLookAndFeel como todos los LookAndFeel
            de AndesJX (accesores del tema disponibles como
            textColour(), fontTiny(), etc.).
          - Layout del sprite sheet: la imagen debe medir exactamente
            frameWidth píxeles de ancho y (numFrames * frameHeight)
            píxeles de alto, con los frames apilados de arriba a
            abajo desde el frame 0 (slider en mínimo) hasta el frame
            N-1 (slider en máximo).
          - Para estilos de slider no-verticales, la llamada cae al
            dibujado por defecto de JUCE para que la clase no rompa
            nada si se reusa para un tipo de slider no previsto.
*/

#pragma once
#include <JuceHeader.h>
#include "AndesBaseLookAndFeel.h"


class FaderLookAndFeel : public AndesBaseLookAndFeel
{
public:
    // EN: Constructor parameters set the frame dimensions of the
    //     sprite sheet. The defaults match AndesJX's shipping
    //     AndesFaderADSR.png (32 x 128 per frame). If a future asset
    //     exports at a different resolution, instantiate the class
    //     with the new dimensions instead of editing this header.
    // ES: Los parámetros del constructor fijan las dimensiones de los
    //     frames del sprite sheet. Los defaults coinciden con el
    //     AndesFaderADSR.png que se entrega con AndesJX (32 x 128 por
    //     frame). Si un asset futuro se exporta a otra resolución,
    //     instanciar la clase con las nuevas dimensiones en lugar de
    //     editar este header.
    FaderLookAndFeel(int frameW = 32, int frameH = 128)
        : frameWidth(frameW), frameHeight(frameH)
    {
        faderImage = juce::ImageCache::getFromMemory(
            BinaryData::AndesFaderADSR_png,
            BinaryData::AndesFaderADSR_pngSize);
    }


    // ------------------------------------------------------------------------
    //  drawLinearSlider override / Override de drawLinearSlider
    // ------------------------------------------------------------------------

    // EN: JUCE calls this every time a linear slider needs repainting.
    //     Five steps:
    //       1. Bail out cleanly if the sprite sheet failed to load.
    //       2. Defer to JUCE's default drawing for any non-vertical
    //          style. AndesJX only uses LinearVertical here, but this
    //          guards against accidental misuse.
    //       3. Sanity-check the sprite sheet geometry (width must
    //          equal frameWidth; numFrames must be > 0).
    //       4. Map the slider value to a frame index via
    //          valueToProportionOfLength, which respects custom
    //          slider skews. jlimit clamps the index so values exactly
    //          at the maximum do not produce an out-of-range frame.
    //       5. Blit the selected frame into the destination
    //          rectangle. While the user hovers or drags, overlay
    //          the live value text just above the fader cap so the
    //          number is visible without obscuring the cap itself.
    // ES: JUCE llama esto cada vez que un slider linear necesita
    //     repintarse. Cinco pasos:
    //       1. Salir limpiamente si el sprite sheet no se pudo cargar.
    //       2. Delegar al dibujado por defecto de JUCE para cualquier
    //          estilo no-vertical. AndesJX solo usa LinearVertical
    //          aquí, pero esto protege contra mal uso accidental.
    //       3. Validar la geometría del sprite sheet (el ancho debe
    //          coincidir con frameWidth; numFrames debe ser > 0).
    //       4. Mapear el valor del slider a un índice de frame vía
    //          valueToProportionOfLength, que respeta los skews custom
    //          del slider. jlimit limita el índice para que valores
    //          exactamente en el máximo no produzcan un frame fuera
    //          de rango.
    //       5. Hacer blit del frame seleccionado en el rectángulo de
    //          destino. Mientras el usuario está sobre el fader o
    //          arrastrándolo, superponer el texto del valor en vivo
    //          justo arriba del cap del fader para que el número sea
    //          visible sin tapar el cap mismo.
    void drawLinearSlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        const juce::Slider::SliderStyle style,
        juce::Slider& slider) override
    {
        // EN: Position arguments come from JUCE for primitive-based
        //     drawing. The sprite-sheet path uses the slider's
        //     normalized value instead, so we mark them as unused.
        // ES: Los argumentos de posición vienen de JUCE para dibujado
        //     basado en primitivas. El camino de sprite sheet usa el
        //     valor normalizado del slider, así que los marcamos como
        //     no usados.
        juce::ignoreUnused(sliderPos, minSliderPos, maxSliderPos);

        if (!faderImage.isValid())
            return;

        if (style != juce::Slider::LinearVertical)
        {
            juce::LookAndFeel_V4::drawLinearSlider(
                g, x, y, width, height,
                sliderPos, minSliderPos, maxSliderPos, style, slider);
            return;
        }

        const int numFrames = faderImage.getHeight() / frameHeight;

        if (numFrames <= 0 || faderImage.getWidth() != frameWidth)
            return;

        // EN: valueToProportionOfLength returns [0, 1] respecting any
        //     skew applied to the slider, so the visual position
        //     matches what the user perceives.
        // ES: valueToProportionOfLength devuelve [0, 1] respetando
        //     cualquier skew aplicado al slider, así la posición
        //     visual coincide con lo que el usuario percibe.
        const double proportion = slider.valueToProportionOfLength(slider.getValue());

        const int frameIndex = juce::jlimit(
            0,
            numFrames - 1,
            (int)std::round(proportion * (numFrames - 1)));

        g.drawImage(faderImage,
            x, y, width, height,
            0, frameIndex * frameHeight,
            frameWidth, frameHeight);

        // EN: Live value overlay. Only visible during interaction so
        //     the GUI stays clean when the user is not touching the
        //     fader. Drawn slightly above the slider area (y - 4)
        //     with extended width (width + 8) so multi-character
        //     values like "100" do not get clipped at narrow
        //     fader columns.
        // ES: Overlay del valor en vivo. Solo visible durante la
        //     interacción para que la GUI quede limpia cuando el
        //     usuario no está tocando el fader. Se dibuja un poco por
        //     encima del área del slider (y - 4) con ancho extendido
        //     (width + 8) para que valores de varios caracteres como
        //     "100" no se recorten en columnas estrechas de fader.
        if (slider.isMouseOverOrDragging())
        {
            const auto valueText = slider.getTextFromValue(slider.getValue());

            const auto textArea = juce::Rectangle<int>(x - 4, y - 4, width + 8, 12);

            g.setColour(textColour());
            g.setFont(juce::Font(juce::FontOptions(fontTiny())));
            g.drawFittedText(valueText, textArea, juce::Justification::centred, 1);
        }
    }


private:
    // ------------------------------------------------------------------------
    //  Sprite sheet + frame geometry
    //  Sprite sheet + geometría de frames
    // ------------------------------------------------------------------------

    juce::Image faderImage;
    int frameWidth = 32;
    int frameHeight = 128;
};
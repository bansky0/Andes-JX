/*
  ==============================================================================

    SecondaryKnobLookAndFeel.h
    Created: 2 Apr 2026 3:36:06pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: SecondaryKnobLookAndFeel
    Purpose:
        EN: Custom JUCE LookAndFeel for the 15 secondary value-labeled
            knobs of AndesJX (oscTune, stereoWidth, noise, vibrato,
            filterEnv, etc.). Renders the knob as an animated sprite
            picked from a pre-baked sprite sheet (AndesKnobSecondary.png),
            and optionally overlays the current value text in the
            knob's center.
        ES: LookAndFeel custom de JUCE para los 15 knobs secundarios
            con label de valor de AndesJX (oscTune, stereoWidth, noise,
            vibrato, filterEnv, etc.). Renderiza el knob como un sprite
            animado elegido de un sprite sheet pre-renderizado
            (AndesKnobSecondary.png), y opcionalmente superpone el
            texto del valor actual en el centro del knob.

    Architectural role:
        EN: Diverges from the canonical "draw with primitives" approach
            of ToggleLookAndFeel and SegmentedButtonLookAndFeel. Here
            the knob is NOT drawn from geometric primitives at runtime;
            instead, a sprite sheet pre-rendered by a designer holds
            every visual state of the knob stacked vertically, and the
            slider's normalized position picks the correct frame.
            This trades runtime drawing flexibility for richer visual
            quality (anti-aliasing, beveling, lighting effects baked
            into the PNG by the design tool of choice).
        ES: Diverge del enfoque canónico "dibujar con primitivas" de
            ToggleLookAndFeel y SegmentedButtonLookAndFeel. Aquí el
            knob NO se dibuja desde primitivas geométricas en tiempo
            de ejecución; en su lugar, un sprite sheet pre-renderizado
            por un diseñador contiene cada estado visual del knob
            apilado verticalmente, y la posición normalizada del
            slider elige el frame correcto. Esto cambia flexibilidad
            de dibujo en runtime por calidad visual más rica
            (antialiasing, biselado, efectos de iluminación
            pre-pintados en el PNG por la herramienta de diseño).

    Notes:
        EN:
          - The sprite sheet convention is "square frames stacked
            vertically": frame 0 at the top, frame N-1 at the bottom,
            each frame frameSize x frameSize pixels. Image width is
            frameSize, image height is numFrames * frameSize. Drawing
            uses a sub-rectangle from the source image that selects
            one frame.
          - showValueText is set to FALSE by the editor for these
            knobs, because the editor draws an EXTERNAL value Label
            beside each knob (initialiseValueLabeledControl in
            PluginEditor.cpp). The internal text drawing in this class
            is kept as a feature for hypothetical future uses or for
            standalone testing of the LookAndFeel.
          - The knob image is loaded once in the constructor through
            juce::ImageCache, which is shared globally and avoids
            re-decoding the PNG every time a control is repainted.
        ES:
          - La convención del sprite sheet es "frames cuadrados
            apilados verticalmente": frame 0 arriba, frame N-1 abajo,
            cada frame de frameSize x frameSize píxeles. El ancho de
            la imagen es frameSize, el alto es numFrames * frameSize.
            El dibujado usa un sub-rectángulo de la imagen fuente que
            selecciona un frame.
          - showValueText lo pone el editor en FALSE para estos knobs,
            porque el editor dibuja un Label de valor EXTERNO al lado
            de cada knob (initialiseValueLabeledControl en
            PluginEditor.cpp). El dibujado interno de texto en esta
            clase se conserva como feature para usos futuros
            hipotéticos o para pruebas standalone del LookAndFeel.
          - La imagen del knob se carga una sola vez en el constructor
            a través de juce::ImageCache, que está compartido
            globalmente y evita re-decodificar el PNG cada vez que un
            control se repinta.
*/

#pragma once
#include <JuceHeader.h>
#include "AndesBaseLookAndFeel.h"


class SecondaryKnobLookAndFeel : public AndesBaseLookAndFeel
{
public:
    // EN: Loads the sprite sheet from BinaryData via juce::ImageCache.
    //     The cache means subsequent instances of this LookAndFeel
    //     reuse the already-decoded image instead of decoding again.
    // ES: Carga el sprite sheet desde BinaryData vía juce::ImageCache.
    //     El cache hace que las instancias subsecuentes de este
    //     LookAndFeel reusen la imagen ya decodificada en vez de
    //     volver a decodificarla.
    SecondaryKnobLookAndFeel()
    {
        knobImage = juce::ImageCache::getFromMemory(
            BinaryData::AndesKnobSecondary_png,
            BinaryData::AndesKnobSecondary_pngSize);
    }

    ~SecondaryKnobLookAndFeel() override = default;


    // ------------------------------------------------------------------------
    //  drawRotarySlider override / Override de drawRotarySlider
    // ------------------------------------------------------------------------

    // EN: JUCE calls this every time a rotary slider needs repainting.
    //     Sprite-sheet workflow:
    //       1. Compute frame layout: frameSize is the image width
    //          (each frame is square), numFrames is height/frameSize.
    //       2. Map the slider's normalized [0, 1] position to a frame
    //          index, clamped safely to [0, numFrames-1].
    //       3. Center the knob in the available area, using the
    //          smaller of width/height so it stays square.
    //       4. Blit only the selected frame from the sprite sheet
    //          into the destination rectangle.
    //       5. Optionally overlay the value text on top.
    //     The rotaryStart/End angles are unused because the knob's
    //     rotation is baked into the sprite frames; JUCE provides them
    //     for primitive-based drawing, which we are NOT doing here.
    // ES: JUCE llama esto cada vez que un slider rotatorio necesita
    //     repintarse. Flujo del sprite sheet:
    //       1. Calcular el layout de frames: frameSize es el ancho de
    //          la imagen (cada frame es cuadrado), numFrames es
    //          alto/frameSize.
    //       2. Mapear la posición normalizada [0, 1] del slider a un
    //          índice de frame, limitado seguro a [0, numFrames-1].
    //       3. Centrar el knob en el área disponible, usando el menor
    //          de width/height para que se mantenga cuadrado.
    //       4. Hacer blit solo del frame seleccionado del sprite sheet
    //          al rectángulo de destino.
    //       5. Opcionalmente superponer el texto del valor encima.
    //     Los ángulos rotaryStart/End se ignoran porque la rotación
    //     del knob está pre-pintada en los frames del sprite; JUCE los
    //     provee para dibujado basado en primitivas, que NO estamos
    //     haciendo aquí.
    void drawRotarySlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float /*rotaryStartAngle*/,
        float /*rotaryEndAngle*/,
        juce::Slider& slider) override
    {
        if (!knobImage.isValid())
            return;

        const int frameSize = knobImage.getWidth();
        const int numFrames = knobImage.getHeight() / frameSize;

        // EN: jlimit clamps to [0, numFrames-1] so values exactly at
        //     1.0 do not produce an out-of-range index.
        // ES: jlimit limita a [0, numFrames-1] para que valores
        //     exactamente en 1.0 no produzcan un índice fuera de rango.
        const int frameIndex = juce::jlimit(
            0, numFrames - 1,
            juce::roundToInt(sliderPosProportional * static_cast<float>(numFrames - 1)));

        // EN: Use the smaller dimension so the knob always stays
        //     square, then center it within the available area.
        // ES: Usar la menor dimensión para que el knob siempre se
        //     mantenga cuadrado, luego centrarlo en el área disponible.
        const int drawSize = juce::jmin(width, height);
        const int drawX = x + (width - drawSize) / 2;
        const int drawY = y + (height - drawSize) / 2;

        // EN: Blit a single frame from the sprite sheet. The source
        //     rectangle is (0, frameIndex*frameSize, frameSize,
        //     frameSize): take a frameSize-tall slice at the right
        //     vertical offset.
        // ES: Blit de un único frame del sprite sheet. El rectángulo
        //     fuente es (0, frameIndex*frameSize, frameSize,
        //     frameSize): tomar una rebanada de altura frameSize en
        //     el offset vertical correcto.
        g.drawImage(knobImage,
            drawX, drawY, drawSize, drawSize,
            0, frameIndex * frameSize, frameSize, frameSize);

        if (showValueText)
            drawKnobValueText(g, slider, drawX, drawY, drawSize);
    }


    // ------------------------------------------------------------------------
    //  Setters / Setters
    // ------------------------------------------------------------------------

    void setTextFontHeight(float newHeight) noexcept { textFontHeight = newHeight; }
    void setShowValueText(bool  shouldShow) noexcept { showValueText = shouldShow; }
    void setTextInset(int   newInset)   noexcept { textInset = newInset; }


private:
    // ------------------------------------------------------------------------
    //  Internal value-text drawing / Dibujado interno de texto de valor
    // ------------------------------------------------------------------------

    // EN: Draws the slider's current value as a small centered string
    //     inside the knob image. Used only when showValueText is true,
    //     which the editor disables for the AndesJX secondary knobs
    //     (they use external labels). textInset shrinks the text area
    //     so the string stays clear of the knob's outer ring.
    // ES: Dibuja el valor actual del slider como una string pequeña
    //     centrada dentro de la imagen del knob. Solo se usa cuando
    //     showValueText es true, lo cual el editor desactiva para los
    //     knobs secundarios de AndesJX (usan labels externos).
    //     textInset achica el área del texto para que la string se
    //     mantenga lejos del anillo exterior del knob.
    void drawKnobValueText(juce::Graphics& g,
        juce::Slider& slider,
        int drawX, int drawY, int drawSize)
    {
        g.setColour(textColour());

        const auto valueText = slider.getTextFromValue(slider.getValue());

        auto textBounds = juce::Rectangle<int>(drawX, drawY, drawSize, drawSize)
            .reduced(textInset, textInset);

        g.setFont(juce::Font(juce::FontOptions(textFontHeight)));
        g.drawFittedText(valueText,
            textBounds,
            juce::Justification::centred,
            1);
    }


private:
    // ------------------------------------------------------------------------
    //  Sprite sheet + display configuration
    //  Sprite sheet + configuración de display
    // ------------------------------------------------------------------------

    juce::Image knobImage;

    // EN: Defaults match the AndesJX secondary knob conventions; the
    //     editor overrides them via setters.
    // ES: Los defaults coinciden con las convenciones de los knobs
    //     secundarios de AndesJX; el editor los sobrescribe vía
    //     setters.
    float textFontHeight{ 7.5f };
    bool  showValueText{ true };
    int   textInset{ 3 };
};
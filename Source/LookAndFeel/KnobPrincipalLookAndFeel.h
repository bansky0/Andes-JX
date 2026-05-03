/*
  ==============================================================================

    KnobPrincipalLookAndFeel.h
    Created: 1 Apr 2026 4:09:44pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: KnobPrincipalLookAndFeel
    Purpose:
        EN: Custom JUCE LookAndFeel for the four headline knobs of
            AndesJX (oscMix, filterFreq, filterReso, output). Renders
            each knob as an animated sprite from a pre-baked sprite
            sheet (AndesKnobPrincipal2.png) and ALWAYS overlays the
            current value text in the knob's center, with two possible
            layouts depending on which knob is being drawn:
              - twoLine layout (output, filterReso, filterFreq):
                value on top, unit suffix below ("0.0" / "dB").
              - singleLine layout (oscMix and any future addition):
                a single concise string ("70:30" for mix, etc.).
        ES: LookAndFeel custom de JUCE para los cuatro knobs principales
            de AndesJX (oscMix, filterFreq, filterReso, output).
            Renderiza cada knob como un sprite animado desde un sprite
            sheet pre-renderizado (AndesKnobPrincipal2.png) y SIEMPRE
            superpone el texto del valor actual en el centro del knob,
            con dos layouts posibles según qué knob se esté dibujando:
              - layout twoLine (output, filterReso, filterFreq): valor
                arriba, sufijo de unidad abajo ("0.0" / "dB").
              - layout singleLine (oscMix y cualquier adición futura):
                una única string concisa ("70:30" para mix, etc.).

    Architectural role:
        EN: Same sprite-sheet paradigm as SecondaryKnobLookAndFeel,
            but with three intentional richness upgrades that justify
            its existence as a separate class:
              - The internal value text is ALWAYS drawn (not optional
                via a flag). The headline knobs do not have external
                value labels, so the text inside the knob IS the value
                display. This contrasts with secondary knobs, which
                rely on labels drawn next to them by the editor.
              - Two text layouts (singleLine vs twoLine), chosen per
                knob via componentID dispatch.
              - Per-componentID formatting rules for primary and
                secondary text strings (e.g. mix is "osc1:osc2",
                output is signed dB).
        ES: Mismo paradigma de sprite sheet que SecondaryKnobLookAndFeel,
            pero con tres mejoras intencionales de riqueza que
            justifican su existencia como clase separada:
              - El texto interno del valor SIEMPRE se dibuja (no es
                opcional vía flag). Los knobs principales no tienen
                labels externos de valor, así que el texto adentro del
                knob ES el display del valor. Contrasta con los knobs
                secundarios, que dependen de labels dibujados al lado
                por el editor.
              - Dos layouts de texto (singleLine vs twoLine), elegidos
                por knob mediante dispatch por componentID.
              - Reglas de formato por componentID para los strings de
                texto primario y secundario (p. ej. mix es "osc1:osc2",
                output es dB con signo).

    Notes:
        EN:
          - The componentID-based dispatch in resolveTextLayout,
            resolveTwoLineValueFontHeight, resolvePrimaryText and
            resolveSecondaryText establishes a contract with
            PluginEditor::initialiseKnobs: if the componentID of a
            headline knob ever changes there, the corresponding
            switch arms in this class must be updated to match. The
            current IDs are "oscMix", "filterFreq", "filterReso",
            "output".
          - Within the twoLine layout, two visual weights coexist:
              - "main" (mainValueFontHeight, default 10 pt) is used by
                the output knob, which is visually emphasized as the
                master output control.
              - "compact" (compactValueFontHeight, default 8.5 pt) is
                used by filterFreq and filterReso. Together with the
                singleLine font (also 8.5 pt for oscMix), this gives
                three of the four headline knobs the same value-text
                weight, while output stands out as the "louder" one.
          - All fonts are built through AndesStyleHelpers::makeUIFont
            so the knob value text uses the same Arial-non-bold
            typeface as the rest of AndesJX (see makeUIFont's docs for
            the per-host bold-default trap it defends against).
          - For oscMix, this LookAndFeel intentionally REPLICATES the
            same "osc1:osc2" formatting that the APVTS string
            formatter applies in createParameterLayout. The duplication
            is deliberate: the APVTS formatter is what the host shows
            in its automation lanes; this formatter is what the user
            sees inside the knob. They MUST stay in sync if the GUI
            and host views are to agree.
        ES:
          - El dispatch por componentID en resolveTextLayout,
            resolveTwoLineValueFontHeight, resolvePrimaryText y
            resolveSecondaryText establece un contrato con
            PluginEditor::initialiseKnobs: si el componentID de un
            knob principal llegara a cambiar allí, las ramas
            correspondientes del switch en esta clase deben
            actualizarse para coincidir. Los IDs actuales son
            "oscMix", "filterFreq", "filterReso", "output".
          - Dentro del layout twoLine coexisten dos pesos visuales:
              - "main" (mainValueFontHeight, default 10 pt) lo usa el
                knob output, que se enfatiza visualmente como el
                control de salida master.
              - "compact" (compactValueFontHeight, default 8.5 pt) lo
                usan filterFreq y filterReso. Junto con la fuente
                singleLine (también 8.5 pt para oscMix), esto da a
                tres de los cuatro knobs principales el mismo peso
                de texto de valor, mientras que output destaca como
                el "más fuerte".
          - Todas las fuentes se construyen vía
            AndesStyleHelpers::makeUIFont para que el texto de valor
            del knob use la misma typeface Arial-no-bold que el resto
            de AndesJX (ver los docs de makeUIFont para la trampa de
            bold-por-defecto-según-host contra la que defiende).
          - Para oscMix, este LookAndFeel REPLICA intencionadamente el
            mismo formato "osc1:osc2" que el formateador de string del
            APVTS aplica en createParameterLayout. La duplicación es
            deliberada: el formateador del APVTS es lo que muestra el
            host en sus lanes de automatización; este formateador es
            lo que el usuario ve dentro del knob. Ambos DEBEN
            mantenerse en sincronía para que las vistas del GUI y del
            host coincidan.
*/

#pragma once
#include <JuceHeader.h>
#include "AndesBaseLookAndFeel.h"
#include "AndesStyleHelpers.h"


class KnobPrincipalLookAndFeel : public AndesBaseLookAndFeel
{
public:
    // EN: Loads the headline knob sprite sheet from BinaryData via
    //     juce::ImageCache. Same caching rationale as
    //     SecondaryKnobLookAndFeel: multiple instances of this class
    //     reuse the already-decoded image.
    // ES: Carga el sprite sheet del knob principal desde BinaryData
    //     vía juce::ImageCache. Misma razón de caching que
    //     SecondaryKnobLookAndFeel: múltiples instancias de esta clase
    //     reusan la imagen ya decodificada.
    KnobPrincipalLookAndFeel()
    {
        knobImage = juce::ImageCache::getFromMemory(
            BinaryData::AndesKnobPrincipal2_png,
            BinaryData::AndesKnobPrincipal2_pngSize);
    }

    ~KnobPrincipalLookAndFeel() override = default;


    // ------------------------------------------------------------------------
    //  drawRotarySlider override / Override de drawRotarySlider
    // ------------------------------------------------------------------------

    // EN: JUCE calls this every time a rotary slider needs repainting.
    //     Same five-step sprite-sheet workflow as
    //     SecondaryKnobLookAndFeel — see that file for the canonical
    //     description. Two differences:
    //       - The internal value text is drawn UNCONDITIONALLY at
    //         the end of every paint, because the headline knobs have
    //         no external label.
    //       - rotaryStart/End angles are unused for the same reason
    //         as the secondary knob (the rotation is baked into the
    //         sprite frames).
    // ES: JUCE llama esto cada vez que un slider rotatorio necesita
    //     repintarse. Mismo flujo de cinco pasos del sprite sheet que
    //     SecondaryKnobLookAndFeel — ver ese archivo para la
    //     descripción canónica. Dos diferencias:
    //       - El texto interno del valor se dibuja SIN CONDICIÓN al
    //         final de cada paint, porque los knobs principales no
    //         tienen label externo.
    //       - Los ángulos rotaryStart/End se ignoran por la misma
    //         razón que el knob secundario (la rotación está
    //         pre-pintada en los frames del sprite).
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

        const int frameIndex = juce::jlimit(
            0, numFrames - 1,
            juce::roundToInt(sliderPosProportional * static_cast<float>(numFrames - 1)));

        const int drawSize = juce::jmin(width, height);
        const int drawX = x + (width - drawSize) / 2;
        const int drawY = y + (height - drawSize) / 2;

        g.drawImage(knobImage,
            drawX, drawY, drawSize, drawSize,
            0, frameIndex * frameSize, frameSize, frameSize);

        drawKnobValueText(g, slider, drawX, drawY, drawSize);
    }


    // ------------------------------------------------------------------------
    //  Font-height setters / Setters de altos de fuente
    // ------------------------------------------------------------------------

    // EN: Four font sizes for four text roles inside the knob:
    //       - mainValueFontHeight    : value number in twoLine layout
    //                                  for the "loud" headline knob
    //                                  (output). Default 10 pt.
    //       - compactValueFontHeight : value number in twoLine layout
    //                                  for the "compact" headline
    //                                  knobs (filterFreq, filterReso).
    //                                  Default 8.5 pt.
    //       - unitFontHeight         : the unit suffix below the value
    //                                  in twoLine layout ("dB", "%").
    //                                  Default 7.5 pt.
    //       - singleLineFontHeight   : the single string used by the
    //                                  singleLine layout (oscMix).
    //                                  Default 8.5 pt.
    //     Defaults are tuned for the AndesJX shipping artwork; the
    //     editor adjusts them in initialiseLookAndFeels.
    // ES: Cuatro tamaños de fuente para cuatro roles de texto dentro
    //     del knob:
    //       - mainValueFontHeight    : número del valor en layout
    //                                  twoLine para el knob principal
    //                                  "fuerte" (output). Default 10 pt.
    //       - compactValueFontHeight : número del valor en layout
    //                                  twoLine para los knobs
    //                                  principales "compactos"
    //                                  (filterFreq, filterReso).
    //                                  Default 8.5 pt.
    //       - unitFontHeight         : sufijo de unidad debajo del
    //                                  valor en layout twoLine
    //                                  ("dB", "%"). Default 7.5 pt.
    //       - singleLineFontHeight   : la única string que usa el
    //                                  layout singleLine (oscMix).
    //                                  Default 8.5 pt.
    //     Los defaults están afinados para el arte que se entrega con
    //     AndesJX; el editor los ajusta en initialiseLookAndFeels.
    void setMainValueFontHeight(float newHeight) noexcept { mainValueFontHeight = newHeight; }
    void setCompactValueFontHeight(float newHeight) noexcept { compactValueFontHeight = newHeight; }
    void setUnitFontHeight(float newHeight) noexcept { unitFontHeight = newHeight; }
    void setSingleLineFontHeight(float newHeight) noexcept { singleLineFontHeight = newHeight; }


private:
    // ------------------------------------------------------------------------
    //  Text layout enum / Enum de layout de texto
    // ------------------------------------------------------------------------

    // EN: Two layouts supported. Each headline knob picks one through
    //     resolveTextLayout below.
    // ES: Se soportan dos layouts. Cada knob principal elige uno
    //     mediante resolveTextLayout de abajo.
    enum class TextLayout
    {
        singleLine,
        twoLine
    };


    // ------------------------------------------------------------------------
    //  Internal value-text drawing / Dibujado interno del texto de valor
    // ------------------------------------------------------------------------

    // EN: Draws the value text inside the knob. Two layouts:
    //
    //       twoLine: value on top + unit below, separated by a small
    //         vertical spacing. Used when the value benefits from
    //         a unit suffix that does not fit comfortably on the
    //         same line ("0.0 dB" would be too cramped at the
    //         knob's diameter). The value's font height is selected
    //         per knob via resolveTwoLineValueFontHeight, so the
    //         output knob can be visually louder than filterFreq /
    //         filterReso.
    //
    //       singleLine: a single horizontally-fitted string. Used
    //         when the value is self-explanatory or compact enough to
    //         not need a separate unit ("70:30" for mix).
    //
    //     The 2-px reduction at the top creates a visual margin
    //     between the value text and the inner ring of the knob
    //     image, preventing the text from kissing the rim.
    //
    // ES: Dibuja el texto del valor dentro del knob. Dos layouts:
    //
    //       twoLine: valor arriba + unidad abajo, separados por un
    //         pequeño espaciado vertical. Se usa cuando el valor se
    //         beneficia de un sufijo de unidad que no entra cómodo
    //         en la misma línea ("0.0 dB" quedaría apretado al
    //         diámetro del knob). El alto de fuente del valor se
    //         elige por knob vía resolveTwoLineValueFontHeight, así
    //         el knob output puede ser visualmente más fuerte que
    //         filterFreq / filterReso.
    //
    //       singleLine: una única string ajustada horizontalmente.
    //         Se usa cuando el valor es auto-explicativo o lo
    //         bastante compacto para no necesitar una unidad
    //         separada ("70:30" para mix).
    //
    //     La reducción de 2 px arriba crea un margen visual entre el
    //     texto del valor y el anillo interno de la imagen del knob,
    //     evitando que el texto toque el reborde.
    void drawKnobValueText(juce::Graphics& g,
        juce::Slider& slider,
        int drawX, int drawY, int drawSize)
    {
        g.setColour(textColour());

        const auto layout = resolveTextLayout(slider);
        const auto valueText = resolvePrimaryText(slider);
        const auto unitText = resolveSecondaryText(slider);

        auto bounds = juce::Rectangle<float>(static_cast<float>(drawX),
            static_cast<float>(drawY),
            static_cast<float>(drawSize),
            static_cast<float>(drawSize)).reduced(2.0f);

        if (layout == TextLayout::twoLine)
        {
            const float centerY = bounds.getCentreY();
            const float spacing = 3.0f;

            // EN: Value sits just above the vertical center; unit just
            //     below it. The 12 / 8 px heights match the visual
            //     weight of the value and unit fonts respectively.
            // ES: El valor se sitúa justo arriba del centro vertical;
            //     la unidad justo debajo. Los altos 12 / 8 px coinciden
            //     con el peso visual de las fuentes de valor y unidad
            //     respectivamente.
            juce::Rectangle<float> valueArea(bounds.getX(),
                centerY - 10.0f,
                bounds.getWidth(),
                12.0f);

            juce::Rectangle<float> unitArea(bounds.getX(),
                centerY + spacing,
                bounds.getWidth(),
                8.0f);

            // EN: Per-knob font height selection: output uses
            //     mainValueFontHeight (loud); filterFreq/filterReso
            //     use compactValueFontHeight (matches mix).
            // ES: Selección del alto de fuente por knob: output usa
            //     mainValueFontHeight (fuerte); filterFreq/filterReso
            //     usan compactValueFontHeight (coincide con mix).
            const float valueFontHeight = resolveTwoLineValueFontHeight(slider);

            g.setFont(AndesStyleHelpers::makeUIFont(valueFontHeight));
            g.drawText(valueText, valueArea, juce::Justification::centred);

            g.setFont(AndesStyleHelpers::makeUIFont(unitFontHeight));
            g.drawText(unitText, unitArea, juce::Justification::centred);
        }
        else
        {
            auto textBounds = juce::Rectangle<int>(drawX, drawY, drawSize, drawSize).reduced(2, 2);

            g.setFont(AndesStyleHelpers::makeUIFont(singleLineFontHeight));
            g.drawFittedText(valueText,
                textBounds,
                juce::Justification::centred,
                1);
        }
    }


    // ------------------------------------------------------------------------
    //  ComponentID-based resolvers / Resolvedores por componentID
    // ------------------------------------------------------------------------

    // EN: Picks the layout for a given knob. Three of the four
    //     headline knobs go twoLine because they have meaningful
    //     unit suffixes (dB for output, % for cutoff and resonance).
    //     oscMix goes singleLine because its "70:30" ratio is its
    //     own complete display.
    // ES: Elige el layout para un knob dado. Tres de los cuatro
    //     knobs principales van twoLine porque tienen sufijos de
    //     unidad significativos (dB para output, % para cutoff y
    //     resonancia). oscMix va singleLine porque su ratio "70:30"
    //     es su propio display completo.
    TextLayout resolveTextLayout(juce::Slider& slider) const
    {
        const auto id = slider.getComponentID();

        if (id == "output" || id == "filterReso" || id == "filterFreq")
            return TextLayout::twoLine;

        return TextLayout::singleLine;
    }


    // EN: Picks the value's font height inside the twoLine layout.
    //     output gets the "loud" main font; filterFreq and filterReso
    //     get the smaller compact font so the three "control" knobs
    //     (mix, cutoff, resonance) share the same value-text weight
    //     and the output knob stands out as the master.
    // ES: Elige el alto de fuente del valor dentro del layout twoLine.
    //     output recibe la fuente main "fuerte"; filterFreq y
    //     filterReso reciben la fuente compact más chica para que los
    //     tres knobs de "control" (mix, cutoff, resonance) compartan
    //     el mismo peso de texto de valor y el knob output destaque
    //     como master.
    float resolveTwoLineValueFontHeight(juce::Slider& slider) const
    {
        const auto id = slider.getComponentID();

        if (id == "output")
            return mainValueFontHeight;

        return compactValueFontHeight;
    }


    // EN: Returns the PRIMARY text string (the value itself) for a
    //     given knob. Per-knob formatting rules:
    //
    //       output     -> signed one-decimal with "0" sentinel near
    //                     zero (avoids displaying "+0.0" or "-0.0").
    //       oscMix     -> ratio "osc1:osc2", computed from the slider
    //                     value (which represents osc2's percentage).
    //                     MUST agree with the formatter declared in
    //                     PluginProcessor::createParameterLayout for
    //                     oscMix.
    //       filterFreq -> rounded integer percentage.
    //       filterReso -> rounded integer percentage.
    //       (default)  -> JUCE's default getTextFromValue, which uses
    //                     the parameter's range and step for
    //                     formatting.
    //
    // ES: Devuelve la string de texto PRIMARIA (el valor mismo) para
    //     un knob dado. Reglas de formato por knob:
    //
    //       output     -> con signo y un decimal, con sentinel "0"
    //                     cerca de cero (evita mostrar "+0.0" o
    //                     "-0.0").
    //       oscMix     -> ratio "osc1:osc2", calculado a partir del
    //                     valor del slider (que representa el
    //                     porcentaje de osc2). DEBE coincidir con el
    //                     formateador declarado en
    //                     PluginProcessor::createParameterLayout para
    //                     oscMix.
    //       filterFreq -> porcentaje entero redondeado.
    //       filterReso -> porcentaje entero redondeado.
    //       (default)  -> getTextFromValue por defecto de JUCE, que
    //                     usa el rango y paso del parámetro para el
    //                     formato.
    juce::String resolvePrimaryText(juce::Slider& slider) const
    {
        const auto  id = slider.getComponentID();
        const float value = static_cast<float>(slider.getValue());

        if (id == "output")
        {
            if (std::abs(value) < 0.05f)
                return "0";

            return value > 0.0f
                ? "+" + juce::String(value, 1)
                : juce::String(value, 1);
        }

        if (id == "oscMix")
        {
            const int osc2 = juce::roundToInt(value);
            const int osc1 = 100 - osc2;
            return juce::String(osc1) + ":" + juce::String(osc2);
        }

        if (id == "filterReso" || id == "filterFreq")
            return juce::String(juce::roundToInt(value));

        return slider.getTextFromValue(value);
    }


    // EN: Returns the SECONDARY text string (the unit suffix) for a
    //     given knob, or an empty string when there is no unit. Only
    //     consulted when the layout is twoLine.
    // ES: Devuelve la string de texto SECUNDARIA (el sufijo de unidad)
    //     para un knob dado, o una string vacía cuando no hay unidad.
    //     Solo se consulta cuando el layout es twoLine.
    juce::String resolveSecondaryText(juce::Slider& slider) const
    {
        const auto id = slider.getComponentID();

        if (id == "output")                              return "dB";
        if (id == "filterReso" || id == "filterFreq")    return "%";

        return {};
    }


private:
    // ------------------------------------------------------------------------
    //  Sprite sheet + font configuration
    //  Sprite sheet + configuración de fuentes
    // ------------------------------------------------------------------------

    juce::Image knobImage;

    float mainValueFontHeight{ 10.0f };
    float compactValueFontHeight{ 8.5f };
    float unitFontHeight{ 7.5f };
    float singleLineFontHeight{ 8.5f };
};
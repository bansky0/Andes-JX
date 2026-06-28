/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

/*
    Module: AndesJXAudioProcessorEditor (implementation)
    Purpose:
        EN: Implements the JUCE AudioProcessorEditor declared in
            PluginEditor.h. Builds the visual layout of AndesJX,
            instantiates every control, applies custom LookAndFeels,
            wires controls to APVTS parameters via attachments, and
            keeps value labels in sync with the underlying parameter
            values (whether they change by GUI, host automation, MIDI
            CC or preset load).
        ES: Implementa el AudioProcessorEditor de JUCE declarado en
            PluginEditor.h. Construye la disposición visual de AndesJX,
            instancia cada control, aplica los LookAndFeels custom,
            cablea los controles a los parámetros APVTS mediante
            attachments y mantiene los labels de valor sincronizados con
            los valores reales (vengan del GUI, automatización del host,
            CC MIDI o carga de preset).

    Structure / Estructura:
        EN:
          1. Anonymous namespace: shared text-drawing helpers and
             tracking constants used by paint().
          2. Constructor + lifecycle (background, LookAndFeels).
          3. Generic widget setup helpers + value-labeled-control
             helper (the single function that absorbs the boilerplate
             shared by 15 secondary knobs).
          4. Per-control format / update / initialise functions.
          5. Envelope, oscillator, preset, glide-mode, poly toggle and
             filter-type initializers.
          6. APVTS attachments wiring.
          7. Destructor (LookAndFeel pointers cleanup).
          8. paint() and resized() (background, labels, layout).
          9. parameterChanged() and changeListenerCallback().
        ES:
          1. Namespace anónimo: helpers compartidos para dibujar texto
             y constantes de tracking usados por paint().
          2. Constructor + ciclo de vida (fondo, LookAndFeels).
          3. Helpers genéricos por widget + helper para controles con
             label de valor (la única función que absorbe el
             boilerplate compartido por 15 knobs secundarios).
          4. Funciones format / update / initialise por control.
          5. Inicializadores de envolventes, osciladores, presets,
             glide-mode, toggle de polifonía y tipo de filtro.
          6. Cableado de attachments del APVTS.
          7. Destructor (limpieza de punteros a LookAndFeel).
          8. paint() y resized() (fondo, labels, layout).
          9. parameterChanged() y changeListenerCallback().
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


// ============================================================================
//  1. ANONYMOUS NAMESPACE: TEXT-DRAWING HELPERS
//     NAMESPACE ANÓNIMO: HELPERS PARA DIBUJAR TEXTO
// ============================================================================

// EN: An anonymous namespace gives these helpers internal linkage:
//     they are visible only inside this translation unit, so they will
//     not collide with similarly-named functions elsewhere in the
//     project. paint() uses them to draw the labels and section titles
//     printed on top of the background image.
// ES: El namespace anónimo da a estos helpers enlazado interno: solo
//     son visibles dentro de esta unidad de traducción, así que no
//     chocan con funciones de igual nombre en otras partes del proyecto.
//     paint() los usa para dibujar las etiquetas y títulos de sección
//     impresos encima de la imagen de fondo.
namespace
{
    // EN: Letter-spacing factors (kerning) tuned for each text role so
    //     the typography stays consistent across the GUI. Larger values
    //     spread the glyphs apart; the section title gets the most
    //     spacing to feel "headlined".
    // ES: Factores de espaciado entre letras (kerning) afinados para
    //     cada rol de texto para mantener la tipografía consistente en
    //     toda la GUI. Valores mayores separan más los glyphs; el
    //     título de sección recibe el mayor espaciado para sentirse
    //     "titular".
    constexpr float kTitleTracking = 0.08f;
    constexpr float kLabelTracking = 0.06f;
    constexpr float kTinyLabelTracking = 0.04f;


    // EN: Generic text-drawing primitive. Every other helper in this
    //     namespace is a thin wrapper around this one, fixing certain
    //     parameters to obtain the desired text role.
    // ES: Primitiva genérica para dibujar texto. Todos los demás helpers
    //     de este namespace son envoltorios finos sobre esta función,
    //     fijando ciertos parámetros para obtener el rol de texto
    //     deseado.
    void drawTextLabel(juce::Graphics& g,
        juce::Rectangle<int> area,
        const juce::String& text,
        float fontHeight,
        float alpha,
        float kerning,
        juce::Justification justification)
    {
        juce::Font font(juce::FontOptions{ fontHeight });
        font.setTypefaceName("Helvetica, Arial, sans-serif");
        font.setBold(false);
        font = font.withExtraKerningFactor(kerning);

        g.setFont(font);
        g.setColour(AndesTheme::Colours::text.withAlpha(alpha));
        g.drawFittedText(text, area, justification, 1);
    }


    // EN: Section title. 11pt, slightly transparent, left-aligned, wide
    //     tracking. Used for the headers of GUI panels (OSCILLATORS,
    //     FILTER, ENVELOPES, etc.).
    // ES: Título de sección. 11pt, ligeramente transparente, alineado a
    //     la izquierda, tracking amplio. Para los encabezados de los
    //     paneles de la GUI (OSCILLATORS, FILTER, ENVELOPES, etc.).
    void drawSectionTitle(juce::Graphics& g,
        juce::Rectangle<int> area,
        const juce::String& title)
    {
        drawTextLabel(g, area, title, 11.0f, 0.6f, kTitleTracking,
            juce::Justification::centredLeft);
    }


    // EN: Standard control caption. 7.5pt, mostly opaque, centered.
    //     Drawn under each knob to label what it controls.
    // ES: Caption estándar de control. 7.5pt, casi opaco, centrado.
    //     Se dibuja debajo de cada knob para indicar qué controla.
    void drawControlLabel(juce::Graphics& g,
        juce::Rectangle<int> area,
        const juce::String& text)
    {
        drawTextLabel(g, area, text, 7.5f, 0.8f, kLabelTracking,
            juce::Justification::centred);
    }


    // EN: Smaller variant for tight spaces (e.g. ADSR fader captions).
    //     7pt with reduced kerning so the label fits in narrow columns.
    // ES: Variante más pequeña para espacios apretados (p. ej. captions
    //     de los faders de ADSR). 7pt con kerning reducido para que el
    //     texto entre en columnas estrechas.
    void drawTinyControlLabel(juce::Graphics& g,
        juce::Rectangle<int> area,
        const juce::String& text)
    {
        drawTextLabel(g, area, text, 7.0f, 0.8f, kTinyLabelTracking,
            juce::Justification::centred);
    }
}


// ============================================================================
//  2. CONSTRUCTOR
//     CONSTRUCTOR
// ============================================================================

// EN: Builds the GUI. The work is delegated to a sequence of small
//     initialise* helpers, each one responsible for a coherent group
//     of controls. After every widget is alive and wired to its APVTS
//     parameter through initialiseAttachments(), the editor subscribes
//     to the processor's ChangeListener so it can react to "preset
//     became custom" notifications, and runs an initial round of
//     updateXValueLabel() calls so all value labels show the right
//     text before the first paint().
// ES: Construye la GUI. El trabajo se delega a una secuencia de
//     pequeños helpers initialise*, cada uno responsable de un grupo
//     coherente de controles. Después de que todos los widgets estén
//     vivos y cableados a su parámetro APVTS mediante
//     initialiseAttachments(), el editor se suscribe al ChangeListener
//     del processor para reaccionar a las notificaciones de "el preset
//     se volvió custom" y ejecuta una ronda inicial de
//     updateXValueLabel() para que todos los labels de valor muestren
//     el texto correcto antes del primer paint().
AndesJXAudioProcessorEditor::AndesJXAudioProcessorEditor(AndesJXAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // EN: Background and global look-and-feels first: the size of the
    //     editor depends on the background image, and the LookAndFeels
    //     must exist before any control that uses them.
    // ES: Fondo y look-and-feels globales primero: el tamaño del editor
    //     depende de la imagen de fondo, y los LookAndFeels deben
    //     existir antes que cualquier control que los use.
    initialiseBackground();
    initialiseLookAndFeels();

    // EN: Per-group control initialization. Order is not strict, but
    //     the value-labeled controls run their initial updateX calls
    //     at the end of the constructor, so the slider values must
    //     already be wired by the time we get there.
    // ES: Inicialización por grupos de controles. El orden no es
    //     estricto, pero los controles con label de valor ejecutan sus
    //     llamadas iniciales updateX al final del constructor, así que
    //     los valores de los sliders ya deben estar cableados para ese
    //     entonces.
    initialiseOscWaveSelectors();
    initialiseKnobs();
    initialiseAmpEnvelopeControls();
    initialiseFilterEnvelopeADSRControls();
    initialiseOscTuneControl();
    initialiseStereoWidthControl();
    initialiseNoiseControl();
    initialiseOscFineControl();
    initialiseOctaveControl();
    initialiseTuningControl();
    initialiseGlideRateControl();
    initialiseGlideBendControl();
    initialiseVibratoControl();
    initialiseFilterVelocityControl();
    initialiseFilterEnvControl();
    initialiseFilterLFOControl();
    initialiseFilterKeytrackControl();
    initialiseLFORateControl();
    initialiseFilterKeycenterControl();
    initialisePresetSelector();
    initialiseGlideModeSelector();
    initialisePolyToggle();
    initialiseFilterTypeControl();
    initialiseAttachments();

    // EN: Subscribe to "isCustomPreset" notifications from the
    //     processor. The unsubscribe happens in the destructor.
    // ES: Suscribirse a las notificaciones de "isCustomPreset" del
    //     processor. La desuscripción ocurre en el destructor.
    audioProcessor.addChangeListener(this);

    // EN: Initial label refresh. The sliders already hold the right
    //     values (loaded from the APVTS via attachments), but the
    //     labels have not been formatted yet. Each call reads the
    //     slider value and pushes the formatted string to the label.
    //     Visual grouping below mirrors the GUI sections, not any
    //     execution dependency.
    // ES: Refresco inicial de labels. Los sliders ya tienen los valores
    //     correctos (cargados desde el APVTS vía attachments), pero los
    //     labels aún no se han formateado. Cada llamada lee el valor
    //     del slider y envía la string formateada al label. La
    //     agrupación visual de abajo refleja las secciones de la GUI,
    //     no ninguna dependencia de ejecución.
    updateLFORateValueLabel();
    updateFilterLFOValueLabel();

    updateFilterEnvValueLabel();
    updateFilterKeycenterValueLabel();
    updateFilterKeytrackValueLabel();

    updateFilterVelocityValueLabel();
    updateVibratoValueLabel();
    updateGlideBendValueLabel();
    updateGlideRateValueLabel();

    updateOscTuneValueLabel();
    updateStereoWidthValueLabel();
    updateNoiseValueLabel();
    updateOscFineValueLabel();
    updateOctaveValueLabel();
    updateTuningValueLabel();
}


// EN: Loads the background image from BinaryData and sets the editor
//     size to match (downscaled by 4x to keep the artwork crisp at
//     standard plugin window dimensions). If the image fails to load,
//     falls back to a hardcoded 500x430 size so the editor is still
//     usable for testing.
// ES: Carga la imagen de fondo desde BinaryData y ajusta el tamaño del
//     editor para que coincida (reescalada a 1/4 para mantener el arte
//     nítido en las dimensiones estándar de ventana de plugin). Si la
//     imagen no se carga, hace fallback a un tamaño 500x430
//     hardcodeado para que el editor siga siendo usable para pruebas.
void AndesJXAudioProcessorEditor::initialiseBackground()
{
    backgroundAndesJX = juce::ImageCache::getFromMemory(
        BinaryData::backgroundAndesJX_png,
        BinaryData::backgroundAndesJX_pngSize);

    if (backgroundAndesJX.isValid())
    {
        // EN: The source PNG is exported at 4x its logical display size
        //     to give JUCE extra resolution to draw from on HiDPI/Retina
        //     displays. The image is NOT rescaled in memory: keeping the
        //     full 4x pixel data lets paint() pick the right level of
        //     detail at the actual display density (1x, 1.5x, 2x...).
        //     The editor's logical size is set to width/4 and height/4
        //     so the GUI lays out as if the artwork were 1x; JUCE handles
        //     the pixel-mapping internally.
        // ES: El PNG original se exporta a 4x su tamaño lógico de display
        //     para dar a JUCE resolución extra desde la cual dibujar en
        //     pantallas HiDPI/Retina. La imagen NO se reescala en memoria:
        //     mantener los píxeles 4x permite que paint() elija el nivel
        //     de detalle adecuado en la densidad real del display
        //     (1x, 1.5x, 2x...). El tamaño lógico del editor se asigna a
        //     width/4 y height/4 para que la GUI se disponga como si el
        //     arte fuera 1x; JUCE maneja el mapeo de píxeles internamente.
        setSize(backgroundAndesJX.getWidth() / 4,
                backgroundAndesJX.getHeight() / 4);
    }
    else
    {
        setSize(500, 430);
    }
}


// EN: Instantiates each custom LookAndFeel and tweaks the parameters
//     that are exposed for runtime adjustment (font heights, colors,
//     corner radii). The LookAndFeel objects themselves implement the
//     drawing; this function only configures them.
//
//     Six LookAndFeels are owned by the editor:
//       - comboBoxLookAndFeel        : preset selector, wave selectors,
//                                       glide mode
//       - knobPrincipalLookAndFeel    : the four headline knobs
//                                       (mix, cutoff, resonance, output)
//       - secondaryKnobLookAndFeel    : every value-labeled knob
//       - faderLookAndFeel            : the eight ADSR faders
//       - toggleLookAndFeel           : the Mono / Poly toggle
//       - segmentedButtonLookAndFeel  : the SVF / Moog selector
//
// ES: Instancia cada LookAndFeel custom y ajusta los parámetros que se
//     exponen para ajuste en tiempo de ejecución (altos de fuente,
//     colores, radios de esquina). Los objetos LookAndFeel mismos
//     implementan el dibujado; esta función solo los configura.
//
//     El editor posee seis LookAndFeels:
//       - comboBoxLookAndFeel        : selector de presets, selectores
//                                       de onda, modo de glide
//       - knobPrincipalLookAndFeel    : los cuatro knobs principales
//                                       (mix, cutoff, resonance, output)
//       - secondaryKnobLookAndFeel    : cada knob con label de valor
//       - faderLookAndFeel            : los ocho faders de ADSR
//       - toggleLookAndFeel           : el toggle Mono / Poly
//       - segmentedButtonLookAndFeel  : el selector SVF / Moog
void AndesJXAudioProcessorEditor::initialiseLookAndFeels()
{
    comboBoxLookAndFeel = std::make_unique<ComboBoxLookAndFeel>();

    // EN: Headline knobs. Four font heights cover the value text:
    //       - main    (10 pt) for the "loud" output knob.
    //       - compact (8.5 pt) for filterFreq and filterReso, so they
    //                  share the same value-text weight as oscMix.
    //       - unit    (7.5 pt) for the unit suffix in the twoLine
    //                  layout ("dB", "%").
    //       - single  (8.5 pt) for oscMix in the singleLine layout.
    //     Together, mix / cutoff / reso are visually equivalent and
    //     output stands out as the master.
    // ES: Knobs principales. Cuatro altos de fuente cubren el texto
    //     del valor:
    //       - main    (10 pt) para el knob "fuerte" output.
    //       - compact (8.5 pt) para filterFreq y filterReso, así
    //                  comparten el mismo peso de texto de valor que
    //                  oscMix.
    //       - unit    (7.5 pt) para el sufijo de unidad en el layout
    //                  twoLine ("dB", "%").
    //       - single  (8.5 pt) para oscMix en el layout singleLine.
    //     Juntos, mix / cutoff / reso quedan visualmente equivalentes
    //     y output destaca como master.
    knobPrincipalLookAndFeel = std::make_unique<KnobPrincipalLookAndFeel>();
    knobPrincipalLookAndFeel->setMainValueFontHeight(10.0f);
    knobPrincipalLookAndFeel->setCompactValueFontHeight(8.5f);
    knobPrincipalLookAndFeel->setUnitFontHeight(7.5f);
    knobPrincipalLookAndFeel->setSingleLineFontHeight(8.5f);

    // EN: Secondary knobs do not draw their value text themselves; the
    //     editor draws an external Label next to each one. setShowValueText
    //     false avoids duplicating the value on screen.
    // ES: Los knobs secundarios no dibujan su texto de valor; el editor
    //     dibuja un Label externo al lado. setShowValueText false evita
    //     duplicar el valor en pantalla.
    secondaryKnobLookAndFeel = std::make_unique<SecondaryKnobLookAndFeel>();
    secondaryKnobLookAndFeel->setTextFontHeight(7.5f);
    secondaryKnobLookAndFeel->setShowValueText(false);
    secondaryKnobLookAndFeel->setTextInset(3);

    faderLookAndFeel = std::make_unique<FaderLookAndFeel>();

    // EN: Toggle uses theme colors so it adapts to any future palette
    //     change in AndesTheme without modifying this file.
    // ES: El toggle usa colores del tema para adaptarse a cualquier
    //     cambio futuro de paleta en AndesTheme sin modificar este
    //     archivo.
    toggleLookAndFeel = std::make_unique<ToggleLookAndFeel>();
    toggleLookAndFeel->setColour(ToggleLookAndFeel::backgroundOnColourId, AndesTheme::Colours::panel);
    toggleLookAndFeel->setColour(ToggleLookAndFeel::backgroundOffColourId, AndesTheme::Colours::panelDark);
    toggleLookAndFeel->setColour(ToggleLookAndFeel::textColourId, AndesTheme::Colours::text);
    toggleLookAndFeel->setColour(ToggleLookAndFeel::outlineColourId, AndesTheme::Colours::outline);
    toggleLookAndFeel->setToggleFontHeight(10.0f);

    // EN: Segmented button (SVF / Moog selector). Same theme-driven
    //     coloring approach; corner radius 2 matches the rounded-but-
    //     subtle look used elsewhere.
    // ES: Botón segmentado (selector SVF / Moog). Mismo enfoque de
    //     coloración por tema; radio de esquina 2 coincide con el
    //     aspecto redondeado-pero-sutil usado en otras partes.
    segmentedButtonLookAndFeel = std::make_unique<SegmentedButtonLookAndFeel>();
    segmentedButtonLookAndFeel->setDefaultBackground(AndesTheme::Colours::panel);
    segmentedButtonLookAndFeel->setDefaultText(AndesTheme::Colours::text);
    segmentedButtonLookAndFeel->setFontHeight(10.0f);
    segmentedButtonLookAndFeel->setCornerRadius(2.0f);
}
// ============================================================================
//  3. GENERIC WIDGET SETUP HELPERS
//     HELPERS GENÉRICOS PARA WIDGETS
// ============================================================================

// EN: Each setupX function applies the standard configuration for one
//     widget type and registers it in the editor. They are called by
//     the more specific initialise* functions below to avoid repeating
//     the same boilerplate.
//
//     Four setups exist:
//       - setupCombo            : combo boxes (preset, wave, glide mode)
//       - setupKnob             : the four headline rotary knobs
//       - setupSecondaryKnob    : value-labeled rotary knobs (defined
//                                 lower in the file, near the
//                                 envelope helpers)
//       - setupFader            : ADSR vertical faders (also defined
//                                 lower in the file)
//
// ES: Cada función setupX aplica la configuración estándar para un
//     tipo de widget y lo registra en el editor. Las llaman las
//     funciones initialise* más específicas de abajo para evitar
//     repetir el mismo boilerplate.
//
//     Existen cuatro setups:
//       - setupCombo            : combo boxes (preset, onda, glide mode)
//       - setupKnob             : los cuatro knobs rotatorios principales
//       - setupSecondaryKnob    : knobs rotatorios con label de valor
//                                 (definido más abajo, junto a los
//                                 helpers de envolventes)
//       - setupFader            : faders verticales de ADSR (también
//                                 definido más abajo)


// EN: Combo box setup. Applies the AndesJX combo LookAndFeel and the
//     theme background / text colors. Always called before the combo's
//     items are populated.
// ES: Setup de combo box. Aplica el LookAndFeel de combo de AndesJX y
//     los colores de fondo / texto del tema. Se llama antes de poblar
//     los items del combo.
void AndesJXAudioProcessorEditor::setupCombo(juce::ComboBox& combo)
{
    combo.setLookAndFeel(comboBoxLookAndFeel.get());
    combo.setColour(juce::ComboBox::backgroundColourId, AndesTheme::Colours::panel);
    combo.setColour(juce::ComboBox::textColourId, AndesTheme::Colours::text);
    addAndMakeVisible(combo);
}


// EN: Headline knob setup. RotaryHorizontalVerticalDrag means the user
//     can drag the knob in either direction (horizontal or vertical)
//     to change its value, which is the most ergonomic style for plugin
//     knobs. NoTextBox suppresses JUCE's default value box because the
//     KnobPrincipalLookAndFeel draws the value text itself, with the
//     custom typography.
// ES: Setup de knob principal. RotaryHorizontalVerticalDrag permite
//     que el usuario arrastre el knob en cualquier dirección
//     (horizontal o vertical) para cambiar su valor, que es el estilo
//     más ergonómico para knobs de plugin. NoTextBox suprime el cuadro
//     de valor por defecto de JUCE porque el KnobPrincipalLookAndFeel
//     dibuja el texto del valor por su cuenta con la tipografía custom.
void AndesJXAudioProcessorEditor::setupKnob(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setLookAndFeel(knobPrincipalLookAndFeel.get());
    addAndMakeVisible(slider);
}


// ============================================================================
//  4. PER-CONTROL FORMAT / UPDATE / INITIALISE FUNCTIONS
//     FUNCIONES POR CONTROL FORMAT / UPDATE / INITIALISE
// ============================================================================

// EN: Each value-labeled control follows the same three-function pattern:
//
//       formatXValue(value)       returns the user-readable string for
//                                 a given numeric value. This is the
//                                 ONLY part that varies between
//                                 controls; each control has its own
//                                 formatting rules (units, signed
//                                 numbers, special states, alignment
//                                 padding with non-breaking spaces).
//
//       updateXValueLabel()       reads the slider's current value,
//                                 calls formatXValue, and pushes the
//                                 result into the label. Trivial
//                                 one-liner; uniform across controls.
//
//       initialiseXControl()      delegates to initialiseValueLabeledControl
//                                 (defined further down) which applies
//                                 the shared boilerplate (slider style,
//                                 label color, font, intercepts,
//                                 onValueChange callback).
//
//     The block below shows the pattern in full detail using oscTune as
//     the canonical example. The remaining 14 controls follow the same
//     skeleton; only their formatXValue function differs.
//
// ES: Cada control con label de valor sigue el mismo patrón de tres
//     funciones:
//
//       formatXValue(value)       devuelve la string legible para un
//                                 valor numérico dado. Es la ÚNICA
//                                 parte que varía entre controles;
//                                 cada uno tiene sus propias reglas
//                                 de formato (unidades, números con
//                                 signo, estados especiales, padding
//                                 de alineación con espacios
//                                 no-rompibles).
//
//       updateXValueLabel()       lee el valor actual del slider, llama
//                                 a formatXValue, y envía el resultado
//                                 al label. One-liner trivial; uniforme
//                                 en todos los controles.
//
//       initialiseXControl()      delega a initialiseValueLabeledControl
//                                 (definida más abajo) que aplica el
//                                 boilerplate compartido (estilo del
//                                 slider, color del label, font,
//                                 intercepts, callback onValueChange).
//
//     El bloque de abajo muestra el patrón en detalle con oscTune como
//     ejemplo canónico. Los 14 controles restantes siguen el mismo
//     esqueleto; solo difieren en su función formatXValue.


// ----------------------------------------------------------------------------
//  oscTune (canonical example) / oscTune (ejemplo canónico)
// ----------------------------------------------------------------------------

// EN: Format the oscillator coarse tune in semitones, with sign and
//     unit. Three layout cases are handled to keep all values the
//     same width in the GUI:
//       - Zero          -> "  0 st"          (two NBSP + "0")
//       - Single digit  -> " +N st" or " -N st"
//       - Double digit  -> "+NN st" or "-NN st"
//     The non-breaking spaces (0xA0) act as left padding so single-
//     and double-digit values share the same horizontal alignment.
// ES: Formatea la afinación gruesa del oscilador en semitonos, con
//     signo y unidad. Se manejan tres casos de layout para que todos
//     los valores tengan el mismo ancho en la GUI:
//       - Cero            -> "  0 st"          (dos NBSP + "0")
//       - Un solo dígito  -> " +N st" o " -N st"
//       - Dos dígitos     -> "+NN st" o "-NN st"
//     Los espacios no-rompibles (0xA0) actúan como padding a la
//     izquierda para que valores de uno y dos dígitos compartan el
//     mismo alineamiento horizontal.
juce::String AndesJXAudioProcessorEditor::formatOscTuneValue(double value) const
{
    const int semitones = juce::roundToInt(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (semitones == 0)
        return nbsp + nbsp + "0 st";

    if (semitones > 0)
    {
        if (semitones < 10)
            return nbsp + "+" + juce::String(semitones) + " st";

        return "+" + juce::String(semitones) + " st";
    }

    if (std::abs(semitones) < 10)
        return nbsp + juce::String(semitones) + " st";

    return juce::String(semitones) + " st";
}

// EN: Reads the current slider value and pushes the formatted string
//     into the label. The juce::dontSendNotification flag prevents a
//     feedback loop: setting the label's text would otherwise notify
//     listeners and could re-trigger the update.
// ES: Lee el valor actual del slider y envía la string formateada al
//     label. La bandera juce::dontSendNotification evita un loop de
//     retroalimentación: poner el texto del label notificaría a los
//     listeners y podría re-disparar el update.
void AndesJXAudioProcessorEditor::updateOscTuneValueLabel()
{
    oscTuneValueLabel.setText(formatOscTuneValue(oscTuneSlider.getValue()),
        juce::dontSendNotification);
}

// EN: Wires the slider + label pair into the editor through the
//     centralized helper. The lambda passed as the last argument is
//     the per-control update callback that the helper hooks into the
//     slider's onValueChange.
// ES: Cablea el par slider + label en el editor mediante el helper
//     centralizado. La lambda pasada como último argumento es el
//     callback de update específico del control, que el helper conecta
//     a onValueChange del slider.
void AndesJXAudioProcessorEditor::initialiseOscTuneControl()
{
    initialiseValueLabeledControl(oscTuneSlider, oscTuneValueLabel, "oscTune",
        [this]() { updateOscTuneValueLabel(); });
}
// ----------------------------------------------------------------------------
//  stereoWidth / stereoWidth
// ----------------------------------------------------------------------------

// EN: Same width-padding pattern as oscTune, but unsigned (range
//     0..100 %). Three layout cases: 1-digit, 2-digit, 3-digit (only
//     "100" reaches 3 digits).
// ES: Mismo patrón de padding por ancho que oscTune, pero sin signo
//     (rango 0..100 %). Tres casos: 1, 2 y 3 dígitos (solo "100" llega
//     a 3 dígitos).
juce::String AndesJXAudioProcessorEditor::formatStereoWidthValue(double value) const
{
    const int width = juce::roundToInt(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (width == 0)   return nbsp + nbsp + "0 %";
    if (width < 10)   return nbsp + nbsp + juce::String(width) + " %";
    if (width < 100)  return nbsp + juce::String(width) + " %";
    return juce::String(width) + " %";
}

void AndesJXAudioProcessorEditor::updateStereoWidthValueLabel()
{
    stereoWidthValueLabel.setText(formatStereoWidthValue(stereoWidthSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseStereoWidthControl()
{
    initialiseValueLabeledControl(stereoWidthSlider, stereoWidthValueLabel, "stereoWidth",
        [this]() { updateStereoWidthValueLabel(); });
}


// ----------------------------------------------------------------------------
//  noise / noise
// ----------------------------------------------------------------------------

// EN: Identical layout to stereoWidth (unsigned 0..100 %).
// ES: Layout idéntico a stereoWidth (sin signo 0..100 %).
juce::String AndesJXAudioProcessorEditor::formatNoiseValue(double value) const
{
    const int noise = juce::roundToInt(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (noise == 0)   return nbsp + nbsp + "0 %";
    if (noise < 10)   return nbsp + nbsp + juce::String(noise) + " %";
    if (noise < 100)  return nbsp + juce::String(noise) + " %";
    return juce::String(noise) + " %";
}

void AndesJXAudioProcessorEditor::updateNoiseValueLabel()
{
    noiseValueLabel.setText(formatNoiseValue(noiseSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseNoiseControl()
{
    initialiseValueLabeledControl(noiseSlider, noiseValueLabel, "noise",
        [this]() { updateNoiseValueLabel(); });
}


// ----------------------------------------------------------------------------
//  oscFine / oscFine
// ----------------------------------------------------------------------------

// EN: Fine tune in cents, signed and one-decimal. The "near zero"
//     threshold is 0.05 (half a tenth) so that values below the
//     display resolution snap to "0.0 c" instead of jittering between
//     "+0.0 c" and "-0.0 c". Three NBSPs handle the cases where the
//     formatted number is shorter than the +/- sign + digit.
// ES: Afinación fina en cents, con signo y un decimal. El umbral
//     "cerca de cero" es 0.05 (media décima) para que los valores
//     debajo de la resolución del display caigan a "0.0 c" en lugar
//     de oscilar entre "+0.0 c" y "-0.0 c". Tres NBSPs manejan los
//     casos donde el número formateado es más corto que el signo
//     +/- más el dígito.
juce::String AndesJXAudioProcessorEditor::formatOscFineValue(double value) const
{
    const float cents = static_cast<float>(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (std::abs(cents) < 0.05f)
        return nbsp + nbsp + nbsp + "0.0 c";

    if (cents > 0.0f)
    {
        if (cents < 10.0f)
            return nbsp + nbsp + "+" + juce::String(cents, 1) + " c";

        return "+" + juce::String(cents, 1) + " c";
    }

    if (std::abs(cents) < 10.0f)
        return nbsp + nbsp + juce::String(cents, 1) + " c";

    return juce::String(cents, 1) + " c";
}

void AndesJXAudioProcessorEditor::updateOscFineValueLabel()
{
    oscFineValueLabel.setText(formatOscFineValue(oscFineSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseOscFineControl()
{
    initialiseValueLabeledControl(oscFineSlider, oscFineValueLabel, "oscFine",
        [this]() { updateOscFineValueLabel(); });
}


// ----------------------------------------------------------------------------
//  octave / octave
// ----------------------------------------------------------------------------

// EN: Octave shift in [-2, +2] integer range. Single-character magnitude
//     so just two padding cases: zero and signed.
// ES: Desplazamiento de octava en rango entero [-2, +2]. La magnitud
//     es de un solo carácter, así que solo hay dos casos de padding:
//     cero y con signo.
juce::String AndesJXAudioProcessorEditor::formatOctaveValue(double value) const
{
    const int oct = juce::roundToInt(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (oct == 0)  return nbsp + nbsp + "0 oct";
    if (oct > 0)   return nbsp + "+" + juce::String(oct) + " oct";
    return nbsp + juce::String(oct) + " oct";
}

void AndesJXAudioProcessorEditor::updateOctaveValueLabel()
{
    octaveValueLabel.setText(formatOctaveValue(octaveSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseOctaveControl()
{
    initialiseValueLabeledControl(octaveSlider, octaveValueLabel, "octave",
        [this]() { updateOctaveValueLabel(); });
}


// ----------------------------------------------------------------------------
//  tuning / tuning
// ----------------------------------------------------------------------------

// EN: Same logic as oscFine (signed, one decimal, cents) but with a
//     subtly different padding: tuning uses two NBSPs for zero and one
//     NBSP for sub-10 values, because the typical visible range is
//     [-100, +100] cents (three-digit max width is shorter than oscFine
//     by one position).
// ES: Misma lógica que oscFine (con signo, un decimal, cents) pero con
//     un padding sutilmente distinto: tuning usa dos NBSPs para cero y
//     un NBSP para valores menores de 10, porque el rango visible
//     típico es [-100, +100] cents (el ancho máximo de tres dígitos es
//     una posición más corto que oscFine).
juce::String AndesJXAudioProcessorEditor::formatTuningValue(double value) const
{
    const float cents = static_cast<float>(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (std::abs(cents) < 0.05f)
        return nbsp + nbsp + nbsp + "0.0 c";

    if (cents > 0.0f)
    {
        if (cents < 10.0f)
            return nbsp + "+" + juce::String(cents, 1) + " c";

        return "+" + juce::String(cents, 1) + " c";
    }

    if (std::abs(cents) < 10.0f)
        return nbsp + juce::String(cents, 1) + " c";

    return juce::String(cents, 1) + " c";
}

void AndesJXAudioProcessorEditor::updateTuningValueLabel()
{
    tuningValueLabel.setText(formatTuningValue(tuningSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseTuningControl()
{
    initialiseValueLabeledControl(tuningSlider, tuningValueLabel, "tuning",
        [this]() { updateTuningValueLabel(); });
}


// ----------------------------------------------------------------------------
//  glideRate / glideRate
// ----------------------------------------------------------------------------

// EN: Identical layout to stereoWidth (unsigned 0..100 %).
// ES: Layout idéntico a stereoWidth (sin signo 0..100 %).
juce::String AndesJXAudioProcessorEditor::formatGlideRateValue(double value) const
{
    const int rate = juce::roundToInt(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (rate == 0)   return nbsp + nbsp + "0 %";
    if (rate < 10)   return nbsp + nbsp + juce::String(rate) + " %";
    if (rate < 100)  return nbsp + juce::String(rate) + " %";
    return juce::String(rate) + " %";
}

void AndesJXAudioProcessorEditor::updateGlideRateValueLabel()
{
    glideRateValueLabel.setText(formatGlideRateValue(glideRateSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseGlideRateControl()
{
    initialiseValueLabeledControl(glideRateSlider, glideRateValueLabel, "glideRate",
        [this]() { updateGlideRateValueLabel(); });
}


// ----------------------------------------------------------------------------
//  glideBend / glideBend
// ----------------------------------------------------------------------------

// EN: Glide bend in semitones, signed and one-decimal. Same shape as
//     oscFine but with " st" suffix and a tighter zero threshold
//     (0.005) because the parameter step is finer.
// ES: Bend de glide en semitonos, con signo y un decimal. Misma forma
//     que oscFine pero con sufijo " st" y umbral de cero más ajustado
//     (0.005) porque el paso del parámetro es más fino.
juce::String AndesJXAudioProcessorEditor::formatGlideBendValue(double value) const
{
    const float semis = static_cast<float>(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (std::abs(semis) < 0.005f)
        return nbsp + nbsp + nbsp + "0.0 st";

    if (semis > 0.0f)
    {
        if (semis < 10.0f)
            return nbsp + "+" + juce::String(semis, 1) + " st";

        return "+" + juce::String(semis, 1) + " st";
    }

    if (std::abs(semis) < 10.0f)
        return nbsp + juce::String(semis, 1) + " st";

    return juce::String(semis, 1) + " st";
}

void AndesJXAudioProcessorEditor::updateGlideBendValueLabel()
{
    glideBendValueLabel.setText(formatGlideBendValue(glideBendSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseGlideBendControl()
{
    initialiseValueLabeledControl(glideBendSlider, glideBendValueLabel, "glideBend",
        [this]() { updateGlideBendValueLabel(); });
}


// ----------------------------------------------------------------------------
//  vibrato (bipolar Vibrato / PWM) / vibrato (Vibrato / PWM bipolar)
// ----------------------------------------------------------------------------

// EN: Special bipolar formatter. The vibrato parameter is one knob with
//     two functions (see PluginProcessor::update): positive values
//     drive vibrato, negative values drive PWM. The label reflects this
//     with three distinct strings:
//       - "OFF"          when the absolute value is below 0.05
//       - "PWM <n>"      when the value is negative (PWM depth = -value)
//       - "VIB <n>"      when the value is positive (vibrato depth)
//     This is the most user-facing example in the GUI of the
//     "one knob, two functions" decision documented in the processor.
// ES: Formateador bipolar especial. El parámetro vibrato es un único
//     knob con dos funciones (ver PluginProcessor::update): valores
//     positivos activan vibrato, valores negativos activan PWM. El
//     label refleja esto con tres strings distintas:
//       - "OFF"          cuando el valor absoluto está por debajo de 0.05
//       - "PWM <n>"      cuando el valor es negativo (profundidad PWM = -valor)
//       - "VIB <n>"      cuando el valor es positivo (profundidad de vibrato)
//     Es el ejemplo más visible de la GUI de la decisión "un knob, dos
//     funciones" documentada en el processor.
juce::String AndesJXAudioProcessorEditor::formatVibratoValue(double value) const
{
    const float v = static_cast<float>(value);

    if (std::abs(v) < 0.05f)
        return "OFF";

    if (v < 0.0f)
        return "PWM " + juce::String(-v, 1);

    return "VIB " + juce::String(v, 1);
}

void AndesJXAudioProcessorEditor::updateVibratoValueLabel()
{
    vibratoValueLabel.setText(formatVibratoValue(vibratoSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseVibratoControl()
{
    initialiseValueLabeledControl(vibratoSlider, vibratoValueLabel, "vibrato",
        [this]() { updateVibratoValueLabel(); });
}


// ----------------------------------------------------------------------------
//  filterVelocity / filterVelocity
// ----------------------------------------------------------------------------

// EN: Bipolar percentage with an "OFF" sentinel below -90. The OFF
//     state matches PluginProcessor::update, where filterVelocity < -90
//     disables the velocity tracking entirely (instead of being a small
//     negative amount). This makes the GUI honest about the special
//     state instead of showing "-95 %".
// ES: Porcentaje bipolar con un sentinel "OFF" por debajo de -90. El
//     estado OFF coincide con PluginProcessor::update, donde
//     filterVelocity < -90 deshabilita el seguimiento de velocity por
//     completo (en lugar de ser una cantidad negativa pequeña). Hace
//     que la GUI sea honesta sobre el estado especial en lugar de
//     mostrar "-95 %".
juce::String AndesJXAudioProcessorEditor::formatFilterVelocityValue(double value) const
{
    const float v = static_cast<float>(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (v < -90.0f)
        return "OFF";

    if (std::abs(v) < 0.5f)
        return nbsp + nbsp + "0 %";

    if (v > 0.0f)
    {
        const int iv = juce::roundToInt(v);

        if (iv < 10)
            return nbsp + "+" + juce::String(iv) + " %";

        return "+" + juce::String(iv) + " %";
    }

    const int iv = juce::roundToInt(v);

    if (std::abs(iv) < 10)
        return nbsp + juce::String(iv) + " %";

    return juce::String(iv) + " %";
}

void AndesJXAudioProcessorEditor::updateFilterVelocityValueLabel()
{
    filterVelocityValueLabel.setText(formatFilterVelocityValue(filterVelocitySlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseFilterVelocityControl()
{
    initialiseValueLabeledControl(filterVelocitySlider, filterVelocityValueLabel, "filterVelocity",
        [this]() { updateFilterVelocityValueLabel(); });
}


// ----------------------------------------------------------------------------
//  filterEnv / filterEnv
// ----------------------------------------------------------------------------

// EN: Signed integer percentage. Same shape as filterVelocity minus
//     the OFF sentinel (filterEnv has no special "off" state; the
//     center of the bipolar range simply means "no envelope effect").
// ES: Porcentaje entero con signo. Misma forma que filterVelocity pero
//     sin el sentinel OFF (filterEnv no tiene estado especial "off";
//     el centro del rango bipolar simplemente significa "sin efecto
//     de envolvente").
juce::String AndesJXAudioProcessorEditor::formatFilterEnvValue(double value) const
{
    const float v = static_cast<float>(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (std::abs(v) < 0.05f)
        return nbsp + nbsp + "0 %";

    if (v > 0.0f)
    {
        const int iv = juce::roundToInt(v);

        if (iv < 10)
            return nbsp + "+" + juce::String(iv) + " %";

        return "+" + juce::String(iv) + " %";
    }

    const int iv = juce::roundToInt(v);

    if (std::abs(iv) < 10)
        return nbsp + juce::String(iv) + " %";

    return juce::String(iv) + " %";
}

void AndesJXAudioProcessorEditor::updateFilterEnvValueLabel()
{
    filterEnvValueLabel.setText(formatFilterEnvValue(filterEnvSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseFilterEnvControl()
{
    initialiseValueLabeledControl(filterEnvSlider, filterEnvValueLabel, "filterEnv",
        [this]() { updateFilterEnvValueLabel(); });
}


// ----------------------------------------------------------------------------
//  filterLFO / filterLFO
// ----------------------------------------------------------------------------

// EN: Identical layout to stereoWidth (unsigned 0..100 %).
// ES: Layout idéntico a stereoWidth (sin signo 0..100 %).
juce::String AndesJXAudioProcessorEditor::formatFilterLFOValue(double value) const
{
    const int lfo = juce::roundToInt(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (lfo == 0)   return nbsp + nbsp + "0 %";
    if (lfo < 10)   return nbsp + nbsp + juce::String(lfo) + " %";
    if (lfo < 100)  return nbsp + juce::String(lfo) + " %";
    return juce::String(lfo) + " %";
}

void AndesJXAudioProcessorEditor::updateFilterLFOValueLabel()
{
    filterLFOValueLabel.setText(formatFilterLFOValue(filterLFOSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseFilterLFOControl()
{
    initialiseValueLabeledControl(filterLFOSlider, filterLFOValueLabel, "filterLFO",
        [this]() { updateFilterLFOValueLabel(); });
}


// ----------------------------------------------------------------------------
//  filterKeytrack / filterKeytrack
// ----------------------------------------------------------------------------

// EN: Unsigned percentage in [0, 200] %. Same shape as stereoWidth;
//     the only difference is that the range goes up to 200 instead of
//     100, but the formatter only cares about the digit count, so the
//     three-case logic (0 / <10 / <100) still applies cleanly with a
//     "no padding" fallback for 100..200.
// ES: Porcentaje sin signo en [0, 200] %. Misma forma que stereoWidth;
//     la única diferencia es que el rango llega a 200 en lugar de 100,
//     pero al formateador solo le importa la cantidad de dígitos, así
//     que la lógica de tres casos (0 / <10 / <100) sigue aplicando
//     limpiamente con un fallback "sin padding" para 100..200.
juce::String AndesJXAudioProcessorEditor::formatFilterKeytrackValue(double value) const
{
    const int keytrack = juce::roundToInt(value);
    const juce::String nbsp = juce::String::charToString(0xA0);

    if (keytrack == 0)   return nbsp + nbsp + "0 %";
    if (keytrack < 10)   return nbsp + nbsp + juce::String(keytrack) + " %";
    if (keytrack < 100)  return nbsp + juce::String(keytrack) + " %";
    return juce::String(keytrack) + " %";
}

void AndesJXAudioProcessorEditor::updateFilterKeytrackValueLabel()
{
    filterKeytrackValueLabel.setText(formatFilterKeytrackValue(filterKeytrackSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseFilterKeytrackControl()
{
    initialiseValueLabeledControl(filterKeytrackSlider, filterKeytrackValueLabel, "filterKeytrack",
        [this]() { updateFilterKeytrackValueLabel(); });
}


// ----------------------------------------------------------------------------
//  lfoRate / lfoRate
// ----------------------------------------------------------------------------

// EN: LFO rate display. The slider is exposed as a normalised [0, 1]
//     value, so the formatter MIRRORS the same exp(7x - 4) mapping
//     used in PluginProcessor::update before showing the result in Hz.
//     If that mapping is ever changed in the processor, this formula
//     must be updated too to keep the displayed Hz consistent with the
//     actual rate.
// ES: Display de la rate del LFO. El slider se expone como valor
//     normalizado [0, 1], así que el formateador REPLICA el mismo
//     mapeo exp(7x - 4) que usa PluginProcessor::update antes de
//     mostrar el resultado en Hz. Si ese mapeo llegara a cambiar en
//     el processor, esta fórmula debe actualizarse también para que
//     los Hz mostrados sigan siendo consistentes con la rate real.
juce::String AndesJXAudioProcessorEditor::formatLFORateValue(double value) const
{
    const float normalised = static_cast<float>(value);
    const float lfoHz = std::exp(7.0f * normalised - 4.0f);

    return juce::String(lfoHz, 3) + " Hz";
}

void AndesJXAudioProcessorEditor::updateLFORateValueLabel()
{
    lfoRateValueLabel.setText(formatLFORateValue(lfoRateSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseLFORateControl()
{
    initialiseValueLabeledControl(lfoRateSlider, lfoRateValueLabel, "lfoRate",
        [this]() { updateLFORateValueLabel(); });
}


// ----------------------------------------------------------------------------
//  filterKeycenter (note name) / filterKeycenter (nombre de nota)
// ----------------------------------------------------------------------------

// EN: Special formatter that turns a MIDI note number into the
//     standard "<name><octave>" notation (e.g. 60 -> "C4", 69 -> "A4").
//     Two conventions worth pinning down:
//       - The note-name table is sharps-only (no flats) for visual
//         consistency.
//       - The "octave" subtraction of 1 reflects the convention where
//         MIDI 60 = C4 (Yamaha / common DAW convention). Some MIDI
//         tools place MIDI 60 at C3; if you ever switch convention,
//         change the "- 1" here.
//     The initialiser also installs a setDoubleClickReturnValue at 60
//     so the user can quickly snap back to Middle C.
// ES: Formateador especial que convierte un número de nota MIDI en la
//     notación estándar "<nombre><octava>" (p. ej. 60 -> "C4",
//     69 -> "A4"). Dos convenciones que vale la pena fijar:
//       - La tabla de nombres usa solo sostenidos (no bemoles) por
//         consistencia visual.
//       - La resta "- 1" en la octava refleja la convención donde
//         MIDI 60 = C4 (convención Yamaha / DAWs habituales). Algunos
//         tools MIDI ubican MIDI 60 en C3; si se cambia de convención,
//         hay que tocar el "- 1" aquí.
//     El inicializador además instala un setDoubleClickReturnValue en
//     60 para que el usuario pueda volver rápidamente a Do central.
juce::String AndesJXAudioProcessorEditor::formatFilterKeycenterValue(double value) const
{
    static const char* noteNames[] =
    {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };

    const int note = juce::roundToInt(value);
    const int octave = (note / 12) - 1;
    const int noteNameIndex = note % 12;

    return juce::String(noteNames[noteNameIndex]) + juce::String(octave);
}

void AndesJXAudioProcessorEditor::updateFilterKeycenterValueLabel()
{
    filterKeycenterValueLabel.setText(formatFilterKeycenterValue(filterKeycenterSlider.getValue()),
        juce::dontSendNotification);
}

void AndesJXAudioProcessorEditor::initialiseFilterKeycenterControl()
{
    initialiseValueLabeledControl(filterKeycenterSlider, filterKeycenterValueLabel, "filterKeycenter",
        [this]() { updateFilterKeycenterValueLabel(); });

    // EN: Double-clicking the knob snaps to MIDI note 60 = C4 (Middle C).
    // ES: Doble clic sobre el knob salta a la nota MIDI 60 = C4 (Do central).
    filterKeycenterSlider.setDoubleClickReturnValue(true, 60.0);
}

// ----------------------------------------------------------------------------
//  setupFader / setupFader
// ----------------------------------------------------------------------------

// EN: Fader setup. LinearVertical is the standard fader orientation
//     (mixing console style). Same NoTextBox / custom LookAndFeel
//     pattern as setupKnob: the FaderLookAndFeel paints any value text
//     itself, JUCE does not.
// ES: Setup de fader. LinearVertical es la orientación estándar de
//     fader (estilo consola de mezclas). Mismo patrón NoTextBox /
//     LookAndFeel custom que setupKnob: el FaderLookAndFeel pinta el
//     texto del valor por su cuenta, no JUCE.
void AndesJXAudioProcessorEditor::setupFader(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setLookAndFeel(faderLookAndFeel.get());
    addAndMakeVisible(slider);
}


// ============================================================================
//  5. ENVELOPE, OSCILLATOR, PRESET AND TOGGLE INITIALIZERS
//     INICIALIZADORES DE ENVOLVENTES, OSCILADORES, PRESETS Y TOGGLES
// ============================================================================

// EN: Builds the four amplitude-envelope faders (ADSR). The
//     componentIDs are intentionally prefixed "env*" rather than
//     "amp*" because that matches the APVTS parameter IDs declared in
//     PluginProcessor.h: envAttack, envDecay, envSustain, envRelease.
//     The amp* naming on the slider variables themselves is a UI-side
//     convention to distinguish them from the filter envelope faders.
// ES: Construye los cuatro faders de la envolvente de amplitud (ADSR).
//     Los componentIDs llevan a propósito el prefijo "env*" en lugar
//     de "amp*" porque coincide con los IDs de parámetro APVTS
//     declarados en PluginProcessor.h: envAttack, envDecay, envSustain,
//     envRelease. El naming amp* en las variables de slider es una
//     convención del lado UI para distinguirlas de los faders de la
//     envolvente de filtro.
void AndesJXAudioProcessorEditor::initialiseAmpEnvelopeControls()
{
    setupFader(ampAttackSlider);
    setupFader(ampDecaySlider);
    setupFader(ampSustainSlider);
    setupFader(ampReleaseSlider);

    ampAttackSlider.setComponentID("envAttack");
    ampDecaySlider.setComponentID("envDecay");
    ampSustainSlider.setComponentID("envSustain");
    ampReleaseSlider.setComponentID("envRelease");
}


// EN: Builds the four filter-envelope faders. Same shape as
//     initialiseAmpEnvelopeControls(); both bound to filter*Attack /
//     Decay / Sustain / Release in the APVTS.
// ES: Construye los cuatro faders de la envolvente de filtro. Misma
//     forma que initialiseAmpEnvelopeControls(); ambos vinculados a
//     filter*Attack / Decay / Sustain / Release en el APVTS.
void AndesJXAudioProcessorEditor::initialiseFilterEnvelopeADSRControls()
{
    setupFader(filterAttackSlider);
    setupFader(filterDecaySlider);
    setupFader(filterSustainSlider);
    setupFader(filterReleaseSlider);

    filterAttackSlider.setComponentID("filterAttack");
    filterDecaySlider.setComponentID("filterDecay");
    filterSustainSlider.setComponentID("filterSustain");
    filterReleaseSlider.setComponentID("filterRelease");
}


// ----------------------------------------------------------------------------
//  setupSecondaryKnob / setupSecondaryKnob
// ----------------------------------------------------------------------------

// EN: Secondary knob setup. Identical to setupKnob except for the
//     LookAndFeel (SecondaryKnob has smaller dimensions and no
//     internal value text, since the editor draws an external label
//     beside each one). Used through initialiseValueLabeledControl
//     below by all 15 value-labeled knobs.
// ES: Setup de knob secundario. Idéntico a setupKnob salvo por el
//     LookAndFeel (SecondaryKnob tiene dimensiones más pequeñas y no
//     dibuja texto interno de valor, ya que el editor pinta un label
//     externo al lado). Lo usan a través de initialiseValueLabeledControl
//     más abajo los 15 knobs con label de valor.
void AndesJXAudioProcessorEditor::setupSecondaryKnob(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setLookAndFeel(secondaryKnobLookAndFeel.get());
    addAndMakeVisible(slider);
}


// ----------------------------------------------------------------------------
//  initialiseValueLabeledControl (the centralized helper)
//  initialiseValueLabeledControl (el helper centralizado)
// ----------------------------------------------------------------------------

// EN: Centralizes the boilerplate shared by every value-labeled knob:
//
//       1. Apply the SecondaryKnob look (delegated to setupSecondaryKnob).
//       2. Set the slider's componentID (used by paint() to lay out
//          the matching caption text on the background).
//       3. Format the label: centred, theme color, 7.5pt font, mouse
//          clicks pass through (so a stray click on the label does not
//          steal focus from the knob).
//       4. Run the initial update so the label shows the right text
//          before the first paint.
//       5. Wire onValueChange to refresh the label every time the
//          slider moves.
//
//     The lambda updateFn captures the per-control updateXValueLabel()
//     function. It is taken by value (not by reference) so the
//     captured object lives inside the slider's onValueChange handler
//     for as long as the slider exists.
//
//     Before this helper, every initialiseXControl repeated the same
//     ~13 lines of code. After centralization, each one is a single
//     call to this function, totalling ~190 fewer lines across the
//     file.
//
// ES: Centraliza el boilerplate compartido por cada knob con label de
//     valor:
//
//       1. Aplicar el look de SecondaryKnob (delegado a
//          setupSecondaryKnob).
//       2. Asignar el componentID del slider (lo usa paint() para
//          ubicar el texto de caption correspondiente en el fondo).
//       3. Formatear el label: centrado, color del tema, fuente 7.5pt,
//          los clics de mouse lo atraviesan (para que un clic
//          accidental en el label no le robe el foco al knob).
//       4. Ejecutar el update inicial para que el label muestre el
//          texto correcto antes del primer paint.
//       5. Cablear onValueChange para refrescar el label cada vez que
//          el slider se mueva.
//
//     La lambda updateFn captura la función updateXValueLabel() del
//     control específico. Se toma por valor (no por referencia) para
//     que el objeto capturado viva dentro del handler onValueChange
//     del slider mientras el slider exista.
//
//     Antes de este helper, cada initialiseXControl repetía las mismas
//     ~13 líneas de código. Después de centralizar, cada uno es una
//     sola llamada a esta función, lo que ahorra ~190 líneas en
//     total.
void AndesJXAudioProcessorEditor::initialiseValueLabeledControl(juce::Slider& slider,
    juce::Label& label,
    const juce::String& mycomponentID,
    std::function<void()> updateFn)
{
    setupSecondaryKnob(slider);
    slider.setComponentID(mycomponentID);

    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, AndesTheme::Colours::text);
    label.setFont(juce::Font(juce::FontOptions(7.5f)));
    label.setInterceptsMouseClicks(false, false);

    updateFn();

    slider.onValueChange = [updateFn]() { updateFn(); };

    addAndMakeVisible(label);
}


// ----------------------------------------------------------------------------
//  Oscillator wave selectors / Selectores de forma de onda
// ----------------------------------------------------------------------------

// EN: Populates the two oscillator-wave combo boxes with the same
//     items in the same order: 1=Sine, 2=Saw, 3=Square, 4=Triangle,
//     5=PWM. The IDs (1..5) are the visible-to-host item IDs, which
//     JUCE maps to choice indices [0..4]; this index ordering MUST
//     match the toWave() lambda in PluginProcessor::update.
// ES: Pobla los dos combo boxes de forma de onda con los mismos items
//     en el mismo orden: 1=Sine, 2=Saw, 3=Square, 4=Triangle, 5=PWM.
//     Los IDs (1..5) son los IDs visibles para el host, que JUCE mapea
//     a índices de choice [0..4]; este orden de índices DEBE coincidir
//     con la lambda toWave() en PluginProcessor::update.
void AndesJXAudioProcessorEditor::initialiseOscWaveSelectors()
{
    oscWaveSelector.addItem("Sine", 1);
    oscWaveSelector.addItem("Saw", 2);
    oscWaveSelector.addItem("Square", 3);
    oscWaveSelector.addItem("Triangle", 4);
    oscWaveSelector.addItem("PWM", 5);
    setupCombo(oscWaveSelector);

    osc2WaveSelector.addItem("Sine", 1);
    osc2WaveSelector.addItem("Saw", 2);
    osc2WaveSelector.addItem("Square", 3);
    osc2WaveSelector.addItem("Triangle", 4);
    osc2WaveSelector.addItem("PWM", 5);
    setupCombo(osc2WaveSelector);
}


// ----------------------------------------------------------------------------
//  Headline knobs / Knobs principales
// ----------------------------------------------------------------------------

// EN: The four headline rotary knobs of AndesJX: oscillator mix,
//     filter resonance, filter cutoff, master output. Their
//     componentIDs match the APVTS parameter IDs except for `output`,
//     which is named `outputLevel` in the APVTS — this asymmetry is
//     historical and harmless (componentIDs are only consumed by
//     paint() for label layout, not by attachments).
// ES: Los cuatro knobs rotatorios principales de AndesJX: mezcla de
//     osciladores, resonancia de filtro, cutoff de filtro, salida
//     master. Sus componentIDs coinciden con los IDs de parámetro
//     APVTS excepto `output`, que en el APVTS se llama `outputLevel`
//     — esta asimetría es histórica e inofensiva (los componentIDs los
//     consume solo paint() para el layout de labels, no los
//     attachments).
void AndesJXAudioProcessorEditor::initialiseKnobs()
{
    setupKnob(mixSlider);
    setupKnob(resonanceSlider);
    setupKnob(cutoffSlider);
    setupKnob(outputSlider);

    mixSlider.setComponentID("oscMix");
    resonanceSlider.setComponentID("filterReso");
    cutoffSlider.setComponentID("filterFreq");
    outputSlider.setComponentID("output");
}


// ----------------------------------------------------------------------------
//  Preset selector / Selector de presets
// ----------------------------------------------------------------------------

// EN: Populates the preset combo with the names of every factory
//     program followed by a separator and a final "Custom" entry
//     (id 1000) used when the active state no longer matches any
//     factory preset.
//
//     The selector is NOT bound through an APVTS attachment because
//     "currentProgram" is not an APVTS parameter — it is a JUCE
//     program index (see PluginProcessor::setCurrentProgram). Instead,
//     a manual onChange callback bridges the combo to the processor:
//     valid factory IDs (1..numPrograms) call setCurrentProgram; the
//     id 1000 (Custom) is selected by the editor (in
//     changeListenerCallback) but cannot be selected by the user
//     directly.
//
// ES: Pobla el combo de presets con los nombres de cada programa de
//     fábrica seguidos de un separador y una entrada final "Custom"
//     (id 1000) que se usa cuando el estado activo ya no coincide con
//     ningún preset de fábrica.
//
//     El selector NO se vincula vía attachment del APVTS porque
//     "currentProgram" no es un parámetro APVTS — es un índice de
//     programa JUCE (ver PluginProcessor::setCurrentProgram). En su
//     lugar, un callback onChange manual hace de puente entre el combo
//     y el processor: IDs válidos de fábrica (1..numPrograms) llaman a
//     setCurrentProgram; el id 1000 (Custom) lo selecciona el editor
//     (en changeListenerCallback) pero el usuario no puede elegirlo
//     directamente.
void AndesJXAudioProcessorEditor::initialisePresetSelector()
{
    setupCombo(presetSelector);

    presetSelector.clear();

    const int numPrograms = audioProcessor.getNumPrograms();
    for (int i = 0; i < numPrograms; ++i)
        presetSelector.addItem(audioProcessor.getProgramName(i), i + 1);

    presetSelector.addSeparator();
    presetSelector.addItem("Custom", 1000);

    // EN: Initial selection: if the loaded state is custom, show
    //     "Custom"; otherwise show the active preset.
    // ES: Selección inicial: si el estado cargado es custom, mostrar
    //     "Custom"; si no, mostrar el preset activo.
    if (audioProcessor.isCustomPresetActive())
        presetSelector.setSelectedId(1000, juce::dontSendNotification);
    else
        presetSelector.setSelectedId(audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);

    presetSelector.onChange = [this]()
        {
            const int id = presetSelector.getSelectedId();

            // EN: Only react to factory IDs. The "Custom" id (1000) lands
            //     here too when the editor sets it programmatically, but
            //     should not trigger a load.
            // ES: Solo reaccionar a IDs de fábrica. El id "Custom" (1000)
            //     también llega aquí cuando el editor lo asigna
            //     programáticamente, pero no debe disparar una carga.
            if (id >= 1 && id <= audioProcessor.getNumPrograms())
                audioProcessor.setCurrentProgram(id - 1);
        };
}


// ----------------------------------------------------------------------------
//  Glide mode selector / Selector de modo de glide
// ----------------------------------------------------------------------------

// EN: Three options matching the glideMode APVTS parameter:
//       1 = Off, 2 = Legato, 3 = Always.
//     The actual binding to the APVTS happens in initialiseAttachments();
//     this function only populates the menu.
// ES: Tres opciones que coinciden con el parámetro APVTS glideMode:
//       1 = Off, 2 = Legato, 3 = Always.
//     El vínculo real con el APVTS ocurre en initialiseAttachments();
//     esta función solo pobla el menú.
void AndesJXAudioProcessorEditor::initialiseGlideModeSelector()
{
    glideModeSelector.addItem("Off", 1);
    glideModeSelector.addItem("Legato", 2);
    glideModeSelector.addItem("Always", 3);
    setupCombo(glideModeSelector);
}


// ----------------------------------------------------------------------------
//  Polyphony toggle / Toggle de polifonía
// ----------------------------------------------------------------------------

// EN: Mono / Poly toggle. Wired through a manual onClick handler
//     instead of an APVTS attachment because polyMode is an
//     AudioParameterChoice (Mono/Poly) and JUCE does not provide a
//     direct attachment from ToggleButton to AudioParameterChoice.
//     The handler reads the current choice index, toggles it, and
//     writes the new value through setValueNotifyingHost so the host
//     learns about the change (and reflects it in any automation).
//     The button text is updated in lockstep so the label always
//     matches the underlying state.
//     A parameterChanged listener is also installed so that external
//     changes (host automation, preset load) keep the toggle in sync.
// ES: Toggle Mono / Poly. Se cablea con un handler onClick manual en
//     lugar de un attachment del APVTS porque polyMode es
//     AudioParameterChoice (Mono/Poly) y JUCE no provee un attachment
//     directo de ToggleButton a AudioParameterChoice. El handler lee
//     el índice actual del choice, lo toggle-a y escribe el nuevo
//     valor con setValueNotifyingHost para que el host se entere del
//     cambio (y lo refleje en cualquier automatización). El texto del
//     botón se actualiza al mismo tiempo para que el label siempre
//     coincida con el estado real.
//     También se instala un listener parameterChanged para que los
//     cambios externos (automatización del host, carga de preset)
//     mantengan el toggle sincronizado.
void AndesJXAudioProcessorEditor::initialisePolyToggle()
{
    polyToggle.setLookAndFeel(toggleLookAndFeel.get());
    polyToggle.setButtonText("Mono");
    polyToggle.setToggleState(false, juce::dontSendNotification);
    polyToggle.setTooltip("Mono / Poly");
    addAndMakeVisible(polyToggle);

    if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(
        audioProcessor.apvts.getParameter("polyMode")))
    {
        const bool isPoly = (param->getIndex() == 1);
        polyToggle.setToggleState(isPoly, juce::dontSendNotification);
        polyToggle.setButtonText(isPoly ? "Poly" : "Mono");

        polyToggle.onClick = [this, param]()
            {
                const int cur = param->getIndex();
                const int next = (cur == 0) ? 1 : 0;
                param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(next)));
                polyToggle.setButtonText(next == 1 ? "Poly" : "Mono");
            };

        audioProcessor.apvts.addParameterListener("polyMode", this);
    }
}


// ----------------------------------------------------------------------------
//  Filter type segmented control / Control segmentado de tipo de filtro
// ----------------------------------------------------------------------------

// EN: Wires the SegmentedControl to the filterType APVTS parameter.
//     Same dual-binding strategy as polyToggle: SegmentedControl
//     itself has no JUCE attachment, so the editor pushes user
//     changes (onChange callback) and pulls external changes
//     (parameterChanged listener) manually.
//     The radio group ID 1002 is arbitrary; it just needs to be unique
//     within the editor so the segmented buttons treat each other as
//     mutually exclusive.
// ES: Cablea el SegmentedControl al parámetro APVTS filterType. Misma
//     estrategia de doble enlace que polyToggle: SegmentedControl no
//     tiene attachment de JUCE, así que el editor empuja los cambios
//     del usuario (callback onChange) y tira de los cambios externos
//     (listener parameterChanged) manualmente.
//     El radio group ID 1002 es arbitrario; solo necesita ser único
//     dentro del editor para que los botones segmentados se traten
//     entre sí como mutuamente excluyentes.
void AndesJXAudioProcessorEditor::initialiseFilterTypeControl()
{
    filterTypeControl.setLookAndFeelForButtons(segmentedButtonLookAndFeel.get());
    filterTypeControl.setItems({ "SVF", "Moog" }, 1002);
    addAndMakeVisible(filterTypeControl);

    if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(
        audioProcessor.apvts.getParameter("filterType")))
    {
        filterTypeControl.setSelectedIndex(param->getIndex(), juce::dontSendNotification);

        filterTypeControl.onChange = [this, param](int index)
            {
                param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(index)));
            };

        audioProcessor.apvts.addParameterListener("filterType", this);
    }
}


// ============================================================================
//  6. APVTS ATTACHMENTS WIRING
//     CABLEADO DE ATTACHMENTS DEL APVTS
// ============================================================================

// EN: Constructs every JUCE APVTS attachment. Each attachment binds
//     a UI control to its parameter, so changes propagate in BOTH
//     directions:
//       - User moves a slider/combo  -> APVTS updates -> Synth sees
//                                        the new value next block.
//       - Host automation or preset  -> APVTS updates -> attachment
//         load updates the parameter   moves the control on screen.
//
//     This function MUST run after every attached widget has been
//     created (most have been created by the time we get here, since
//     the constructor calls initialiseX* before initialiseAttachments).
//
//     Three controls in the GUI are NOT bound here, by design:
//       - presetSelector   -> not an APVTS parameter; manual handling.
//       - polyToggle        -> Choice parameter, no direct attachment;
//                              manual handling.
//       - filterTypeControl -> Choice parameter, no direct attachment;
//                              manual handling.
//
// ES: Construye todos los attachments del APVTS de JUCE. Cada
//     attachment vincula un control de UI con su parámetro, así los
//     cambios se propagan en AMBAS direcciones:
//       - Usuario mueve slider/combo  -> APVTS actualiza -> Synth ve
//                                         el nuevo valor en el siguiente
//                                         bloque.
//       - Automatización del host o   -> APVTS actualiza -> attachment
//         carga de preset actualiza     mueve el control en pantalla.
//         el parámetro
//
//     Esta función DEBE correr después de que cada widget vinculado
//     haya sido creado (la mayoría ya lo están al llegar aquí, porque
//     el constructor llama a initialiseX* antes de initialiseAttachments).
//
//     Tres controles de la GUI NO se vinculan aquí, a propósito:
//       - presetSelector   -> no es parámetro APVTS; manejo manual.
//       - polyToggle        -> parámetro Choice, sin attachment directo;
//                              manejo manual.
//       - filterTypeControl -> parámetro Choice, sin attachment directo;
//                              manejo manual.
void AndesJXAudioProcessorEditor::initialiseAttachments()
{
    // EN: Combo box attachments (oscillator waves, glide mode).
    // ES: Attachments de combo box (ondas de osciladores, modo de glide).
    oscWaveAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "osc1Wave", oscWaveSelector);
    osc2WaveAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "osc2Wave", osc2WaveSelector);
    glideModeAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "glideMode", glideModeSelector);

    // EN: Headline knobs (mix, resonance, cutoff, output).
    // ES: Knobs principales (mix, resonancia, cutoff, salida).
    mixAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "oscMix", mixSlider);
    resonanceAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterReso", resonanceSlider);
    cutoffAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterFreq", cutoffSlider);
    outputAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "outputLevel", outputSlider);

    // EN: Secondary value-labeled knobs.
    // ES: Knobs secundarios con label de valor.
    oscTuneAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "oscTune", oscTuneSlider);
    stereoWidthAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "stereoWidth", stereoWidthSlider);
    noiseAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise", noiseSlider);
    oscFineAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "oscFine", oscFineSlider);
    octaveAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "octave", octaveSlider);
    tuningAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "tuning", tuningSlider);
    glideRateAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "glideRate", glideRateSlider);
    glideBendAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "glideBend", glideBendSlider);
    vibratoAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "vibrato", vibratoSlider);
    filterVelocityAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterVelocity", filterVelocitySlider);
    filterEnvAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterEnv", filterEnvSlider);
    filterLFOAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterLFO", filterLFOSlider);
    filterKeycenterAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterKeycenter", filterKeycenterSlider);
    filterKeytrackAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterKeytrack", filterKeytrackSlider);
    lfoRateAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "lfoRate", lfoRateSlider);

    // EN: Amplitude envelope faders.
    // ES: Faders de la envolvente de amplitud.
    ampAttackAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "envAttack", ampAttackSlider);
    ampDecayAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "envDecay", ampDecaySlider);
    ampSustainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "envSustain", ampSustainSlider);
    ampReleaseAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "envRelease", ampReleaseSlider);

    // EN: Filter envelope faders.
    // ES: Faders de la envolvente de filtro.
    filterAttackAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterAttack", filterAttackSlider);
    filterDecayAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterDecay", filterDecaySlider);
    filterSustainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterSustain", filterSustainSlider);
    filterReleaseAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterRelease", filterReleaseSlider);
}

// ============================================================================
//  7. DESTRUCTOR
//     DESTRUCTOR
// ============================================================================

// EN: Tear-down ritual. Order matters here because every control with a
//     custom LookAndFeel holds a non-owning pointer to it; if we
//     destroy the LookAndFeel while the control is still pointing to
//     it, JUCE will crash on the next paint(). Therefore the rule is:
//
//        For each LookAndFeel:
//          1. Reset every control's lookAndFeel pointer to nullptr.
//          2. Reset the unique_ptr that owns the LookAndFeel.
//
//     The ChangeListener and ParameterListener subscriptions are also
//     undone here, mirroring the addChangeListener/addParameterListener
//     calls in the constructor and in initialisePolyToggle /
//     initialiseFilterTypeControl.
// ES: Ritual de destrucción. El orden importa porque cada control con
//     un LookAndFeel custom tiene un puntero no-propietario a él; si
//     destruimos el LookAndFeel mientras el control sigue apuntando a
//     él, JUCE crashea en el siguiente paint(). Por lo tanto la regla
//     es:
//
//        Para cada LookAndFeel:
//          1. Resetear el puntero lookAndFeel de cada control a nullptr.
//          2. Resetear el unique_ptr que posee el LookAndFeel.
//
//     Las suscripciones ChangeListener y ParameterListener también se
//     deshacen aquí, en espejo a las llamadas addChangeListener /
//     addParameterListener del constructor y de initialisePolyToggle /
//     initialiseFilterTypeControl.
AndesJXAudioProcessorEditor::~AndesJXAudioProcessorEditor()
{
    audioProcessor.removeChangeListener(this);


    // ------------------------------------------------------------------------
    //  Combo box LookAndFeel / LookAndFeel del combo box
    // ------------------------------------------------------------------------
    oscWaveSelector.setLookAndFeel(nullptr);
    osc2WaveSelector.setLookAndFeel(nullptr);
    presetSelector.setLookAndFeel(nullptr);
    glideModeSelector.setLookAndFeel(nullptr);
    comboBoxLookAndFeel.reset();


    // ------------------------------------------------------------------------
    //  Segmented button LookAndFeel + filter-type listener
    //  LookAndFeel del botón segmentado + listener de filter-type
    // ------------------------------------------------------------------------
    filterTypeControl.setLookAndFeelForButtons(nullptr);
    segmentedButtonLookAndFeel.reset();


    // ------------------------------------------------------------------------
    //  Manual parameter listeners / Listeners manuales de parámetros
    // ------------------------------------------------------------------------

    // EN: Mirrors the addParameterListener calls in initialisePolyToggle
    //     and initialiseFilterTypeControl.
    // ES: En espejo a las llamadas addParameterListener en
    //     initialisePolyToggle e initialiseFilterTypeControl.
    audioProcessor.apvts.removeParameterListener("polyMode", this);
    audioProcessor.apvts.removeParameterListener("filterType", this);


    // ------------------------------------------------------------------------
    //  Toggle LookAndFeel / LookAndFeel del toggle
    // ------------------------------------------------------------------------
    polyToggle.setLookAndFeel(nullptr);
    toggleLookAndFeel.reset();


    // ------------------------------------------------------------------------
    //  Headline knob LookAndFeel / LookAndFeel del knob principal
    // ------------------------------------------------------------------------
    mixSlider.setLookAndFeel(nullptr);
    resonanceSlider.setLookAndFeel(nullptr);
    cutoffSlider.setLookAndFeel(nullptr);
    outputSlider.setLookAndFeel(nullptr);
    knobPrincipalLookAndFeel.reset();


    // ------------------------------------------------------------------------
    //  Secondary knob LookAndFeel / LookAndFeel del knob secundario
    // ------------------------------------------------------------------------
    oscTuneSlider.setLookAndFeel(nullptr);
    stereoWidthSlider.setLookAndFeel(nullptr);
    noiseSlider.setLookAndFeel(nullptr);
    oscFineSlider.setLookAndFeel(nullptr);
    octaveSlider.setLookAndFeel(nullptr);
    tuningSlider.setLookAndFeel(nullptr);

    glideRateSlider.setLookAndFeel(nullptr);
    glideBendSlider.setLookAndFeel(nullptr);

    vibratoSlider.setLookAndFeel(nullptr);
    filterVelocitySlider.setLookAndFeel(nullptr);

    filterEnvSlider.setLookAndFeel(nullptr);
    filterLFOSlider.setLookAndFeel(nullptr);
    filterKeytrackSlider.setLookAndFeel(nullptr);
    filterKeycenterSlider.setLookAndFeel(nullptr);

    lfoRateSlider.setLookAndFeel(nullptr);

    secondaryKnobLookAndFeel.reset();


    // ------------------------------------------------------------------------
    //  Fader LookAndFeel / LookAndFeel del fader
    // ------------------------------------------------------------------------
    ampAttackSlider.setLookAndFeel(nullptr);
    ampDecaySlider.setLookAndFeel(nullptr);
    ampSustainSlider.setLookAndFeel(nullptr);
    ampReleaseSlider.setLookAndFeel(nullptr);

    filterAttackSlider.setLookAndFeel(nullptr);
    filterDecaySlider.setLookAndFeel(nullptr);
    filterSustainSlider.setLookAndFeel(nullptr);
    filterReleaseSlider.setLookAndFeel(nullptr);

    faderLookAndFeel.reset();
}


// ============================================================================
//  8. PAINT (BACKGROUND IMAGE + LABELS)
//     PAINT (IMAGEN DE FONDO + LABELS)
// ============================================================================

// EN: Two-step paint:
//
//       1. Background image stretched to fit the editor's bounds.
//          The default-window backgroundColour fills behind the image
//          first as a safety net (visible only if the PNG fails to
//          load and the fallback size kicks in).
//
//       2. Section titles and per-control captions drawn on top of
//          the background. Coordinates are hardcoded to align with
//          the artwork's visual landmarks; if the background PNG ever
//          changes its layout, these numbers must be re-tuned.
//          The captions are grouped here by GUI panel (OSCILLATORS,
//          PERFORMANCE, FILTER, MODULATION, MASTER, ENVELOPES) so
//          that finding "where is the COARSE label?" only requires
//          reading the right panel block.
//
// ES: Paint en dos pasos:
//
//       1. Imagen de fondo estirada para llenar los límites del
//          editor. El backgroundColour por defecto del window se
//          pinta detrás de la imagen primero como red de seguridad
//          (solo se ve si el PNG no se carga y entra el fallback de
//          tamaño).
//
//       2. Títulos de sección y captions por control dibujados encima
//          del fondo. Las coordenadas están hardcodeadas para alinear
//          con los puntos de referencia visuales del arte; si el PNG
//          de fondo llegara a cambiar de layout, estos números deben
//          re-ajustarse.
//          Los captions están agrupados aquí por panel de GUI
//          (OSCILLATORS, PERFORMANCE, FILTER, MODULATION, MASTER,
//          ENVELOPES) para que encontrar "¿dónde está el label
//          COARSE?" solo requiera leer el bloque de panel correcto.
void AndesJXAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    if (backgroundAndesJX.isValid())
    {
        // EN: Use high-quality resampling when scaling the background
        //     image down from its 4x source to the editor's logical
        //     size. On HiDPI displays the system multiplies that logical
        //     size again, and JUCE uses the source's full resolution
        //     instead of upscaling pre-downscaled pixels.
        // ES: Usar resampling de alta calidad al escalar la imagen de
        //     fondo desde su fuente 4x al tamaño lógico del editor. En
        //     pantallas HiDPI el sistema multiplica ese tamaño lógico de
        //     nuevo, y JUCE usa la resolución completa de la fuente en
        //     lugar de upscalar píxeles ya pre-reducidos.
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

        g.drawImageWithin(
            backgroundAndesJX,
            0, 0,
            getWidth(), getHeight(),
            juce::RectanglePlacement::stretchToFit);
    }


    // ------------------------------------------------------------------------
    //  Section titles / Títulos de sección
    // ------------------------------------------------------------------------
    drawSectionTitle(g, { 20,  15, 140, 15 }, "OSCILLATORS");
    drawSectionTitle(g, { 300,  15, 120, 15 }, "PERFORMANCE");
    drawSectionTitle(g, { 20, 165, 100, 15 }, "FILTER");
    drawSectionTitle(g, { 300, 165, 120, 15 }, "ENVELOPES");
    drawSectionTitle(g, { 20, 315, 120, 15 }, "MODULATION");
    drawSectionTitle(g, { 370, 315, 100, 15 }, "MASTER");


    // ------------------------------------------------------------------------
    //  Oscillators panel / Panel de osciladores
    // ------------------------------------------------------------------------

    // EN: Osc 1 row.
    // ES: Fila de Osc 1.
    drawControlLabel(g, { 20, 62, 50, 12 }, "WAVE");
    drawControlLabel(g, { 158, 62, 40, 12 }, "COARSE");
    drawControlLabel(g, { 198, 62, 40, 12 }, "WIDTH");
    drawControlLabel(g, { 238, 62, 40, 12 }, "NOISE");

    // EN: Osc 2 row.
    // ES: Fila de Osc 2.
    drawControlLabel(g, { 20, 122, 50, 12 }, "WAVE");
    drawControlLabel(g, { 158, 122, 40, 12 }, "FINE");
    drawControlLabel(g, { 198, 122, 40, 12 }, "OCTAVE");
    drawControlLabel(g, { 238, 122, 40, 12 }, "TUNE");

    // EN: Mix knob caption between the two rows.
    // ES: Caption del knob de mix entre las dos filas.
    drawControlLabel(g, { 80, 105, 64, 12 }, "MIX");


    // ------------------------------------------------------------------------
    //  Performance panel / Panel de performance
    // ------------------------------------------------------------------------
    drawControlLabel(g, { 320,  68, 50, 12 }, "GLIDE");
    drawControlLabel(g, { 408,  68, 54, 12 }, "VOICE");

    drawControlLabel(g, { 302, 122, 48, 12 }, "RATE");
    drawControlLabel(g, { 342, 122, 48, 12 }, "BEND");

    drawControlLabel(g, { 396, 122, 44, 12 }, "PWM/VIB");
    drawControlLabel(g, { 434, 122, 50, 12 }, "VEL FLTR");


    // ------------------------------------------------------------------------
    //  Filter panel / Panel de filtro
    // ------------------------------------------------------------------------
    drawControlLabel(g, { 47, 212, 76, 12 }, "TYPE");

    drawControlLabel(g, { 25, 280, 64, 12 }, "CUTOFF");
    drawControlLabel(g, { 90, 280, 64, 12 }, "RESO");

    drawControlLabel(g, { 194, 272, 48, 12 }, "ENV AMT");
    drawControlLabel(g, { 174, 212, 48, 12 }, "KEY TRCK");
    drawControlLabel(g, { 218, 212, 52, 12 }, "KEY CNTR");


    // ------------------------------------------------------------------------
    //  Modulation panel / Panel de modulación
    // ------------------------------------------------------------------------
    drawControlLabel(g, { 24, 382, 60, 12 }, "LFO RATE");
    drawControlLabel(g, { 74, 382, 60, 12 }, "VCF MOD");


    // ------------------------------------------------------------------------
    //  Master panel / Panel master
    // ------------------------------------------------------------------------
    drawControlLabel(g, { 395, 400,  60, 12 }, "OUTPUT");
    drawControlLabel(g, { 205, 400, 100, 12 }, "PRESET");


    // ------------------------------------------------------------------------
    //  Envelopes panel / Panel de envolventes
    // ------------------------------------------------------------------------

    // EN: Amplitude ADSR. The "A D S R" tiny labels live below their
    //     respective faders.
    // ES: ADSR de amplitud. Los tiny labels "A D S R" están debajo de
    //     sus faders respectivos.
    drawControlLabel(g, { 305, 186, 70, 12 }, "AMP");
    drawTinyControlLabel(g, { 300, 272, 16, 10 }, "A");
    drawTinyControlLabel(g, { 320, 272, 16, 10 }, "D");
    drawTinyControlLabel(g, { 340, 272, 16, 10 }, "S");
    drawTinyControlLabel(g, { 360, 272, 16, 10 }, "R");

    // EN: Filter ADSR.
    // ES: ADSR de filtro.
    drawControlLabel(g, { 403, 186, 70, 12 }, "FILTER");
    drawTinyControlLabel(g, { 400, 272, 16, 10 }, "A");
    drawTinyControlLabel(g, { 420, 272, 16, 10 }, "D");
    drawTinyControlLabel(g, { 440, 272, 16, 10 }, "S");
    drawTinyControlLabel(g, { 460, 272, 16, 10 }, "R");
}

// ============================================================================
//  9. RESIZED, PARAMETER CHANGES AND PRESET CHANGES
//     RESIZED, CAMBIOS DE PARÁMETROS Y CAMBIOS DE PRESETS
// ============================================================================

// EN: Lays out every control on the editor. Coordinates are hardcoded
//     in pixel space and align with the artwork drawn in paint(): each
//     slider/combo/toggle sits over the visual "well" provided by the
//     background PNG.
//
//     Layout conventions:
//       - The editor uses a fixed window size matching the background
//         image (set in initialiseBackground), so this function is
//         called once at construction and again only if the host
//         resizes the plugin window (which AndesJX does not do).
//       - Each value-labeled knob is paired with its label drawn just
//         above the knob (same X with a small horizontal offset). The
//         label rectangle is intentionally wider than the knob so a
//         "+100 %" or "C#7" string fits without truncation.
//       - The two ADSR rows place their four faders in a 20-pixel
//         pitch (300, 320, 340, 360 for amp; 400, 420, 440, 460 for
//         filter). The same pitch is used by the tiny A/D/S/R captions
//         in paint().
//
//     If the background PNG is ever redesigned, every coordinate here
//     must be re-tuned to match the new visual layout.
//
// ES: Dispone cada control en el editor. Las coordenadas están
//     hardcodeadas en espacio de píxeles y se alinean con el arte
//     dibujado en paint(): cada slider/combo/toggle se posa sobre el
//     "pozo" visual que provee el PNG de fondo.
//
//     Convenciones de layout:
//       - El editor usa un tamaño de ventana fijo que coincide con la
//         imagen de fondo (se asigna en initialiseBackground), así que
//         esta función se llama una vez en la construcción y de nuevo
//         solo si el host redimensiona la ventana del plugin (cosa que
//         AndesJX no hace).
//       - Cada knob con label de valor se empareja con su label
//         dibujado justo encima del knob (mismo X con un pequeño
//         desplazamiento horizontal). El rectángulo del label es
//         intencionadamente más ancho que el knob para que strings
//         como "+100 %" o "C#7" entren sin truncarse.
//       - Las dos filas de ADSR colocan sus cuatro faders con paso de
//         20 píxeles (300, 320, 340, 360 para amp; 400, 420, 440, 460
//         para filter). El mismo paso lo usan los tiny captions
//         A/D/S/R en paint().
//
//     Si el PNG de fondo se rediseña, cada coordenada de aquí debe
//     re-ajustarse para que coincida con el nuevo layout visual.
void AndesJXAudioProcessorEditor::resized()
{
    // ------------------------------------------------------------------------
    //  Combo boxes, toggle, segmented control / Combos, toggle, segmentado
    // ------------------------------------------------------------------------
    oscWaveSelector.setBounds(20, 40, 50, 16);
    osc2WaveSelector.setBounds(20, 100, 50, 16);
    presetSelector.setBounds(205, 380, 100, 16);
    polyToggle.setBounds(410, 45, 50, 16);
    glideModeSelector.setBounds(320, 45, 50, 16);
    filterTypeControl.setBounds(45, 190, 80, 16);


    // ------------------------------------------------------------------------
    //  Headline knobs / Knobs principales
    // ------------------------------------------------------------------------
    mixSlider.setBounds(80, 50, 64, 64);
    resonanceSlider.setBounds(90, 225, 64, 64);
    cutoffSlider.setBounds(25, 225, 64, 64);
    outputSlider.setBounds(385, 325, 80, 80);


    // ------------------------------------------------------------------------
    //  Oscillator row (Osc 1 + Osc 2 secondary knobs)
    //  Fila de osciladores (knobs secundarios de Osc 1 + Osc 2)
    // ------------------------------------------------------------------------

    // EN: Osc 1 row. The label sits 8 px above the knob so the value
    //     reads cleanly above the rotary indicator.
    // ES: Fila de Osc 1. El label se ubica 8 px arriba del knob para
    //     que el valor se lea limpiamente sobre el indicador rotatorio.
    oscTuneValueLabel.setBounds(160, 22, 34, 10);
    oscTuneSlider.setBounds(160, 30, 36, 36);
    stereoWidthValueLabel.setBounds(196, 22, 48, 10);
    stereoWidthSlider.setBounds(200, 30, 36, 36);
    noiseValueLabel.setBounds(236, 22, 48, 10);
    noiseSlider.setBounds(240, 30, 36, 36);

    // EN: Osc 2 row, same X positions as Osc 1 shifted +60 pixels in Y.
    // ES: Fila de Osc 2, mismas posiciones X que Osc 1 desplazadas
    //     +60 píxeles en Y.
    oscFineValueLabel.setBounds(160, 82, 34, 10);
    oscFineSlider.setBounds(160, 90, 36, 36);
    octaveValueLabel.setBounds(198, 82, 34, 10);
    octaveSlider.setBounds(200, 90, 36, 36);
    tuningValueLabel.setBounds(240, 82, 34, 10);
    tuningSlider.setBounds(240, 90, 36, 36);


    // ------------------------------------------------------------------------
    //  Performance panel / Panel de performance
    // ------------------------------------------------------------------------

    // EN: Glide rate / glide bend pair.
    // ES: Par glide rate / glide bend.
    glideRateValueLabel.setBounds(302, 82, 48, 10);
    glideRateSlider.setBounds(308, 90, 36, 36);
    glideBendValueLabel.setBounds(342, 82, 48, 10);
    glideBendSlider.setBounds(348, 90, 36, 36);

    // EN: Vibrato (PWM/VIB) / filter velocity pair.
    // ES: Par vibrato (PWM/VIB) / filter velocity.
    vibratoValueLabel.setBounds(392, 82, 52, 10);
    vibratoSlider.setBounds(400, 90, 36, 36);
    filterVelocityValueLabel.setBounds(432, 82, 52, 10);
    filterVelocitySlider.setBounds(440, 90, 36, 36);


    // ------------------------------------------------------------------------
    //  Filter panel (env amount, key tracking, key center)
    //  Panel de filtro (env amount, key tracking, key center)
    // ------------------------------------------------------------------------
    filterEnvValueLabel.setBounds(194, 232, 48, 10);
    filterEnvSlider.setBounds(200, 240, 36, 36);
    filterKeytrackValueLabel.setBounds(174, 172, 52, 10);
    filterKeytrackSlider.setBounds(180, 180, 36, 36);
    filterKeycenterValueLabel.setBounds(219, 172, 48, 10);
    filterKeycenterSlider.setBounds(225, 180, 36, 36);


    // ------------------------------------------------------------------------
    //  Modulation panel (LFO rate + filter LFO)
    //  Panel de modulación (LFO rate + filter LFO)
    // ------------------------------------------------------------------------
    lfoRateValueLabel.setBounds(25, 340, 52, 10);
    lfoRateSlider.setBounds(35, 350, 36, 36);
    filterLFOValueLabel.setBounds(80, 342, 48, 10);
    filterLFOSlider.setBounds(85, 350, 36, 36);


    // ------------------------------------------------------------------------
    //  Envelope faders / Faders de envolventes
    // ------------------------------------------------------------------------

    // EN: Amplitude ADSR. 20-px pitch, fader column at X = 300, 320,
    //     340, 360.
    // ES: ADSR de amplitud. Paso de 20 px, columna de faders en
    //     X = 300, 320, 340, 360.
    ampAttackSlider.setBounds(300, 190, 16, 90);
    ampDecaySlider.setBounds(320, 190, 16, 90);
    ampSustainSlider.setBounds(340, 190, 16, 90);
    ampReleaseSlider.setBounds(360, 190, 16, 90);

    // EN: Filter ADSR. Same shape as amp, shifted +100 px in X.
    // ES: ADSR de filtro. Misma forma que amp, desplazada +100 px en X.
    filterAttackSlider.setBounds(400, 190, 16, 90);
    filterDecaySlider.setBounds(420, 190, 16, 90);
    filterSustainSlider.setBounds(440, 190, 16, 90);
    filterReleaseSlider.setBounds(460, 190, 16, 90);
}


// ----------------------------------------------------------------------------
//  parameterChanged / parameterChanged
// ----------------------------------------------------------------------------

// EN: APVTS parameter listener. Fires whenever a registered parameter
//     changes, regardless of source (host automation, MIDI CC, preset
//     load, programmatic write). Used to keep the polyToggle and the
//     filterTypeControl in sync with their Choice parameters, since
//     these two controls do NOT have JUCE attachments (see
//     initialisePolyToggle / initialiseFilterTypeControl for the
//     dual-binding rationale).
//
//     Important: this listener is invoked from the AUDIO THREAD when
//     the change comes from automation. Touching JUCE GUI components
//     from the audio thread is illegal and will eventually crash, so
//     the body trampolines through MessageManager::callAsync to do
//     the actual UI work on the message thread.
//
// ES: Listener de parámetros del APVTS. Se dispara cada vez que cambia
//     un parámetro registrado, sin importar la fuente (automatización
//     del host, CC MIDI, carga de preset, escritura programática). Se
//     usa para mantener polyToggle y filterTypeControl sincronizados
//     con sus parámetros Choice, ya que estos dos controles NO tienen
//     attachments de JUCE (ver initialisePolyToggle e
//     initialiseFilterTypeControl para la razón del dual-binding).
//
//     Importante: este listener se invoca desde el HILO DE AUDIO
//     cuando el cambio viene de automatización. Tocar componentes GUI
//     de JUCE desde el hilo de audio es ilegal y eventualmente
//     crashea, así que el cuerpo hace trampolín por
//     MessageManager::callAsync para que el trabajo de UI corra en el
//     hilo de mensajes.
void AndesJXAudioProcessorEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "polyMode")
    {
        // EN: newValue arrives in [0, 1]: convert to a safe choice
        //     index using the parameter's own range mapping.
        // ES: newValue llega en [0, 1]: convertirlo a un índice de
        //     choice seguro usando el mapeo de rango del propio
        //     parámetro.
        if (auto* p = audioProcessor.apvts.getParameter(parameterID))
        {
            const int  index = juce::roundToInt(p->convertFrom0to1(newValue));
            const bool isPoly = (index == 1);

            juce::MessageManager::callAsync([this, isPoly]()
                {
                    polyToggle.setToggleState(isPoly, juce::dontSendNotification);
                    polyToggle.setButtonText(isPoly ? "Poly" : "Mono");
                });
        }
    }
    else if (parameterID == "filterType")
    {
        if (auto* p = audioProcessor.apvts.getParameter(parameterID))
        {
            const int index = juce::roundToInt(p->convertFrom0to1(newValue));

            juce::MessageManager::callAsync([this, index]()
                {
                    filterTypeControl.setSelectedIndex(index, juce::dontSendNotification);
                });
        }
    }
}


// ----------------------------------------------------------------------------
//  changeListenerCallback / changeListenerCallback
// ----------------------------------------------------------------------------

// EN: ChangeListener callback. The processor broadcasts a change
//     message every time the "isCustomPreset" flag flips, and this
//     handler updates the preset selector accordingly:
//       - When the state stops matching the active factory preset
//         (the user moved a knob), the selector switches to "Custom"
//         (id 1000).
//       - When a fresh factory preset is loaded, the selector goes
//         back to its index.
//     juce::dontSendNotification suppresses the combo's onChange so
//     this programmatic update does not bounce back into a preset
//     load.
// ES: Callback de ChangeListener. El processor broadcastea un mensaje
//     de cambio cada vez que cambia la bandera "isCustomPreset", y este
//     handler actualiza el selector de presets en consecuencia:
//       - Cuando el estado deja de coincidir con el preset de fábrica
//         activo (el usuario movió un knob), el selector pasa a
//         "Custom" (id 1000).
//       - Cuando se carga un preset de fábrica nuevo, el selector
//         vuelve a su índice.
//     juce::dontSendNotification suprime el onChange del combo para
//     que esta actualización programática no rebote en una carga de
//     preset.
void AndesJXAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioProcessor)
    {
        if (audioProcessor.isCustomPresetActive())
            presetSelector.setSelectedId(1000, juce::dontSendNotification);
        else
            presetSelector.setSelectedId(audioProcessor.getCurrentProgram() + 1,
                juce::dontSendNotification);
    }
}
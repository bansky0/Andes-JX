/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

/*
    Module: AndesJXAudioProcessorEditor (header)
    Purpose:
        EN: Plugin GUI. Hosts every visible control of AndesJX (knobs,
            faders, combo boxes, segmented controls, toggles), wires them
            to the APVTS parameters via JUCE attachments, and renders the
            background artwork.
        ES: GUI del plugin. Aloja cada control visible de AndesJX (knobs,
            faders, combo boxes, controles segmentados, toggles), los
            conecta a los parámetros APVTS mediante attachments de JUCE
            y renderiza el arte de fondo.

    Main responsibilities:
        EN:
          - Instantiate every UI widget and lay it out in resized()
          - Bind each widget to its APVTS parameter through attachments
          - Apply custom LookAndFeels to give AndesJX its visual identity
          - Listen for parameter changes (label updates) and for
            "preset became custom" notifications from PluginProcessor
        ES:
          - Instanciar cada widget de UI y disponerlos en resized()
          - Vincular cada widget a su parámetro APVTS mediante attachments
          - Aplicar LookAndFeels custom para dar a AndesJX su identidad
            visual
          - Escuchar cambios de parámetros (actualizar labels) y
            notificaciones de "el preset se volvió custom" desde
            PluginProcessor

    Architectural role:
        EN: Created by AndesJXAudioProcessor::createEditor(). Holds a
            reference to the processor to read APVTS values and to
            register/unregister listeners. Does not own any DSP state;
            all the audio engine lives in the processor and Synth.
        ES: AndesJXAudioProcessor::createEditor() lo crea. Mantiene una
            referencia al processor para leer valores del APVTS y para
            registrar/desregistrar listeners. No posee estado DSP; el
            motor de audio vive en el processor y Synth.

    Notes:
        EN:
          - Triple inheritance: AudioProcessorEditor (the JUCE GUI
            contract), APVTS::Listener (to refresh labels when a
            parameter changes), ChangeListener (to react when the
            processor flags a custom preset).
          - The LookAndFeels are owned by unique_ptr because they must
            outlive the controls that use them; the editor owns them so
            their lifetime matches the GUI's.
          - Every value-displaying control follows a consistent
            "format / update / initialise" pattern:
                formatX(value)        -> juce::String
                updateXValueLabel()   -> reads slider, calls formatX,
                                         pushes into label
                initialiseXControl()  -> wires slider + label into the
                                         editor via the helper
                                         initialiseValueLabeledControl()
            The helper centralizes the parts that repeat across all 15
            controls (slider style, label color, font, intercepts,
            onValueChange callback). The per-control formatX functions
            stay individual because each one has its own formatting
            rules (units, special states like "OFF" / "PWM 25.0", note
            names, signed values, etc.).
          - parameterChanged() routes the right updateX call by
            parameter ID, so labels stay in sync with both GUI and
            host changes.
        ES:
          - Herencia triple: AudioProcessorEditor (contrato GUI de
            JUCE), APVTS::Listener (refrescar labels al cambiar
            parámetros), ChangeListener (reaccionar cuando el processor
            marca un preset como custom).
          - Los LookAndFeels son propiedad de unique_ptr porque deben
            sobrevivir a los controles que los usan; el editor los
            posee para que su ciclo de vida coincida con el de la GUI.
          - Cada control que muestra valor sigue un patrón consistente
            "format / update / initialise":
                formatX(value)        -> juce::String
                updateXValueLabel()   -> lee slider, llama a formatX,
                                         envía al label
                initialiseXControl()  -> cablea slider + label dentro
                                         del editor mediante el helper
                                         initialiseValueLabeledControl()
            El helper centraliza las partes que se repiten en los 15
            controles (estilo del slider, color del label, font,
            intercepts, callback onValueChange). Las funciones formatX
            por control se mantienen individuales porque cada una
            tiene sus propias reglas de formato (unidades, estados
            especiales como "OFF" / "PWM 25.0", nombres de nota,
            valores con signo, etc.).
          - parameterChanged() enruta la llamada updateX correcta
            según el ID del parámetro, así los labels quedan
            sincronizados con cambios del GUI y del host.
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LookAndFeel/ComboBoxLookAndFeel.h"
#include "LookAndFeel/ToggleLookAndFeel.h"
#include "SegmentedControl.h"
#include "LookAndFeel/SegmentedButtonLookAndFeel.h"
#include "LookAndFeel/KnobPrincipalLookAndFeel.h"
#include "LookAndFeel/SecondaryKnobLookAndFeel.h"
#include "LookAndFeel/FaderLookAndFeel.h"


//==============================================================================

class AndesJXAudioProcessorEditor : public juce::AudioProcessorEditor,
    private juce::AudioProcessorValueTreeState::Listener,
    private juce::ChangeListener
{
public:
    AndesJXAudioProcessorEditor(AndesJXAudioProcessor&);
    ~AndesJXAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;


private:
    // ========================================================================
    //  PROCESSOR REFERENCE / REFERENCIA AL PROCESSOR
    // ========================================================================

    // EN: Non-owning reference to the audio processor. Used to read
    //     APVTS values, to register/unregister listeners, and to query
    //     preset state (currentProgram, isCustomPreset).
    // ES: Referencia no-propietaria al processor. Se usa para leer
    //     valores del APVTS, registrar/desregistrar listeners y
    //     consultar el estado de presets (currentProgram, isCustomPreset).
    AndesJXAudioProcessor& audioProcessor;


    // ========================================================================
    //  BACKGROUND ARTWORK / ARTE DE FONDO
    // ========================================================================

    // EN: Pre-rendered background image for the GUI. Loaded once in
    //     initialiseBackground() and drawn in paint() under all
    //     controls.
    // ES: Imagen de fondo pre-renderizada de la GUI. Se carga una vez
    //     en initialiseBackground() y se dibuja en paint() debajo de
    //     todos los controles.
    juce::Image backgroundAndesJX;


    // ========================================================================
    //  COMBO BOXES / COMBO BOXES
    // ========================================================================

    juce::ComboBox oscWaveSelector;
    juce::ComboBox osc2WaveSelector;
    juce::ComboBox presetSelector;
    juce::ComboBox glideModeSelector;

    // EN: APVTS attachments that synchronize each combo with its
    //     parameter. Wrapped in unique_ptr so they can be created
    //     after the components themselves (JUCE attachments expect the
    //     control and the parameter to exist before binding).
    // ES: Attachments del APVTS que sincronizan cada combo con su
    //     parámetro. Envueltos en unique_ptr para poder crearse
    //     después de los componentes (los attachments de JUCE esperan
    //     que el control y el parámetro existan antes del vínculo).
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<ComboBoxAttachment> oscWaveAttachment;
    std::unique_ptr<ComboBoxAttachment> osc2WaveAttachment;
    std::unique_ptr<ComboBoxAttachment> glideModeAttachment;


    // ========================================================================
    //  PRIMARY CONTROLS (knobs and main faders)
    //  CONTROLES PRINCIPALES (knobs y faders principales)
    // ========================================================================

    // EN: The four "headline" controls of the synth: oscillator mix,
    //     filter cutoff, filter resonance and master output level.
    //     They use the larger KnobPrincipal / Fader look-and-feels.
    // ES: Los cuatro controles "principales" del synth: mezcla de
    //     osciladores, cutoff de filtro, resonancia de filtro y nivel
    //     de salida master. Usan los look-and-feels más grandes
    //     KnobPrincipal / Fader.
    juce::Slider mixSlider;
    juce::Slider resonanceSlider;
    juce::Slider cutoffSlider;
    juce::Slider outputSlider;


    // ========================================================================
    //  SECONDARY CONTROLS (knobs with value labels)
    //  CONTROLES SECUNDARIOS (knobs con labels de valor)
    // ========================================================================

    // EN: Each control follows the format / update / initialise pattern
    //     described in the file Notes. The slider is for interaction;
    //     the label shows the formatted current value. Their format
    //     functions live in the bottom section of this header.
    // ES: Cada control sigue el patrón format / update / initialise
    //     descrito en las Notes del archivo. El slider es para
    //     interacción; el label muestra el valor formateado actual. Sus
    //     funciones format viven en la sección inferior de este header.
    juce::Slider oscTuneSlider;          juce::Label oscTuneValueLabel;
    juce::Slider stereoWidthSlider;      juce::Label stereoWidthValueLabel;
    juce::Slider noiseSlider;            juce::Label noiseValueLabel;
    juce::Slider oscFineSlider;          juce::Label oscFineValueLabel;
    juce::Slider octaveSlider;           juce::Label octaveValueLabel;
    juce::Slider tuningSlider;           juce::Label tuningValueLabel;
    juce::Slider glideBendSlider;        juce::Label glideBendValueLabel;
    juce::Slider glideRateSlider;        juce::Label glideRateValueLabel;
    juce::Slider vibratoSlider;          juce::Label vibratoValueLabel;
    juce::Slider filterVelocitySlider;   juce::Label filterVelocityValueLabel;
    juce::Slider filterEnvSlider;        juce::Label filterEnvValueLabel;
    juce::Slider filterLFOSlider;        juce::Label filterLFOValueLabel;
    juce::Slider filterKeycenterSlider;  juce::Label filterKeycenterValueLabel;
    juce::Slider filterKeytrackSlider;   juce::Label filterKeytrackValueLabel;
    juce::Slider lfoRateSlider;          juce::Label lfoRateValueLabel;


    // ========================================================================
    //  ENVELOPE CONTROLS / CONTROLES DE ENVOLVENTE
    // ========================================================================

    // EN: Two ADSR envelopes side by side: amplitude (amp*) and filter
    //     (filter*). Each one is laid out as four vertical faders.
    // ES: Dos envolventes ADSR lado a lado: amplitud (amp*) y filtro
    //     (filter*). Cada una se dispone como cuatro faders verticales.
    juce::Slider ampAttackSlider;
    juce::Slider ampDecaySlider;
    juce::Slider ampSustainSlider;
    juce::Slider ampReleaseSlider;

    juce::Slider filterAttackSlider;
    juce::Slider filterDecaySlider;
    juce::Slider filterSustainSlider;
    juce::Slider filterReleaseSlider;


    // ========================================================================
    //  SLIDER ATTACHMENTS / ATTACHMENTS DE SLIDERS
    // ========================================================================

    // EN: Bind each slider to its APVTS parameter so that GUI changes
    //     propagate to Synth and host automation can drive the GUI.
    // ES: Vinculan cada slider con su parámetro APVTS para que los
    //     cambios de GUI se propaguen a Synth y la automatización del
    //     host pueda mover la GUI.
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<SliderAttachment> resonanceAttachment;
    std::unique_ptr<SliderAttachment> cutoffAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;

    std::unique_ptr<SliderAttachment> oscTuneAttachment;
    std::unique_ptr<SliderAttachment> stereoWidthAttachment;
    std::unique_ptr<SliderAttachment> noiseAttachment;
    std::unique_ptr<SliderAttachment> oscFineAttachment;
    std::unique_ptr<SliderAttachment> octaveAttachment;
    std::unique_ptr<SliderAttachment> tuningAttachment;
    std::unique_ptr<SliderAttachment> glideRateAttachment;
    std::unique_ptr<SliderAttachment> glideBendAttachment;
    std::unique_ptr<SliderAttachment> vibratoAttachment;
    std::unique_ptr<SliderAttachment> filterVelocityAttachment;
    std::unique_ptr<SliderAttachment> filterEnvAttachment;
    std::unique_ptr<SliderAttachment> filterLFOAttachment;
    std::unique_ptr<SliderAttachment> filterKeycenterAttachment;
    std::unique_ptr<SliderAttachment> filterKeytrackAttachment;
    std::unique_ptr<SliderAttachment> lfoRateAttachment;

    std::unique_ptr<SliderAttachment> ampAttackAttachment;
    std::unique_ptr<SliderAttachment> ampDecayAttachment;
    std::unique_ptr<SliderAttachment> ampSustainAttachment;
    std::unique_ptr<SliderAttachment> ampReleaseAttachment;

    std::unique_ptr<SliderAttachment> filterAttackAttachment;
    std::unique_ptr<SliderAttachment> filterDecayAttachment;
    std::unique_ptr<SliderAttachment> filterSustainAttachment;
    std::unique_ptr<SliderAttachment> filterReleaseAttachment;


    // ========================================================================
    //  CUSTOM LOOK-AND-FEELS / LOOK-AND-FEELS CUSTOM
    // ========================================================================

    // EN: Custom rendering classes that override how knobs, faders,
    //     toggles, combos and segmented buttons are drawn. Owned by
    //     unique_ptr because they must outlive the controls that use
    //     them: a control that holds a dangling LookAndFeel pointer
    //     would crash on next paint.
    // ES: Clases de renderizado custom que redefinen cómo se dibujan
    //     knobs, faders, toggles, combos y botones segmentados.
    //     Propiedad de unique_ptr porque deben sobrevivir a los
    //     controles que los usan: un control con un puntero a
    //     LookAndFeel colgante crashearía en el siguiente paint.
    std::unique_ptr<ComboBoxLookAndFeel>        comboBoxLookAndFeel;
    std::unique_ptr<KnobPrincipalLookAndFeel>   knobPrincipalLookAndFeel;
    std::unique_ptr<SecondaryKnobLookAndFeel>   secondaryKnobLookAndFeel;
    std::unique_ptr<FaderLookAndFeel>           faderLookAndFeel;
    std::unique_ptr<ToggleLookAndFeel>          toggleLookAndFeel;
    std::unique_ptr<SegmentedButtonLookAndFeel> segmentedButtonLookAndFeel;


    // ========================================================================
    //  SPECIALIZED CONTROLS / CONTROLES ESPECIALIZADOS
    // ========================================================================

    // EN: Polyphony toggle (Mono / Poly). Bound to the polyMode APVTS
    //     parameter through a custom on-click handler instead of an
    //     attachment, because the parameter is a Choice, not a Bool.
    // ES: Toggle de polifonía (Mono / Poly). Se vincula al parámetro
    //     APVTS polyMode mediante un handler on-click custom en lugar
    //     de un attachment, porque el parámetro es Choice, no Bool.
    juce::ToggleButton polyToggle;

    // EN: Custom segmented control (see SegmentedControl.h) that
    //     selects between SVF and Moog filter algorithms. Wired to the
    //     filterType APVTS parameter through its onChange callback.
    // ES: Control segmentado custom (ver SegmentedControl.h) que elige
    //     entre los algoritmos SVF y Moog. Conectado al parámetro
    //     APVTS filterType mediante su callback onChange.
    SegmentedControl filterTypeControl;


    // ========================================================================
    //  LISTENER CALLBACKS / CALLBACKS DE LISTENER
    // ========================================================================

    // EN: APVTS parameter listener. Fired whenever any parameter
    //     changes (from GUI, host automation, MIDI CC or preset load).
    //     Used to refresh value labels.
    // ES: Listener de parámetros del APVTS. Se dispara al cambiar
    //     cualquier parámetro (desde GUI, automatización del host, CC
    //     MIDI o carga de preset). Se usa para refrescar los labels
    //     de valor.
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    // EN: ChangeListener callback. Fired by the processor when the
    //     "isCustomPreset" flag flips, so the GUI can show the user
    //     that the active preset has been edited.
    // ES: Callback de ChangeListener. El processor lo dispara cuando
    //     cambia la bandera "isCustomPreset" para que la GUI muestre
    //     al usuario que el preset activo fue editado.
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;


    // ========================================================================
    //  INITIALIZATION HELPERS / HELPERS DE INICIALIZACIÓN
    // ========================================================================

    // EN: The constructor delegates the GUI build to these helpers, one
    //     per logical group of controls. Splitting the work this way
    //     keeps the constructor short and makes it easy to disable a
    //     section temporarily during UI iteration.
    // ES: El constructor delega la construcción de la GUI a estos
    //     helpers, uno por cada grupo lógico de controles. Dividir el
    //     trabajo así mantiene el constructor corto y facilita
    //     deshabilitar una sección temporalmente al iterar la UI.

    void initialiseBackground();
    void initialiseLookAndFeels();
    void initialiseAttachments();

    // EN: Generic per-widget setup helpers. setupCombo / setupKnob /
    //     setupSecondaryKnob / setupFader apply the right look-and-feel
    //     and add the component to the editor.
    // ES: Helpers genéricos por widget. setupCombo / setupKnob /
    //     setupSecondaryKnob / setupFader aplican el look-and-feel
    //     correcto y añaden el componente al editor.
    void setupFader(juce::Slider& slider);
    void setupCombo(juce::ComboBox& combo);
    void setupKnob(juce::Slider& slider);
    void setupSecondaryKnob(juce::Slider& slider);

    // EN: Centralizes the boilerplate shared by every value-labeled
    //     secondary knob: setup the knob, set its componentID, format
    //     the label (color, font, justification, ignore mouse events),
    //     run the initial update, and wire onValueChange to refresh
    //     the label as the user drags. Each per-control initialiseX
    //     function below collapses to a single call to this helper,
    //     replacing roughly 13 lines of identical code each.
    // ES: Centraliza el boilerplate compartido por cada knob secundario
    //     con label de valor: configura el knob, asigna su componentID,
    //     formatea el label (color, font, justificación, ignorar mouse),
    //     ejecuta el update inicial y cablea onValueChange para
    //     refrescar el label al arrastrar. Cada función initialiseX por
    //     control de abajo colapsa a una sola llamada a este helper,
    //     reemplazando unas 13 líneas idénticas en cada caso.
    void initialiseValueLabeledControl(juce::Slider& slider,
        juce::Label& label,
        const juce::String& componentID,
        std::function<void()> updateFn);

    // EN: Specific control initializers. Each one wires a single
    //     control or a tightly related group (e.g. one ADSR has four
    //     faders). The fifteen value-labeled-knob initializers
    //     (initialiseOscTuneControl ... initialiseLFORateControl) all
    //     route through initialiseValueLabeledControl above.
    // ES: Inicializadores de controles específicos. Cada uno cablea un
    //     solo control o un grupo estrechamente relacionado (p. ej.
    //     un ADSR tiene cuatro faders). Los quince inicializadores de
    //     knobs con label de valor (initialiseOscTuneControl ...
    //     initialiseLFORateControl) pasan por
    //     initialiseValueLabeledControl de arriba.
    void initialiseAmpEnvelopeControls();
    void initialiseFilterEnvelopeADSRControls();
    void initialiseOscWaveSelectors();
    void initialiseKnobs();
    void initialiseOscTuneControl();
    void initialiseStereoWidthControl();
    void initialiseNoiseControl();
    void initialiseOscFineControl();
    void initialiseOctaveControl();
    void initialiseTuningControl();
    void initialiseGlideRateControl();
    void initialiseGlideBendControl();
    void initialiseVibratoControl();
    void initialiseFilterVelocityControl();
    void initialiseFilterEnvControl();
    void initialiseFilterLFOControl();
    void initialiseFilterKeycenterControl();
    void initialiseFilterKeytrackControl();
    void initialiseLFORateControl();
    void initialisePresetSelector();
    void initialiseGlideModeSelector();
    void initialisePolyToggle();
    void initialiseFilterTypeControl();


    // ========================================================================
    //  VALUE LABEL FORMATTERS / FORMATEADORES DE LABELS DE VALOR
    // ========================================================================

    // EN: Each control with a value label has two paired functions:
    //       formatX(value)        : returns the user-readable string for
    //                               a given value (units, special states
    //                               like "OFF" / "PWM 25.0", note names,
    //                               etc.). Some formatters pad with
    //                               non-breaking spaces so values of
    //                               different widths align in the GUI.
    //       updateXValueLabel()   : reads the current value from the
    //                               slider, formats it with formatX and
    //                               pushes the result to the label.
    //     parameterChanged() routes the right updateX call by parameter
    //     ID, so labels stay in sync with both GUI and host changes.
    // ES: Cada control con label de valor tiene dos funciones pareadas:
    //       formatX(value)        : devuelve la string legible para un
    //                               valor dado (unidades, estados
    //                               especiales como "OFF" / "PWM 25.0",
    //                               nombres de nota, etc.). Algunos
    //                               formateadores añaden espacios
    //                               no-rompibles para que valores de
    //                               distintos anchos queden alineados
    //                               en la GUI.
    //       updateXValueLabel()   : lee el valor actual del slider, lo
    //                               formatea con formatX y envía el
    //                               resultado al label.
    //     parameterChanged() enruta la llamada updateX correcta según
    //     el ID del parámetro, así los labels quedan sincronizados con
    //     cambios del GUI y del host.

    juce::String formatLFORateValue(double value) const;
    void updateLFORateValueLabel();

    juce::String formatFilterLFOValue(double value) const;
    void updateFilterLFOValueLabel();

    juce::String formatFilterEnvValue(double value) const;
    void updateFilterEnvValueLabel();

    juce::String formatFilterKeycenterValue(double value) const;
    void updateFilterKeycenterValueLabel();

    juce::String formatFilterKeytrackValue(double value) const;
    void updateFilterKeytrackValueLabel();

    juce::String formatFilterVelocityValue(double value) const;
    void updateFilterVelocityValueLabel();

    juce::String formatVibratoValue(double value) const;
    void updateVibratoValueLabel();

    juce::String formatGlideBendValue(double value) const;
    void updateGlideBendValueLabel();

    juce::String formatGlideRateValue(double value) const;
    void updateGlideRateValueLabel();

    juce::String formatTuningValue(double value) const;
    void updateTuningValueLabel();

    juce::String formatOctaveValue(double value) const;
    void updateOctaveValueLabel();

    juce::String formatNoiseValue(double value) const;
    void updateNoiseValueLabel();

    juce::String formatStereoWidthValue(double value) const;
    void updateStereoWidthValueLabel();

    juce::String formatOscTuneValue(double value) const;
    void updateOscTuneValueLabel();

    juce::String formatOscFineValue(double value) const;
    void updateOscFineValueLabel();


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AndesJXAudioProcessorEditor)
};
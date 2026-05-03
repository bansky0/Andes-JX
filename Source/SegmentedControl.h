/*
  ==============================================================================

    SegmentedControl.h
    Created: 1 Apr 2026 12:20:00pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: SegmentedControl
    Purpose:
        EN: Custom JUCE component that displays a horizontal row of
            mutually-exclusive buttons styled as a single rounded
            segmented control (iOS-style). Used in the AndesJX UI for
            discrete selectors such as oscillator waveform, filter type,
            polyphony mode and glide mode.
        ES: Componente custom de JUCE que muestra una fila horizontal de
            botones mutuamente excluyentes con apariencia de control
            segmentado redondeado (estilo iOS). Se usa en la UI de
            AndesJX para selectores discretos como forma de onda de
            osciladores, tipo de filtro, modo de polifonía y modo de
            glide.

    Main responsibilities:
        EN:
          - Build a row of TextButtons sharing a radio group, so only
            one can be selected at a time
          - Lay them out evenly across the component width
          - Paint a single rounded background and inter-button
            separators that hide the individual button frames
          - Expose a simple onChange(int) callback for index changes
          - Allow a custom LookAndFeel to be applied uniformly to all
            buttons
        ES:
          - Construir una fila de TextButtons que comparten un radio
            group, para que solo uno pueda estar seleccionado
          - Distribuirlos uniformemente a lo ancho del componente
          - Pintar un único fondo redondeado y separadores entre
            botones que ocultan los marcos individuales
          - Exponer un callback simple onChange(int) ante cambios de
            índice
          - Permitir aplicar un LookAndFeel custom uniformemente a
            todos los botones

    Architectural role:
        EN: Owned by PluginEditor, which reads the selected index via
            getSelectedIndex() / getSelectedText() or subscribes to
            onChange to react in real time. The component itself is
            agnostic of which APVTS parameter it represents; the editor
            wires that connection.
        ES: PluginEditor lo posee y lee el índice seleccionado vía
            getSelectedIndex() / getSelectedText(), o se suscribe a
            onChange para reaccionar en tiempo real. El componente en
            sí no sabe qué parámetro APVTS representa; el editor cablea
            esa conexión.

    Notes:
        EN:
          - Two parallel vectors (ownedButtons, buttons) are kept on
            purpose: the unique_ptr vector owns the buttons, the raw
            pointer vector is the convenient access path for indexed
            operations. Keeps the API readable without reaching
            .get() everywhere.
          - The custom paint() draws ONE rounded background plus
            internal vertical separators, instead of letting each
            button paint its own border. The buttons themselves are
            transparent and only contribute their text and toggle
            highlight.
          - All buttons live inside a child buttonsContainer Component
            rather than directly on `this`. This isolates the button
            sub-tree so the parent paint() can draw underneath them
            without interference from JUCE's component repainting
            optimizations.
        ES:
          - Se mantienen dos vectores paralelos (ownedButtons, buttons)
            a propósito: el vector de unique_ptr es dueño de los
            botones, el vector de punteros raw es el camino cómodo de
            acceso indexado. Mantiene la API legible sin tener que
            usar .get() en todas partes.
          - El paint() custom dibuja UN fondo redondeado más
            separadores verticales internos, en vez de dejar que cada
            botón pinte su propio marco. Los botones en sí son
            transparentes y solo aportan su texto y su highlight de
            toggle.
          - Todos los botones viven dentro de un Component hijo
            buttonsContainer, no directamente sobre `this`. Esto
            aísla el sub-árbol de botones para que el paint() del
            padre pueda dibujar por debajo sin interferir con las
            optimizaciones de repintado de JUCE.
*/

#pragma once
#include <JuceHeader.h>


class SegmentedControl : public juce::Component
{
public:
    SegmentedControl()
    {
        addAndMakeVisible(buttonsContainer);
    }


    // EN: Sets a custom LookAndFeel that will be applied to every
    //     button currently in the control and to any future ones added
    //     by setItems(). Useful for matching the AndesJX visual style.
    // ES: Asigna un LookAndFeel custom que se aplicará a cada botón
    //     existente y a los que se añadan luego con setItems(). Útil
    //     para igualar el estilo visual de AndesJX.
    void setLookAndFeelForButtons(juce::LookAndFeel* laf)
    {
        buttonLookAndFeel = laf;

        for (auto* b : buttons)
            b->setLookAndFeel(buttonLookAndFeel);
    }


    // EN: Rebuilds the control with a new list of segment labels. Each
    //     label becomes a TextButton in a shared radio group, so only
    //     one can be active at a time. The first item is selected by
    //     default. Calling this on an existing control discards every
    //     previous button cleanly through the unique_ptr vector.
    // ES: Reconstruye el control con una lista nueva de etiquetas. Cada
    //     etiqueta se convierte en un TextButton dentro de un radio
    //     group compartido, así solo uno puede estar activo. Se
    //     selecciona el primer ítem por defecto. Llamar esto sobre un
    //     control existente descarta todos los botones previos de
    //     manera limpia gracias al vector de unique_ptr.
    void setItems(const juce::StringArray& items, int radioGroupId)
    {
        buttons.clear();
        ownedButtons.clear();
        buttonsContainer.removeAllChildren();

        for (int i = 0; i < items.size(); ++i)
        {
            auto button = std::make_unique<juce::TextButton>(items[i]);

            // EN: Toggling state + radio group makes the buttons behave
            //     like a JUCE-native exclusive selector: clicking one
            //     un-toggles the rest automatically.
            // ES: Estado togglable + radio group hacen que los botones
            //     se comporten como un selector exclusivo nativo de
            //     JUCE: al pulsar uno, los demás se des-togglean
            //     automáticamente.
            button->setClickingTogglesState(true);
            button->setRadioGroupId(radioGroupId);
            button->setLookAndFeel(buttonLookAndFeel);

            // EN: Capture `i` by value so each lambda remembers its own
            //     button index and can report it through onChange.
            // ES: Capturar `i` por valor para que cada lambda recuerde
            //     su propio índice de botón y pueda reportarlo por
            //     onChange.
            auto* raw = button.get();
            raw->onClick = [this, i]()
                {
                    selectedIndex = i;

                    if (onChange != nullptr)
                        onChange(selectedIndex);

                    repaint();
                };

            buttonsContainer.addAndMakeVisible(raw);
            buttons.push_back(raw);
            ownedButtons.push_back(std::move(button));
        }

        // EN: Default selection: first segment, with no notification so
        //     the host does not see a spurious "user changed value"
        //     event during construction.
        // ES: Selección por defecto: primer segmento, sin notificación
        //     para que el host no vea un evento espurio de "el usuario
        //     cambió un valor" durante la construcción.
        if (!buttons.empty())
        {
            buttons.front()->setToggleState(true, juce::dontSendNotification);
            selectedIndex = 0;
        }
        else
        {
            selectedIndex = -1;
        }

        resized();
        repaint();
    }


    // EN: Programmatically selects a segment. Pass
    //     juce::dontSendNotification when the change is driven by code
    //     (e.g. APVTS sync) to avoid feedback loops; pass
    //     juce::sendNotification when you want listeners (onChange) to
    //     fire.
    // ES: Selecciona un segmento programáticamente. Pasar
    //     juce::dontSendNotification cuando el cambio viene del código
    //     (p. ej. sync con APVTS) para evitar bucles de retroalimentación;
    //     pasar juce::sendNotification cuando se desee que los
    //     listeners (onChange) se disparen.
    void setSelectedIndex(int newIndex,
        juce::NotificationType notification = juce::sendNotification)
    {
        if (!juce::isPositiveAndBelow(newIndex, static_cast<int>(buttons.size())))
            return;

        for (int i = 0; i < static_cast<int>(buttons.size()); ++i)
            buttons[i]->setToggleState(i == newIndex, juce::dontSendNotification);

        selectedIndex = newIndex;

        if (notification == juce::sendNotification && onChange != nullptr)
            onChange(selectedIndex);

        repaint();
    }

    int getSelectedIndex() const noexcept
    {
        return selectedIndex;
    }

    juce::String getSelectedText() const
    {
        if (!juce::isPositiveAndBelow(selectedIndex, static_cast<int>(buttons.size())))
            return {};

        return buttons[static_cast<size_t>(selectedIndex)]->getButtonText();
    }


    // EN: Called by listeners (typically PluginEditor) whenever the
    //     selected index changes, either by user click or by
    //     programmatic setSelectedIndex with notification.
    // ES: Lo invocan los listeners (típicamente PluginEditor) cuando
    //     cambia el índice seleccionado, ya sea por clic del usuario o
    //     por setSelectedIndex programático con notificación.
    std::function<void(int)> onChange;


    // EN: Lays out the buttons evenly across the available width. The
    //     last button absorbs any leftover pixel from the integer
    //     division, so the row always fills the full width without a
    //     visible gap on the right edge.
    // ES: Distribuye los botones uniformemente en el ancho disponible.
    //     El último botón absorbe los píxeles sobrantes de la división
    //     entera, así la fila llena el ancho completo sin un hueco
    //     visible en el borde derecho.
    void resized() override
    {
        buttonsContainer.setBounds(getLocalBounds());

        auto area = buttonsContainer.getLocalBounds();
        const int numButtons = static_cast<int>(buttons.size());

        if (numButtons <= 0)
            return;

        const int segmentWidth = area.getWidth() / numButtons;
        int x = 0;

        for (int i = 0; i < numButtons; ++i)
        {
            const int w = (i == numButtons - 1) ? (area.getWidth() - x) : segmentWidth;
            buttons[i]->setBounds(x, 0, w, area.getHeight());
            x += w;
        }
    }


    // EN: Custom paint that gives the control its iOS-style appearance:
    //       - Single rounded rectangle background covering all segments
    //       - Single rounded outline
    //       - Thin vertical separators between segments
    //     Each individual button is transparent (managed by the
    //     LookAndFeel) so the unified background shows through.
    // ES: Paint custom que da al control su apariencia estilo iOS:
    //       - Un único fondo rectangular redondeado que cubre todos
    //         los segmentos
    //       - Un único contorno redondeado
    //       - Separadores verticales finos entre segmentos
    //     Cada botón individual es transparente (lo maneja el
    //     LookAndFeel) para que el fondo unificado se vea a través.
    void paint(juce::Graphics& g) override
    {
        // EN: reduced(0.5f) shrinks the rectangle by half a pixel so
        //     the 1-pixel outline drawn afterwards stays inside the
        //     component's bounds (avoids clipping at the edges).
        // ES: reduced(0.5f) achica el rectángulo medio píxel para que
        //     el contorno de 1 píxel que se dibuja después quede
        //     dentro de los límites del componente (evita el clipping
        //     en los bordes).
        auto bounds = getLocalBounds().toFloat().reduced(0.5f);

        const auto bg = juce::Colour::fromRGB(0x4F, 0x6B, 0x72);
        const auto outline = bg.darker(0.35f);
        constexpr float cornerRadius = 2.0f;

        // EN: Outer rounded background.
        // ES: Fondo exterior redondeado.
        g.setColour(bg);
        g.fillRoundedRectangle(bounds, cornerRadius);

        // EN: Outer rounded border.
        // ES: Borde exterior redondeado.
        g.setColour(outline);
        g.drawRoundedRectangle(bounds, cornerRadius, 1.0f);

        // EN: Internal separators between segments. Drawn at the left
        //     edge of every button starting from the second one.
        //     The 1-pixel inset top and bottom keeps the separator
        //     from overlapping the outer border at the corners.
        // ES: Separadores internos entre segmentos. Se dibujan en el
        //     borde izquierdo de cada botón a partir del segundo.
        //     El inset de 1 píxel arriba y abajo evita que el
        //     separador se superponga con el borde exterior en las
        //     esquinas.
        for (int i = 1; i < static_cast<int>(buttons.size()); ++i)
        {
            const float x = static_cast<float>(buttons[i]->getX());
            g.drawVerticalLine(static_cast<int>(x),
                bounds.getY() + 1.0f,
                bounds.getBottom() - 1.0f);
        }
    }


private:
    // EN: Child Component that contains every TextButton. Having an
    //     intermediate container instead of parenting the buttons
    //     directly on `this` lets the parent's paint() draw underneath
    //     without being broken by JUCE's child-component repaint
    //     optimizations.
    // ES: Component hijo que contiene todos los TextButtons. Usar un
    //     contenedor intermedio en vez de parentar los botones
    //     directamente sobre `this` permite que el paint() del padre
    //     dibuje por debajo sin que las optimizaciones de repintado
    //     de hijos en JUCE lo rompan.
    juce::Component buttonsContainer;

    // EN: Owns the buttons. unique_ptr ensures clean teardown when the
    //     control is rebuilt or destroyed.
    // ES: Posee los botones. unique_ptr asegura limpieza correcta al
    //     reconstruir o destruir el control.
    std::vector<std::unique_ptr<juce::TextButton>> ownedButtons;

    // EN: Parallel raw-pointer vector for indexed access. Always
    //     mirrors ownedButtons in size and order.
    // ES: Vector paralelo de punteros raw para acceso indexado.
    //     Siempre refleja a ownedButtons en tamaño y orden.
    std::vector<juce::TextButton*> buttons;

    juce::LookAndFeel* buttonLookAndFeel = nullptr;
    int selectedIndex = -1;
};
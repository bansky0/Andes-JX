/*
  ==============================================================================

    MoogFilter.h
    Created: 12 Feb 2026 10:43:48am
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: MoogFilter
    Purpose:
        EN: Adapter that exposes LadderFilter through the IFilter interface,
            allowing the Moog ladder model to coexist with other filter
            implementations (SVF, etc.) inside Voice and Synth.
        ES: Adaptador que expone LadderFilter a través de la interfaz
            IFilter, permitiendo que el modelo Moog ladder coexista con
            otras implementaciones de filtro (SVF, etc.) en Voice y Synth.

    Main responsibilities:
        EN:
          - Inherit from IFilter and forward every call to a private
            LadderFilter instance
          - Bridge minor signature differences (e.g. prepare(double) on
            the interface vs setSampleRate(float) on LadderFilter)
        ES:
          - Heredar de IFilter y reenviar cada llamada a una instancia
            privada de LadderFilter
          - Cubrir pequeñas diferencias de firma (p. ej. prepare(double)
            de la interfaz vs setSampleRate(float) de LadderFilter)

    Architectural role:
        EN: Together with the SVFFilter wrapper, this class lets the synth
            switch filter algorithms at runtime by storing an IFilter*
            instead of a concrete type. Voice never references LadderFilter
            directly; it only sees IFilter.
        ES: Junto con el wrapper SVFFilter, esta clase permite que el synth
            cambie de algoritmo de filtro en tiempo de ejecución guardando
            un IFilter* en lugar de un tipo concreto. Voice nunca referencia
            a LadderFilter directamente; solo ve IFilter.

    Notes:
        EN:
          - This is the classic "Adapter" design pattern. It deliberately
            keeps LadderFilter free from any IFilter dependency, so the
            DSP class remains portable to projects that do not use this
            interface.
          - All methods are simple one-line forwards. Any future logic
            specific to the Moog flavor (extra saturation, drive control,
            etc.) would belong here, not inside LadderFilter.
        ES:
          - Es el patrón de diseño "Adapter" clásico. Mantiene a propósito
            a LadderFilter libre de cualquier dependencia de IFilter, para
            que la clase DSP sea portable a proyectos que no usen esta
            interfaz.
          - Todos los métodos son reenvíos de una línea. Cualquier lógica
            futura específica del sabor Moog (saturación extra, control
            de drive, etc.) iría aquí, no dentro de LadderFilter.
*/

#pragma once

#include "IFilter.h"
#include "LadderFilter.h"


class MoogFilter : public IFilter
{
public:
    MoogFilter() = default;


    // EN: Initializes the underlying LadderFilter and clears its state.
    //     Bridges IFilter's double-precision sample rate to the float
    //     used internally by LadderFilter.
    // ES: Inicializa el LadderFilter subyacente y limpia su estado.
    //     Adapta la sample rate de doble precisión de IFilter al float
    //     que usa internamente LadderFilter.
    void prepare(double sampleRate) override
    {
        moog.setSampleRate(static_cast<float>(sampleRate));
        moog.reset();
    }

    // EN: Forwards the reset request to the underlying filter.
    // ES: Reenvía la petición de reset al filtro subyacente.
    void reset() override
    {
        moog.reset();
    }

    // EN: Updates the sample rate of the underlying filter. The wrapped
    //     LadderFilter automatically recalculates its coefficients.
    // ES: Actualiza la sample rate del filtro subyacente. El LadderFilter
    //     envuelto recalcula sus coeficientes automáticamente.
    void setSampleRate(float sampleRate) override
    {
        moog.setSampleRate(sampleRate);
    }

    // EN: Both this interface and LadderFilter use a normalized resonance
    //     in [0, 1], so the value passes through unchanged. If the unit
    //     conventions ever diverge, the conversion would happen here.
    // ES: Tanto esta interfaz como LadderFilter usan una resonancia
    //     normalizada en [0, 1], por lo que el valor pasa sin cambios.
    //     Si las convenciones de unidades llegaran a diferir, la
    //     conversión iría aquí.
    void updateCoefficients(float cutoffHz, float resonance) override
    {
        moog.updateCoefficients(cutoffHz, resonance);
    }

    // EN: Forwards one audio sample to the underlying filter. This is
    //     the hot path, so the indirection is kept as cheap as possible.
    // ES: Reenvía una muestra de audio al filtro subyacente. Es la ruta
    //     caliente, por eso la indirección se mantiene lo más barata posible.
    float render(float input) override
    {
        return moog.render(input);
    }

    // EN: Identification string used for debugging and GUI display.
    // ES: Cadena de identificación usada para depuración y la GUI.
    const char* getFilterType() const override
    {
        return "Moog Ladder (Huovilainen)";
    }


private:
    // EN: The actual DSP. Wrapped instead of inherited so LadderFilter
    //     stays decoupled from the IFilter contract.
    // ES: El DSP real. Se envuelve (composición) en lugar de heredarse
    //     para que LadderFilter quede desacoplado del contrato IFilter.
    LadderFilter moog;
};
/*
  ==============================================================================

    IFilter.h
    Created: 11 Feb 2026 4:36:24pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: IFilter
    Purpose:
        EN: Pure abstract interface that all filter implementations in
            AndesJX must conform to. Lets Voice and Synth treat the available filter adapters
            (MoogFilter and SVFFilter) through the same handle.
        ES: Interfaz abstracta pura que toda implementación de filtro en
            AndesJX debe cumplir. Permite que Voice y Synth manejen los adaptadores de filtro disponibles
            (MoogFilter y SVFFilter) a través del mismo handle.

    Main responsibilities:
        EN:
          - Define the lifecycle methods every filter must implement
            (prepare, reset, setSampleRate)
          - Define the audio-rate processing entry point (render)
          - Define how cutoff and resonance are updated (updateCoefficients)
          - Provide a lightweight identification hook (getFilterType)
        ES:
          - Definir los métodos de ciclo de vida que todo filtro debe
            implementar (prepare, reset, setSampleRate)
          - Definir el punto de entrada de procesamiento por muestra (render)
          - Definir cómo se actualizan cutoff y resonancia (updateCoefficients)
          - Proveer un mecanismo ligero de identificación (getFilterType)

    Architectural role:
        // EN: Implemented by the adapter classes MoogFilter and SVFFilter.
        //     The internal DSP classes LadderFilter and SVF remain deliberately
        //     decoupled from this interface and are wrapped through composition.
        //
        // ES: Implementada por las clases adaptadoras MoogFilter y SVFFilter.
        //     Las clases DSP internas LadderFilter y SVF permanecen desacopladas
        //     deliberadamente de esta interfaz y se envuelven mediante composición.

    Notes:
        EN:
          - This is the "Strategy" design pattern applied to DSP. Adding
            a new filter algorithm only requires writing a class that
            inherits from IFilter; no changes to Voice or Synth.
          - The virtual destructor is essential: filters are destroyed
            through an IFilter* pointer, and without it the derived class
            destructor would not run, leaking resources.
          - All methods are pure virtual (= 0). This class cannot be
            instantiated directly; it exists only as a contract.
        ES:
          - Es el patrón de diseño "Strategy" aplicado a DSP. Añadir un
            nuevo algoritmo de filtro solo exige escribir una clase que
            herede de IFilter; sin cambios en Voice ni en Synth.
          - El destructor virtual es esencial: los filtros se destruyen
            a través de un puntero IFilter*, y sin él no se ejecutaría el
            destructor de la clase derivada, dejando recursos colgados.
          - Todos los métodos son virtuales puros (= 0). Esta clase no se
            puede instanciar directamente; existe solo como contrato.
*/

#pragma once


class IFilter
{
public:
    // EN: Virtual destructor. Required so that `delete iFilterPtr` calls
    //     the correct derived destructor.
    // ES: Destructor virtual. Necesario para que `delete iFilterPtr` llame
    //     al destructor derivado correcto.
    virtual ~IFilter() = default;


    // --- Lifecycle / Ciclo de vida -----------------------------------------

    // EN: Called by the host once before playback starts. Implementations
    //     should store the sample rate and precompute any rate-dependent
    //     coefficients.
    // ES: Llamado por el host una vez antes de iniciar la reproducción.
    //     Las implementaciones deben guardar la sample rate y precomputar
    //     los coeficientes dependientes de ella.
    virtual void prepare(double sampleRate) = 0;

    // EN: Clears the filter's internal state (delay lines, integrators).
    //     Called on note-on or when switching presets to avoid clicks.
    // ES: Limpia el estado interno del filtro (líneas de retardo,
    //     integradores). Se llama en note-on o al cambiar de preset
    //     para evitar clicks.
    virtual void reset() = 0;

    // EN: Updates the sample rate after `prepare`. Useful when the host
    //     changes sample rate mid-session.
    // ES: Actualiza la sample rate después de `prepare`. Útil cuando el
    //     host cambia la sample rate a mitad de sesión.
    virtual void setSampleRate(float sampleRate) = 0;


    // --- Audio processing / Procesamiento de audio -------------------------

    // EN: Recomputes the internal filter coefficients from the user-facing
    //     parameters (cutoff in Hz, resonance normalized to a per-filter
    //     range). Cheap enough to call per audio block, expensive enough
    //     that it should NOT be called per sample unless modulating.
    // ES: Recalcula los coeficientes internos a partir de los parámetros
    //     que ve el usuario (cutoff en Hz, resonancia normalizada al rango
    //     de cada filtro). Lo bastante barato para llamarse por bloque,
    //     lo bastante caro como para NO llamarse por muestra salvo al
    //     modular en tiempo real.
    virtual void updateCoefficients(float cutoffHz, float resonance) = 0;

    // EN: Processes one audio sample. The hot path of every filter; must
    //     be allocation-free and lock-free.
    // ES: Procesa una muestra de audio. La ruta caliente de todo filtro;
    //     debe estar libre de reservas y libre de locks.
    virtual float render(float input) = 0;


    // --- Introspection / Introspección -------------------------------------

    // EN: Returns a short, human-readable filter name (e.g. "Ladder",
    //     "SVF"). Useful for debugging and for showing the active filter
    //     in the GUI.
    // ES: Devuelve un nombre corto y legible del filtro (p. ej. "Ladder",
    //     "SVF"). Útil para depuración y para mostrar el filtro activo
    //     en la GUI.
    virtual const char* getFilterType() const = 0;
};
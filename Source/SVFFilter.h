/*
  ==============================================================================

    SVFFilter.h
    Created: 11 Feb 2026 4:49:15pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: SVFFilter
    Purpose:
        EN: Adapter that exposes a 24 dB/oct cascade of two SVF stages
            through the IFilter interface. Provides a clean lowpass
            alternative to the Moog-style ladder while using the same
            fourth-order nominal slope.

        ES: Adaptador que expone una cascada de 24 dB/oct de dos etapas
            SVF a través de la interfaz IFilter. Proporciona una
            alternativa lowpass limpia al ladder estilo Moog usando la
            misma pendiente nominal de cuarto orden.

    Main responsibilities:
        EN:
          - Inherit from IFilter and implement the full contract
          - Cascade two SVF instances to double the nominal slope from
            12 to 24 dB/oct
          - Convert normalized resonance [0, 1] into the Q units expected
            by the underlying SVF stages
          - Apply an empirical gain attenuation at high resonance

        ES:
          - Heredar de IFilter e implementar el contrato completo
          - Encadenar dos instancias SVF para duplicar la pendiente nominal
            de 12 a 24 dB/oct
          - Convertir la resonancia normalizada [0, 1] a las unidades de Q
            esperadas por las etapas SVF subyacentes
          - Aplicar una atenuación empírica de ganancia a alta resonancia

    Architectural role:
        EN: Paired with MoogFilter via the IFilter interface. The synth
            picks between them at runtime according to the filterType
            parameter of the active Preset, without knowing which DSP
            algorithm is actually running.
        ES: Emparejado con MoogFilter a través de la interfaz IFilter.
            El synth elige entre ambos en tiempo de ejecución según el
            parámetro filterType del Preset activo, sin saber qué
            algoritmo DSP está corriendo realmente.

    Notes:
        EN:
          - The cascade uses a reduced Q on the second stage (Q2 = 0.7·Q).
            This empirical reduction moderates the resonant peak produced
            by cascading two second-order filters.
          - The gain attenuation  y *= 1 / (1 + 0.8·resonance)  is a
            practical, empirical scaling applied at high resonance. It is
            not a perceptual loudness compensation model.
          - This is the Adapter counterpart of MoogFilter (see MoogFilter.h).
            Together they illustrate the Strategy pattern from IFilter:
            same abstract contract, two different filter implementations.

        ES:
          - La cascada usa un Q reducido en la segunda etapa (Q2 = 0.7·Q).
            Esta reducción empírica modera el pico resonante producido al
            encadenar dos filtros de segundo orden.
          - La atenuación de ganancia  y *= 1 / (1 + 0.8·resonance)  es un
            escalado práctico y empírico aplicado a alta resonancia. No es
            un modelo de compensación perceptual de loudness.
          - Es la contraparte Adapter de MoogFilter (ver MoogFilter.h).
            Juntos ilustran el patrón Strategy desde IFilter: mismo
            contrato abstracto, dos implementaciones de filtro distintas.
*/

#pragma once

#include "IFilter.h"
#include "SVF.h"
#include <algorithm>


class SVFFilter : public IFilter
{
public:
    SVFFilter() = default;


    // EN: Initializes both cascaded SVF stages and clears state.
    // ES: Inicializa ambas etapas SVF encadenadas y limpia el estado.
    void prepare(double newSampleRate) override
    {
        svf1.prepare(static_cast<float>(newSampleRate));
        svf2.prepare(static_cast<float>(newSampleRate));
        reset();
    }

    // EN: Clears the internal state of both SVF stages.
    // ES: Limpia el estado interno de ambas etapas SVF.
    void reset() override
    {
        svf1.reset();
        svf2.reset();
    }

    // EN: Updates the sample rate of both stages. Note that SVF::prepare
    //     is the same as "setSampleRate" for this class.
    // ES: Actualiza la sample rate de ambas etapas. Nótese que SVF::prepare
    //     cumple el rol de "setSampleRate" para esta clase.
    void setSampleRate(float newSampleRate) override
    {
        this->sampleRate = newSampleRate;
        svf1.prepare(newSampleRate);
        svf2.prepare(newSampleRate);
    }

    // EN: Computes coefficients for both cascaded SVF stages. Converts
    //     normalized resonance [0, 1] into Q and applies a reduced Q on
    //     the second stage to moderate the resonant peak of the cascade.
    //
    // ES: Calcula coeficientes para ambas etapas SVF encadenadas. Convierte
    //     la resonancia normalizada [0, 1] a Q y aplica un Q reducido en la
    //     segunda etapa para moderar el pico resonante de la cascada.
    void updateCoefficients(float cutoffHz, float resonance) override
    {
        resonance = std::clamp(resonance, 0.0f, 1.0f);
        lastRes = resonance;

        // EN: Initial map from normalized resonance to Q. The underlying SVF
        //     stages clamp Q to [0.707, 10], so very low resonance values are
        //     effectively limited to Butterworth damping.
        //
        // ES: Mapeo inicial de resonancia normalizada a Q. Las etapas SVF
        //     subyacentes limitan Q a [0.707, 10], por lo que valores muy bajos
        //     de resonancia quedan efectivamente limitados a amortiguamiento
        //     Butterworth.
        float Q = 0.5f + resonance * 9.5f;
        Q = std::clamp(Q, 0.5f, 10.0f);

        // EN: For a 24 dB/oct cascade, a slightly lower Q on the second stage
        //     moderates the resonant peak that would result from cascading two
        //     identical high-Q sections. The 0.7 factor is empirical.
        //
        // ES: Para una cascada de 24 dB/oct, un Q ligeramente menor en la segunda
        //     etapa modera el pico resonante que resultaría de encadenar dos
        //     secciones idénticas con Q alto. El factor 0.7 es empírico.
        float Q2 = 0.7f * Q;

        svf1.updateCoefficients(cutoffHz, Q);
        svf2.updateCoefficients(cutoffHz, Q2);
    }

    // EN: Processes one audio sample through both cascaded stages and
    //     applies loudness compensation based on the active resonance.
    // ES: Procesa una muestra a través de ambas etapas encadenadas y
    //     aplica compensación de loudness según la resonancia activa.
    float render(float input) override
    {
        float y = svf1.render(input);
        y = svf2.render(y);

        // EN: Empirical gain attenuation. High resonance can produce a large
        //     resonant peak; this simple scaling reduces the output level as
        //     resonance increases. It does not guarantee constant perceived
        //     loudness.
        //
        // ES: Atenuación empírica de ganancia. La resonancia alta puede producir
        //     un pico resonante grande; este escalado simple reduce el nivel de
        //     salida conforme aumenta la resonancia. No garantiza loudness
        //     percibido constante.
        y *= 1.0f / (1.0f + 0.8f * lastRes);

        return y;
    }

    // EN: Identification string for debugging and GUI display.
    // ES: Cadena de identificación para depuración y la GUI.
    const char* getFilterType() const override
    {
        return "SVF (State Variable)";
    }


private:
    // EN: Two SVF stages cascaded in series. Each stage is 12 dB/oct,
    //     so the total slope is 24 dB/oct (matching the Moog ladder).
    // ES: Dos etapas SVF encadenadas en serie. Cada etapa es 12 dB/oct,
    //     así la pendiente total es 24 dB/oct (igualando al Moog ladder).
    SVF svf1, svf2;

    // EN: Cached resonance value, used by render() for loudness compensation.
    //     Stored here to avoid passing it through every sample call.
    // ES: Valor de resonancia cacheado, usado por render() para la
    //     compensación de loudness. Se guarda aquí para evitar pasarlo
    //     en cada llamada por muestra.
    float lastRes = 0.0f;

    float sampleRate = 48000.0f;
};
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
            through the IFilter interface. Provides a cleaner, more
            surgical alternative to the Moog ladder while matching its
            per-octave slope.
        ES: Adaptador que expone una cascada de 24 dB/oct de dos etapas
            SVF a través de la interfaz IFilter. Proporciona una
            alternativa más limpia y quirúrgica al Moog ladder,
            igualando su pendiente por octava.

    Main responsibilities:
        EN:
          - Inherit from IFilter and implement the full contract
          - Cascade two SVF instances to double the slope from 12 to
            24 dB/oct
          - Convert the normalized resonance [0, 1] into the Q units
            expected by the underlying SVF
          - Apply a loudness compensation that prevents runaway volume
            at high resonance
        ES:
          - Heredar de IFilter e implementar el contrato completo
          - Encadenar dos instancias SVF para duplicar la pendiente de
            12 a 24 dB/oct
          - Convertir la resonancia normalizada [0, 1] a las unidades
            de Q que espera el SVF subyacente
          - Aplicar compensación de loudness para evitar un volumen
            descontrolado con resonancia alta

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
            Two identical SVFs in series would produce a narrower, more
            aggressive peak than the Moog ladder; the reduction empirically
            matches their perceived character.
          - The loudness compensation  y *= 1 / (1 + 0.8·resonance)  is a
            practical fix, not a physically derived formula. It trades a
            tiny amount of low-resonance level for a much more usable
            high-resonance range.
          - This is the Adapter counterpart of MoogFilter (see MoogFilter.h).
            Together they illustrate the Strategy pattern from IFilter:
            same abstract contract, two different sonic characters.
        ES:
          - La cascada usa Q reducido en la segunda etapa (Q2 = 0.7·Q).
            Dos SVFs idénticos en serie producirían un pico más estrecho
            y agresivo que el Moog ladder; la reducción iguala
            empíricamente el carácter percibido de ambos filtros.
          - La compensación de loudness  y *= 1 / (1 + 0.8·resonance)  es
            un arreglo práctico, no una fórmula derivada físicamente.
            Sacrifica un poco de nivel en baja resonancia a cambio de un
            rango mucho más usable en alta resonancia.
          - Es la contraparte Adapter de MoogFilter (ver MoogFilter.h).
            Juntos ilustran el patrón Strategy desde IFilter: mismo
            contrato abstracto, dos caracteres sonoros distintos.
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
    //     the normalized resonance [0, 1] into Q and applies a reduced Q
    //     on the second stage to match the Moog ladder's character.
    // ES: Calcula coeficientes para ambas etapas SVF encadenadas.
    //     Convierte la resonancia normalizada [0, 1] a Q y aplica un Q
    //     reducido en la segunda etapa para igualar el carácter del
    //     Moog ladder.
    void updateCoefficients(float cutoffHz, float resonance) override
    {
        resonance = std::clamp(resonance, 0.0f, 1.0f);
        lastRes = resonance;

        // EN: Linear map from normalized resonance to Q:
        //       resonance = 0 -> Q = 0.5  (no resonance)
        //       resonance = 1 -> Q = 10   (near self-oscillation)
        // ES: Mapeo lineal de resonancia normalizada a Q:
        //       resonance = 0 -> Q = 0.5  (sin resonancia)
        //       resonance = 1 -> Q = 10   (cerca de auto-oscilación)
        float Q = 0.5f + resonance * 9.5f;
        Q = std::clamp(Q, 0.5f, 10.0f);

        // EN: For a 24 dB/oct cascade, a slightly lower Q on the second
        //     stage prevents the SVF from sounding more aggressive than
        //     the Moog ladder at comparable resonance settings. The 0.7
        //     factor is empirical, derived from A/B comparison.
        // ES: Para una cascada de 24 dB/oct, un Q ligeramente menor en la
        //     segunda etapa evita que el SVF suene más agresivo que el
        //     Moog ladder a ajustes de resonancia comparables. El factor
        //     0.7 es empírico, derivado de comparaciones A/B.
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

        // EN: Loudness compensation. High resonance narrows the response
        //     peak and boosts its gain, which can clip the output. This
        //     attenuation keeps the perceived level consistent across the
        //     resonance range.
        // ES: Compensación de loudness. Resonancia alta estrecha el pico
        //     de respuesta e incrementa su ganancia, lo que puede saturar
        //     la salida. Esta atenuación mantiene el nivel percibido
        //     consistente en todo el rango de resonancia.
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
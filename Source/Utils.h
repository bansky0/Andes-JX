/*
  ==============================================================================

    Utils.h
    Created: 12 Nov 2025 3:36:34pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: Utils
    Purpose:
        EN: Lightweight helpers used across the synthesizer that do not belong
            to any specific DSP module.
        ES: Utilidades ligeras usadas en todo el sintetizador que no
            pertenecen a ningún módulo DSP en particular.

    Main responsibilities:
        EN:
          - Audio safety net: detect NaN/Inf and clamp out-of-range samples
          - Type-safe access to JUCE parameters via a templated cast helper
        ES:
          - Red de seguridad: detectar NaN/Inf y limitar muestras fuera de rango
          - Acceso tipado a parámetros de JUCE mediante un cast con template

    Notes:
        EN:
          - protectYourEars is intended for development. It logs warnings via
            DBG() and silences the buffer on catastrophic errors. In a final
            release build it can be left in place (very low CPU cost).
          - castParameter relies on dynamic_cast and a jassert; mismatched
            types fail loudly in debug builds, which is the desired behavior.
        ES:
          - protectYourEars está pensada para desarrollo. Registra avisos con
            DBG() y silencia el buffer ante errores catastróficos. En una
            release final puede quedarse (su costo de CPU es muy bajo).
          - castParameter usa dynamic_cast y un jassert; los errores de tipo
            fallan ruidosamente en debug, que es el comportamiento deseado.
*/

#pragma once
#include <cmath>
#include <JuceHeader.h>


// --- Audio safety / Seguridad de audio -------------------------------------

// EN: Inspects an audio buffer and protects the user from catastrophic DSP
//     output. NaN/Inf and samples beyond ±2.0 silence the whole buffer.
//     Samples between ±1.0 and ±2.0 are clamped to the legal range.
// ES: Inspecciona un buffer de audio y protege al usuario de salidas DSP
//     catastróficas. NaN/Inf y muestras más allá de ±2.0 silencian todo el
//     buffer. Muestras entre ±1.0 y ±2.0 se limitan al rango válido.
inline void protectYourEars(float* buffer, int sampleCount)
{
    if (buffer == nullptr) { return; }

    // EN: Avoids flooding the log when many consecutive samples need clamping.
    // ES: Evita inundar el log cuando muchas muestras seguidas necesitan clamp.
    bool firstWarning = true;

    for (int i = 0; i < sampleCount; ++i) {
        float x = buffer[i];
        bool silence = false;

        if (std::isnan(x)) {
            DBG("!!! WARNING: nan detected in audio buffer, silencing !!!");
            silence = true;
        }
        else if (std::isinf(x)) {
            DBG("!!! WARNING: inf detected in audio buffer, silencing !!!");
            silence = true;
        }
        else if (x < -2.0f || x > 2.0f) { // screaming feedback / feedback descontrolado
            DBG("!!! WARNING: sample out of range, silencing !!!");
            silence = true;
        }
        else if (x < -1.0f) {
            if (firstWarning) {
                DBG("!!! WARNING: sample out of range, clamping !!!");
                firstWarning = false;
            }
            buffer[i] = -1.0f;
        }
        else if (x > 1.0f) {
            if (firstWarning) {
                DBG("!!! WARNING: sample out of range, clamping !!!");
                firstWarning = false;
            }
            buffer[i] = 1.0f;
        }

        // EN: On catastrophic failure, zero the entire buffer (not just the
        //     remaining samples) and exit. Already-processed samples are
        //     also discarded by design: a corrupted buffer is untrustworthy.
        // ES: Ante un fallo catastrófico se silencia el buffer completo (no
        //     solo lo restante) y se sale. Las muestras ya procesadas también
        //     se descartan a propósito: un buffer corrupto no es confiable.
        if (silence) {
            memset(buffer, 0, sampleCount * sizeof(float));
            return;
        }
    }
}


// --- JUCE parameter access / Acceso a parámetros de JUCE -------------------

// EN: Type-safe access to a JUCE parameter. Performs a dynamic_cast from the
//     generic AudioProcessorParameter* to the concrete subtype expected by
//     the caller (e.g. AudioParameterFloat*, AudioParameterChoice*).
//     The jassert traps wrong IDs or wrong target types in debug builds.
// ES: Acceso tipado a un parámetro de JUCE. Hace dynamic_cast del puntero
//     genérico AudioProcessorParameter* al subtipo concreto esperado por
//     quien llama (ej. AudioParameterFloat*, AudioParameterChoice*).
//     El jassert detecta IDs incorrectos o tipos erróneos en debug.
template<typename T>
inline static void castParameter(juce::AudioProcessorValueTreeState& apvts,
    const juce::ParameterID& id,
    T& destination)
{
    destination = dynamic_cast<T>(apvts.getParameter(id.getParamID()));
    jassert(destination);  // EN: parameter does not exist or wrong type
                           // ES: el parámetro no existe o el tipo es incorrecto
}
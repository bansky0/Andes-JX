/*
  ==============================================================================

    Utils.h

    ESP:
    Utilidades auxiliares compartidas del sintetizador.
    Este archivo reúne funciones pequeñas de soporte que no pertenecen a un
    módulo DSP específico, pero ayudan a mantener estabilidad y claridad en el
    proyecto.

    Actualmente incluye:
    - protectYourEars(): protección básica contra valores inválidos o peligrosos
      en buffers de audio.
    - castParameter(): helper para obtener y castear parámetros desde el APVTS.

    ENG:
    Shared helper utilities for the synthesizer.
    This file contains small support functions that do not belong to a specific
    DSP module, but help maintain stability and clarity across the project.

    Currently includes:
    - protectYourEars(): basic protection against invalid or dangerous values
      in audio buffers.
    - castParameter(): helper to retrieve and cast parameters from the APVTS.

  ==============================================================================
*/

#pragma once
#include <cmath>
#include <JuceHeader.h>

//------------------------------------------------------------------------------
// ESP: Protección de seguridad para buffers de audio.
//      Recorre el buffer y detecta:
//
//      - NaN (Not a Number)
//      - Inf (infinito)
//      - muestras muy fuera de rango (|x| > 2.0), indicio de feedback o error grave
//      - muestras levemente fuera de rango (|x| > 1.0), que se recortan (clamp)
//
//      Si detecta un error grave, silencia TODO el buffer para evitar dañar
//      oídos, altavoces o el flujo de prueba.
//
// ENG: Safety protection for audio buffers.
//      Scans the buffer and detects:
//
//      - NaN (Not a Number)
//      - Inf (infinity)
//      - samples far out of range (|x| > 2.0), often indicating feedback or severe error
//      - mildly out-of-range samples (|x| > 1.0), which are clamped
//
//      If a severe error is detected, the entire buffer is silenced to help
//      protect ears, speakers, and the debugging workflow.
//------------------------------------------------------------------------------
inline void protectYourEars(float* buffer, int sampleCount)
{
    if (buffer == nullptr) { return; }

    // ESP: Se usa para no imprimir repetidamente el mismo warning de clamp
    //      dentro del mismo buffer.
    // ENG: Used to avoid printing the same clamp warning repeatedly within
    //      the same buffer.
    bool firstWarning = true;

    for (int i = 0; i < sampleCount; ++i) {
        float x = buffer[i];
        bool silence = false;

        // ESP/ENG: NaN detection
        if (std::isnan(x)) {
            DBG("!!! WARNING: nan detected in audio buffer, silencing !!!");
            silence = true;
        }
        // ESP/ENG: Infinity detection
        else if (std::isinf(x)) {
            DBG("!!! WARNING: inf detected in audio buffer, silencing !!!");
            silence = true;
        }
        // ESP: Valores muy fuera del rango esperado [-1, 1].
        //      Esto suele ser más serio que un simple clipping.
        // ENG: Values far beyond the expected [-1, 1] range.
        //      Usually more serious than simple clipping.
        else if (x < -2.0f || x > 2.0f) { // screaming feedback
            DBG("!!! WARNING: sample out of range, silencing !!!");
            silence = true;
        }
        // ESP/ENG: Mild negative overflow -> clamp to -1.0
        else if (x < -1.0f) {
            if (firstWarning) {
                DBG("!!! WARNING: sample out of range, clamping !!!");
                firstWarning = false;
            }
            buffer[i] = -1.0f;
        }
        // ESP/ENG: Mild positive overflow -> clamp to +1.0
        else if (x > 1.0f) {
            if (firstWarning) {
                DBG("!!! WARNING: sample out of range, clamping !!!");
                firstWarning = false;
            }
            buffer[i] = 1.0f;
        }

        // ESP: Si se detectó un error grave, se silencia el buffer completo
        //      y se sale inmediatamente.
        // ENG: If a severe error was detected, silence the entire buffer
        //      and exit immediately.
        if (silence) {
            memset(buffer, 0, sampleCount * sizeof(float));
            return;
        }
    }
}

//------------------------------------------------------------------------------
// ESP: Helper para obtener un parámetro desde AudioProcessorValueTreeState y
//      convertirlo al tipo esperado mediante dynamic_cast.
//
//      Uso típico:
//      - obtener un puntero tipado a AudioParameterFloat, AudioParameterChoice, etc.
//      - centralizar validación con jassert
//
//      Nota: destination debe ser un tipo compatible con el resultado del cast,
//      normalmente un puntero.
//
// ENG: Helper to retrieve a parameter from AudioProcessorValueTreeState and
//      cast it to the expected type via dynamic_cast.
//
//      Typical use:
//      - get a typed pointer to AudioParameterFloat, AudioParameterChoice, etc.
//      - centralize validation with jassert
//
//      Note: destination must be compatible with the cast result,
//      typically a pointer type.
//------------------------------------------------------------------------------
template<typename T>
inline static void castParameter(juce::AudioProcessorValueTreeState& apvts, const juce::ParameterID& id, T& destination)
{
    destination = dynamic_cast<T>(apvts.getParameter(id.getParamID()));

    // ESP: Falla si el parámetro no existe o no coincide con el tipo esperado.
    // ENG: Fails if the parameter does not exist or does not match the expected type.
    jassert(destination);  // parameter does not exist or wrong type
}
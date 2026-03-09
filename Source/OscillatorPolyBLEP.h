/*
  ==============================================================================
    OscillatorPolyBLEP.h

    ESP:
    Motor de oscilador band-limited basado en PolyBLEP (Polynomial Band-Limited Step).
    PolyBLEP suaviza las discontinuidades en ondas como saw/square/PWM para reducir
    aliasing sin necesidad de oversampling. Incluye generación de seno, saw, square,
    PWM y triángulo (este último por integración con “leak” para evitar drift DC).

    ENG:
    Band-limited oscillator engine based on PolyBLEP (Polynomial Band-Limited Step).
    PolyBLEP smooths discontinuities in saw/square/PWM to reduce aliasing without
    oversampling. Provides sine, saw, square, PWM, and triangle (triangle via leaky
    integration to avoid DC drift).
  ==============================================================================
*/

#pragma once

#include <cmath>
#include <algorithm>
#include "Constants.h"

class OscillatorPolyBLEP
{
public:
    //==========================================================================
    // ESP: Inicializa variables dependientes del sampleRate y reinicia estado interno.
    // ENG: Initializes sample-rate dependent state and resets internal variables.
    void prepare(double sampleRate);

    //==========================================================================
    // ESP: Configura la frecuencia fundamental en Hz.
    // ENG: Sets fundamental frequency in Hz.
    void setFrequency(float freq);

    //==========================================================================
    // ESP: Configura el ancho de pulso (PWM). Se limita para evitar casos degenerados:
    //      - duty muy cerca de 0 o 1 causa bordes extremadamente estrechos, lo que
    //        aumenta aliasing y puede volver inestable la corrección.
    // ENG: Sets PWM pulse width. Clamped to avoid degenerate cases:
    //      - duty cycles near 0 or 1 create extremely narrow pulses, increasing
    //        aliasing and potentially destabilizing the correction.
    void setPulseWidth(float width)
    {
        pulseWidth = std::clamp(width, 0.05f, 0.95f);
    } //

    //==========================================================================
    // ESP: Reinicia fase e integrador (útil al iniciar una nota).
    // ENG: Resets phase and integrator (useful at note start).
    void reset()
    {
        phase = 0.f;
        integrator = 0.f;
    } //

    //==========================================================================
    // ESP: Sincroniza fase copiándola desde otro oscilador (hard-sync simple).
    // ENG: Phase sync by copying phase from another oscillator (simple hard-sync).
    void syncPhase(const OscillatorPolyBLEP& other);

    //==========================================================================
    // ESP: Formas de onda disponibles (salida típica ~[-1..1], dependiendo de la forma).
    // ENG: Available waveforms (typical output ~[-1..1], depending on waveform).
    float sine();
    float saw();
    float square();
    float squarePWM();
    float triangle();

    //==========================================================================
    // ESP: API genérico: devuelve una forma por defecto.
    // ENG: Generic API: returns a default waveform.
    float nextSample();

private:
    //==========================================================================
    // ESP: Estado interno.
    //      phase está normalizada en [0..1). El avance por muestra depende de frequency/sampleRate.
    // ENG: Internal state.
    //      phase is normalized in [0..1). Per-sample increment depends on frequency/sampleRate.
    float sampleRate = 48000.f;
    float phase = 0.0f;

    // ESP: phaseInc existe pero en el .cpp no se usa (se calcula “on the fly”).
    // ENG: phaseInc exists but is not used in the .cpp (increment is computed on the fly).
    float phaseInc = 0.0f; // (unused in current implementation)

    float frequency = 440.f;

    // ESP/ENG: Integrador para triangle (a partir de square band-limited).
    float integrator = 0.0f;

    // ESP/ENG: Duty-cycle para PWM.
    float pulseWidth = 0.5f;

    //==========================================================================
    // ESP: PolyBLEP: corrección local alrededor de discontinuidades (bordes).
    // ENG: PolyBLEP: local correction around discontinuities (edges).
    float polyBLEP(float t) const;

    //==========================================================================
    // ESP: Avanza la fase y hace wrap para mantener [0..1).
    // ENG: Advances phase and wraps around to keep [0..1).
    float advance();
};
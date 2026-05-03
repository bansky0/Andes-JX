/*
  ==============================================================================

    Constants.h
    Created: 21 Nov 2025 11:49:00am
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: Constants
    Purpose:
        EN: Centralizes global definitions used across the synthesizer.
        ES: Centraliza definiciones globales usadas por todo el sintetizador.

    Main responsibilities:
        EN:
          - Provide math constants tuned for real-time audio (float precision)
          - Define synth engine behavior (analog drift, sustain sentinel)
          - Set hard limits (max voices, LFO update rate)
          - Declare the preset parameter count as an architectural contract
        ES:
          - Proveer constantes matemáticas para audio en tiempo real (float)
          - Definir el comportamiento del motor (drift analógico, sustain)
          - Establecer límites duros (voces máximas, tasa de LFO)
          - Declarar el número de parámetros del preset como contrato

    Architectural role:
        EN: Included by virtually every DSP module (Oscillator, Envelope, Voice,
            Synth). A value lives here only if it is shared by multiple modules
            or defines a contract enforced across the codebase.
        ES: Incluido por casi todos los módulos DSP. Un valor vive aquí solo si
            es compartido por varios módulos o define un contrato del sistema.

    Notes:
        EN:
          - NUM_PARAMS is `const int` (not constexpr) to emphasize that it
            is a shared interface contract, not a pure numeric constant.
          - Math constants are declared locally as float to avoid implicit
            double→float conversions inside the audio callback.
        ES:
          - NUM_PARAMS es `const int` (no constexpr) para enfatizar que es
            un contrato de interfaz, no una constante numérica pura.
          - Las constantes matemáticas se declaran como float localmente
            para evitar conversiones double→float en el audio callback.
*/

#pragma once


// --- Math constants / Constantes matemáticas -------------------------------

static constexpr float PI = 3.14159265358979323846f;
static constexpr float TWO_PI = 6.28318530717958647692f;  // EN: full cycle  |  ES: ciclo completo
static constexpr float PI_OVER_4 = 0.7853981634f;         // EN: stereo pan law  |  ES: ley de paneo


// --- Synth engine / Motor de síntesis --------------------------------------

// EN: Analog drift in semitones per voice index, emulates vintage oscillator instability.
// ES: Deriva analógica en semitonos por índice de voz; emula la inestabilidad vintage.
static constexpr float ANALOG = 0.003f;

// EN: Sentinel value used by the ADSR envelope to mark the sustain phase.
// ES: Valor centinela usado por la envolvente ADSR para marcar la fase de sustain.
static const int SUSTAIN = -1;


// --- Synth limits / Límites del sintetizador -------------------------------

// EN: Maximum simultaneous voices. Classic 8-voice polyphony (Prophet-5, Juno-60).
// ES: Voces simultáneas máximas. Polifonía clásica de 8 voces (Prophet-5, Juno-60).
static constexpr int MAX_VOICES = 8;

// EN: LFO update period in samples; reduces CPU since the LFO is sub-audio.
// ES: Periodo de actualización del LFO en muestras; ahorra CPU porque es sub-audio.
static constexpr int LFO_MAX = 32;


// --- Preset structure / Estructura de presets ------------------------------

// EN: Number of parameters per Preset. Changing this requires updating Preset,
//     PluginProcessor and PluginEditor.
// ES: Número de parámetros por Preset. Cambiarlo obliga a actualizar Preset,
//     PluginProcessor y PluginEditor.
const int NUM_PARAMS = 32;
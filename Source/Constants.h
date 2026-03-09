/*
  ==============================================================================
    Constants.h

    ESP:
    Archivo de constantes compartidas del sintetizador. Centraliza valores
    numéricos "fijos" (matemática, límites y ajustes globales) para evitar
    duplicación, facilitar mantenimiento y asegurar coherencia entre módulos.

    ENG:
    Shared constants for the synthesizer. Centralizes "fixed" numeric values
    (math, limits, and global tuning knobs) to avoid duplication, simplify
    maintenance, and keep modules consistent.
  ==============================================================================
*/

#pragma once

//==============================================================================
// ESP: Constantes matemáticas de uso general (radianes).
// ENG: General-purpose math constants (radians).
static constexpr float PI        = 3.14159265358979323846f;
static constexpr float TWO_PI    = 6.28318530717958647692f;
static constexpr float PI_OVER_4 = 0.7853981634f;

//==============================================================================
// ESP: Constantes de síntesis (comportamiento/“carácter” global).
//      ANALOG introduce una variación muy sutil en semitonos por índice de voz,
//      para simular imperfecciones tipo analógico (drift/detune leve por voz).
// ENG: Synthesis constants (global character/behavior).
//      ANALOG adds a tiny semitone offset per voice index to emulate analog-like
//      imperfections (subtle drift/detune per voice).
static constexpr float ANALOG = 0.003f; // semitones per voice index

//==============================================================================
// ESP: Límites globales del sintetizador.
//      MAX_VOICES define la polifonía máxima del motor.
//      LFO_MAX define el límite/escala interna relacionada al LFO (p.ej. tablas,
//      pasos o rangos según implementación).
// ENG: Global synth limits.
//      MAX_VOICES defines the maximum polyphony.
//      LFO_MAX defines an internal limit/scale for the LFO (e.g., table size,
//      step count, or range depending on implementation).
static constexpr int MAX_VOICES = 8;
static constexpr int LFO_MAX    = 32;

//==============================================================================
// ESP: Número de parámetros almacenados en presets.
//      Mantener este valor sincronizado con la lista real de parámetros del
//      plugin/synth para evitar desfaces al guardar/cargar presets.
// ENG: Number of parameters stored in presets.
//      Keep this in sync with the actual parameter list to avoid mismatches
static constexpr int NUM_PARAMS = 28;
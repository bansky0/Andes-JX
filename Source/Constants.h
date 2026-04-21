/*
  ==============================================================================

    Constants.h
    Created: 21 Nov 2025 11:49:00am
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once

// Constantes matematicas
static constexpr float PI = 3.14159265358979323846f;
static constexpr float TWO_PI = 6.28318530717958647692f;
static constexpr float PI_OVER_4 = 0.7853981634f;
// Constantes de sintesis
static constexpr float ANALOG = 0.003f; // semitonos por indice de voz
static const int SUSTAIN = -1;

// Constantes del sintetizador
static constexpr int MAX_VOICES = 8;
static constexpr int LFO_MAX = 32;

// Numero de parametros presets
const int NUM_PARAMS = 32;

/*
  ==============================================================================

    Preset.h
    Created: 24 Nov 2025 6:07:10pm
    Author:  Valeria

  ==============================================================================
*/

/*
    Module: Preset
    Purpose:
        EN: Defines the in-memory layout of a single factory preset and copies
            its 32 parameter values into a flat float array indexed by role.
        ES: Define la estructura en memoria de un preset de fábrica y copia
            sus 32 valores de parámetro a un arreglo float indexado por rol.

    Main responsibilities:
        EN:
          - Store the preset name as a fixed-size, null-terminated C string
          - Hold all 32 parameter values in a flat array
          - Map constructor arguments (p0..p31) to their semantic role via
            inline comments next to each assignment
        ES:
          - Almacenar el nombre del preset como cadena C de tamaño fijo
          - Contener los 32 valores de parámetros en un arreglo plano
          - Mapear los argumentos del constructor (p0..p31) a su rol semántico
            mediante comentarios al lado de cada asignación

    Architectural role:
        EN: Consumed by the synthesizer when initializing the factory bank
            (see PluginProcessor::createPrograms). The parameter index order
            here MUST match the order declared in PluginProcessor and the
            value of NUM_PARAMS in Constants.h.
        ES: Usado por el sintetizador al inicializar el banco de fábrica
            (ver PluginProcessor::createPrograms). El orden de índices DEBE
            coincidir con el declarado en PluginProcessor y con NUM_PARAMS
            en Constants.h.

    Notes:
        EN:
          - The 32 numeric parameters (p0..p31) follow a positional convention
            rather than named arguments. This keeps the preset table compact
            but makes the inline index comments below the single source of
            truth for the meaning of each slot.
          - `name` is fixed at 40 bytes; longer names are silently truncated
            and always null-terminated.
        ES:
          - Los 32 parámetros (p0..p31) siguen una convención posicional en
            lugar de argumentos con nombre. Esto mantiene compacta la tabla
            de presets, pero convierte a los comentarios de índice de abajo
            en la única fuente de verdad sobre el significado de cada slot.
          - `name` tiene 40 bytes fijos; nombres más largos se truncan en
            silencio y siempre quedan terminados en nulo.
*/

#pragma once

#include <cstring>
#include "Constants.h"


struct Preset
{
    Preset(const char* presetName,
        float p0, float p1, float p2, float p3,
        float p4, float p5, float p6, float p7,
        float p8, float p9, float p10, float p11,
        float p12, float p13, float p14, float p15,
        float p16, float p17, float p18, float p19,
        float p20, float p21, float p22, float p23,
        float p24, float p25, float p26, float p27,
        float p28, float p29, float p30, float p31)
    {
        // EN: Safe copy of the preset name; guarantees null-termination
        //     even if presetName is longer than the buffer.
        // ES: Copia segura del nombre del preset; garantiza terminación nula
        //     incluso si presetName es más largo que el buffer.
        std::strncpy(name, presetName, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';

        // EN: Positional mapping of constructor arguments to parameter slots.
        //     These inline comments are the canonical reference for the role
        //     of each index across the entire codebase.
        // ES: Mapeo posicional de los argumentos del constructor a los slots
        //     de parámetros. Estos comentarios son la referencia canónica
        //     del rol de cada índice en todo el código.
        param[0] = p0;   // osc1Wave
        param[1] = p1;   // osc2Wave
        param[2] = p2;   // oscMix
        param[3] = p3;   // oscTune
        param[4] = p4;   // oscFine
        param[5] = p5;   // glideMode
        param[6] = p6;   // glideRate
        param[7] = p7;   // glideBend
        param[8] = p8;   // filterType
        param[9] = p9;   // filterFreq
        param[10] = p10;  // filterReso
        param[11] = p11;  // filterEnv
        param[12] = p12;  // filterLFO
        param[13] = p13;  // filterVelocity
        param[14] = p14;  // filterKeytrack
        param[15] = p15;  // filterKeycenter
        param[16] = p16;  // filterAttack
        param[17] = p17;  // filterDecay
        param[18] = p18;  // filterSustain
        param[19] = p19;  // filterRelease
        param[20] = p20;  // envAttack
        param[21] = p21;  // envDecay
        param[22] = p22;  // envSustain
        param[23] = p23;  // envRelease
        param[24] = p24;  // lfoRate
        param[25] = p25;  // vibrato
        param[26] = p26;  // noise
        param[27] = p27;  // octave
        param[28] = p28;  // tuning
        param[29] = p29;  // outputLevel
        param[30] = p30;  // polyMode
        param[31] = p31;  // stereoWidth
    }

    // EN: Fixed-size storage. 40 bytes covers preset names like
    //     "Bass - Sincholagua Staccato" with margin to spare.
    // ES: Almacenamiento de tamaño fijo. 40 bytes alcanzan para nombres como
    //     "Bass - Sincholagua Staccato" con margen de sobra.
    char name[40];

    // EN: Flat parameter array. Size is locked to NUM_PARAMS (Constants.h)
    //     so the contract stays consistent across the codebase.
    // ES: Arreglo plano de parámetros. Su tamaño está atado a NUM_PARAMS
    //     (Constants.h) para mantener el contrato consistente.
    float param[NUM_PARAMS];
};
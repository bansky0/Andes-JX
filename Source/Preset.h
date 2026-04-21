/*
  ==============================================================================

    Preset.h
    Created: 24 Nov 2025 6:07:10pm
    Author:  Valeria

  ==============================================================================
*/

#pragma once

#include <cstring>
#include "Constants.h"

struct Preset
{
    Preset(const char* presetName,
           float p0,  float p1,  float p2,  float p3,
           float p4,  float p5,  float p6,  float p7,
           float p8,  float p9,  float p10, float p11,
           float p12, float p13, float p14, float p15,
           float p16, float p17, float p18, float p19,
           float p20, float p21, float p22, float p23,
           float p24, float p25, float p26, float p27,
           float p28, float p29, float p30, float p31)
    {
        std::strncpy(name, presetName, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';

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

    char name[40];
    float param[NUM_PARAMS];
};
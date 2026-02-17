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
    Preset(const char* name,
           float p0,  float p1,  float p2,  float p3,
           float p4,  float p5,  float p6,  float p7,
           float p8,  float p9,  float p10, float p11,
           float p12, float p13, float p14, float p15,
           float p16, float p17, float p18, float p19,
           float p20, float p21, float p22, float p23,
           float p24, float p25, float p26, float p27)
    {
        strcpy(this->name, name);
        param[0]  = p0;   // Osc Mix
        param[1]  = p1;   // Osc Tune
        param[2]  = p2;   // Osc Fine
        param[3]  = p3;   // Glide Mode
        param[4]  = p4;   // Glide Rate
        param[5]  = p5;   // Glide Bend
        param[6]  = p6;   // Filter Freq
        param[7]  = p7;   // Filter Reso
        param[8]  = p8;   // Filter Env
        param[9]  = p9;   // Filter LFO
        param[10] = p10;  // Velocity
        param[11] = p11;  // Filter Attack
        param[12] = p12;  // KeytrackParam
        param[13] = p13;  // KeycenterParam
        param[14] = p14;  // Filter Decay
        param[15] = p15;  // Filter Sustain
        param[16] = p16;  // Filter Release
        param[17] = p17;  // Env Attack
        param[18] = p18;  // Env Decay
        param[19] = p19;  // Env Sustain
        param[20] = p20;  // Env Release
        param[21] = p21;  // LFO Rate
        param[22] = p22;  // Vibrato
        param[23] = p23;  // Noise
        param[24] = p24;  // Octave
        param[25] = p25;  // Tuning
        param[26] = p26;  // Output Level
        param[27] = p27;  // Polyphony
    }

    char name[40];
    float param[NUM_PARAMS];
};
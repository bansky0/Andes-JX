/*
  ==============================================================================

    Voice.h
    Created: 10 Nov 2025 6:45:29pm
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once

#include "Oscillator.h"
 
 struct Voice
 {
     float saw;

     int note;
     Oscillator osc;
     void reset()
     {
         note = 0;
         osc.reset();
         saw = 0.0f;
     } 
     float render()
     {
         float sample = osc.nextSample();
         saw = saw * 0.997f - sample;
         return saw;
     }
 };
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
     int note;
     Oscillator osc; // this is new
     void reset()
     {
         note = 0;
         osc.reset(); // this is new
     } 
      // add this method
     float render()
     {
         return osc.nextSample();
     }
 };
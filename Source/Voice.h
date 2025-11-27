/*
  ==============================================================================

    Voice.h
    Created: 10 Nov 2025 6:45:29pm
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once

//#include "Oscillator.h"
#include "OscillatorPolyBLEP.h"

struct Voice
{
    int note = -1;       // MIDI note actualmente activa
    float freq = 0.0f;   // frecuencia correspondiente
    float velocityGain = 0.997f;

    OscillatorPolyBLEP osc;

    void startNote(int midiNote)
    {
        note = midiNote;
        freq = 440.0f * std::pow(2.0f, (note - 69) / 12.0f); // MIDI → Hz
        osc.setFrequency(freq);
    }

    void stopNote()
    {
        note = -1;
        freq = 0.0f;
        osc.reset();
    }

    void reset()
    {
        stopNote();
    }

    float render()
    {
        if (note < 0)
            return 0.0f;

        // Importante: asegurar que la frecuencia está seteada
        //osc.setFrequency(freq);

        return velocityGain * osc.nextSample();
    }
};

 /*
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
 */